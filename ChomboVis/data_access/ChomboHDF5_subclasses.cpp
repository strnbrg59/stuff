#include "ChomboHDF5_subclasses.h"
#include "../utils/Consts.h"
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

template <typename REAL_T>
OldChomboHDF5<REAL_T>::OldChomboHDF5( char const * infileName, Status * status )
  : ChomboHDF5<REAL_T>::ChomboHDF5( infileName, old_chombo )
{
    this->m_strConsts = new OldConsts;
    this->Init( status );
}

template <typename REAL_T>
EBChomboHDF5<REAL_T>::EBChomboHDF5( char const * infileName, Status * status )
  : ChomboHDF5<REAL_T>::ChomboHDF5( infileName, new_eb_chombo )
{
    this->m_strConsts = new EBConsts;
    this->Init( status );
}


template <typename REAL_T> void
OldChomboHDF5<REAL_T>::GatherComponentNamesAndCenterings(
    HeteroMap<string> const & hdf5GlobalAttributes )
{    
    Trace t("OldChomboHDF5::GatherComponentNames()");
    int iDummy;
    Status status;
    this->m_globalMetadata->m_numComponents =
        hdf5GlobalAttributes.Get( Consts::num_components,
                                  &iDummy, &status );
    if( status != STATUS_OK ) t.FatalError( "No numComponents in hdf5 file." );

    this->m_globalMetadata->m_componentNames =
        this->GetComponentNames(hdf5GlobalAttributes);
    if( this->m_globalMetadata->m_numComponents
    !=  int(this->m_globalMetadata->m_componentNames.size()) )
    {
        t.FatalError( "Advertised number of components -- %d -- is not equal "
            "to the number of component names listed -- %d.",
            this->m_globalMetadata->m_numComponents,
            int(this->m_globalMetadata->m_componentNames.size()));
    }

    // Data centering.  Fill the m_dataCentering vector, whose elements corres-
    // pond to those of m_componentNames, with as many copies of the one data
    // centering as there are components.  If there are zero components, then
    // m_dataCentering should have size 1.
    // This file format supports just one data centering.  Find it.
    // If no data_centering attribute, assume cell-centering, for backward
    // compatibility.
    int iCentering( hdf5GlobalAttributes.Get(
        this->m_strConsts->data_centering, &iDummy, &status ));
    Intvect intvectCentering;
    if( ! IsLegalDataCentering( iCentering ) )
    {
        t.Error( "Illegal data centering attribute: %d", iCentering );
    }
    if( status == STATUS_NOT_FOUND )
    {
        t.Info( "No data centering attribute found.  "
                "We'll assume the data is cell-centered..." );
        intvectCentering = Intvect(0,0,0);
    } else
    {
        intvectCentering =
            DataCenteringInt2Intvect( iCentering, this->m_hdf5Dimensionality );
    }
    vector<Intvect> & cv( this->m_globalMetadata->m_dataCenterings );
    int n( this->m_globalMetadata->m_numComponents );
    if( n == 0 )
    {
        cv.push_back( intvectCentering );
    } else
    {
        cv.insert( cv.end(), n, intvectCentering );
    }
}


template <typename REAL_T> void
EBChomboHDF5<REAL_T>::GatherComponentNamesAndCenterings(
    HeteroMap<string> const & hdf5GlobalAttributes )
{
    // Look for *CenteredComponents groups.
    HDF5GroupID slash_GID( this->m_fileHID, "/" );
    string centeringName = string("Cell")
                         + string(EBConsts::CenteredComponents);
    if( slash_GID.ContainsItem( centeringName.c_str() ) )
    {
        cerr << "Slash group contains " << centeringName << endl;
    } else
    {
        cerr << "Slash group does not contain " << centeringName << endl;
    }
}


template <typename REAL_T> void
OldChomboHDF5<REAL_T>::ScanForGhostStatus( HeteroMap<string> * heteromap )
{
    //
    // Peek into the level-0 group's data_attributes subgroup, to find out
    // about ghost cells.  This isn't pretty; we're assuming every level comes
    // with the same amount of ghost padding.
    //
    ostringstream groupName;
    groupName << this->m_strConsts->level_ << 0;
    HDF5GroupID level_GroupHID( this->m_fileHID, groupName.str().c_str() );
    HDF5GroupID data_attributes_GroupHID( level_GroupHID.GetHid(),
                                          Consts::data_attributes );
    OpdataForAttributeScan opdata( heteromap, this->m_strConsts );
    H5Aiterate( data_attributes_GroupHID.GetHid(), 0, AttributeScan<REAL_T>,
                                                      &opdata );
}


template <typename REAL_T> void
EBChomboHDF5<REAL_T>::ScanForGhostStatus( HeteroMap<string> * heteromap )
{
    // Much easier than under old hdf5 format.  Here, it's in the "/" group,
    // as m_strConsts->ghost;

    HDF5GroupID root_GroupID( this->m_fileHID, "/" ); // dtor closes group ID
    OpdataForAttributeScan opdata( heteromap, this->m_strConsts );
    H5Aiterate( root_GroupID.GetHid(), 0, AttributeScan<REAL_T>, &opdata );
}


template class OldChomboHDF5<float>;
template class OldChomboHDF5<double>;
template class EBChomboHDF5<float>;
template class EBChomboHDF5<double>;
