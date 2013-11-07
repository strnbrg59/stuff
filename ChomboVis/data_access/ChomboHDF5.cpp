/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/
// Author: Ted Sternberg

#include "ChomboHDF5.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <algorithm>
#include <cassert>
#include <iterator>
#include "../utils/Trace.h"
#include "../utils/Consts.h"
#include "../utils/cmdline.h"

using std::cerr;
using std::cout;
using std::endl;

#ifndef isnan
#define isnan(x) ((x) != (x))
#endif

#ifndef isinf
#define isinf(x) ((x!=0) && (fabs(x) == 2*fabs(x)))
#endif


/** This is called very early on, as it finds items which determine what
 *  ChomboHDF5 template parameters and subclass we need to instantiate.
*/
Status
ChomboHDF5DiscoverMetaparameters(
    char const * infileName,
    int * dimensionality,          // 2 or 3
    int * precision,               // 1 (float) or 2 (double)
    ChomboHDF5FileType * filetype )
{
    Trace t("ChomboHDF5DiscoverMetaparameters()");
    Status status( STATUS_OK );

    //
    // Open file.
    //
    hid_t fileHID = H5Fopen( infileName, H5F_ACC_RDONLY, H5P_DEFAULT);
    if( fileHID < 0 )
    {
        status = STATUS_BAD_FILE;
        t.Error( "Failed to open file %s.", infileName );
        return status;
    }

    {
      HDF5GroupID chombo_global_GroupHID( fileHID, Consts::Chombo_global );
  
      //
      // Dimensionality
      //
      hid_t attribute = H5Aopen_name( chombo_global_GroupHID.GetHid(),
                                      Consts::SpaceDim );
      herr_t ret  = H5Aread( attribute, H5T_NATIVE_INT, dimensionality );
      if( ret != 0 )
      {
          status = STATUS_BAD_FILE;
          t.Error( "Failed to find attribute %s.", Consts::SpaceDim );
          return status;
      }
      if( (*dimensionality != 2) && (*dimensionality != 3) )
      {
          status = STATUS_BAD_FILE;
          t.Error( "Illegal dimensionality: %d", *dimensionality );
          return status;
      }
      H5Aclose( attribute );
  
      //
      // Precision
      //
      attribute =
          H5Aopen_name( chombo_global_GroupHID.GetHid(), Consts::testReal );
      hid_t datatype = H5Aget_type( attribute );
  
      if( H5Tget_precision( datatype ) == H5Tget_precision( H5T_NATIVE_FLOAT ) )
      {
          *precision = 1;
      } else if( H5Tget_precision( datatype ) == 
                 H5Tget_precision( H5T_NATIVE_DOUBLE ) )
      {
          *precision = 2;
      } else
      {
          status = STATUS_BAD_FILE;
          t.Error( "Illegal datatype -- neither float nor double." );
          return status;
      }

      H5Aclose( attribute );
      H5Tclose( datatype );
    }

    //
    // Filetype -- old Chombo or new EBChombo.  No point looking at the
    // "filetype" attribute, as it's spelled with 'f' in the old format, and
    // with 'F' in the new format.  So we just go by whether there's a small or
    // capital 'f' there!
    //
    {
      HDF5GroupID root_GroupHID( fileHID, "/" );
      EBConsts  ebConsts;
      if( root_GroupHID.ContainsItem( string(ebConsts.filetype) ) )
      {
          *filetype = new_eb_chombo;
      } else
      {
          *filetype = old_chombo;
      }
    }

    H5Fclose( fileHID );
    return status;
}


//
// Convenience function (and two specializations) for use in AttributeScan.
//
// Interesting: if we put the general template in the .hImpl file, then the
// template specializations (in the .cpp file) don't get instantiated and the
// flow of control goes to the assert(0) here.  But with the general template
// here, none of them get instantiated
template<class REAL_T> herr_t ChomboH5Aread( hid_t attr, REAL_T * x )
{
    assert(0); // <REAL_T> other than float or double is illegal.
    return 0;
}

template<> herr_t ChomboH5Aread<float>( hid_t attr, float * x )
{
    return H5Aread( attr, H5T_NATIVE_FLOAT, x );
}
template<> herr_t ChomboH5Aread<double>( hid_t attr, double * x )
{
    return H5Aread( attr, H5T_NATIVE_DOUBLE, x );
}


string ReadHDF5StringAttribute( HDF5GroupID const & group,
                                string attributeName,
                                Status * status )
{
    hid_t attrHID = H5Aopen_name( group.GetHid(), attributeName.c_str() );
    *status = STATUS_OK;
    if( attrHID <= 0 )
    {
        *status = STATUS_NOT_FOUND;
        return ""; // HDF5 has just printed lots of warning verbiage for us.
    }
    hid_t atype  = H5Aget_type(attrHID);
    size_t size = H5Tget_size(atype);
    char * buf = new char[size+1];
    H5Aread(attrHID, atype, buf);
    buf[size]=0;    

    H5Aclose( attrHID );
    H5Tclose( atype );
    assert( buf );
    return string(buf);
}



//----------- Methods of HDF5GroupID ------------------
//
/** Opens the group.  Prints error message if something goes wrong.
 *  If you try to open a nonexistent group, hdf5 will complain verbosely.  The
 *  right way to check if a group exists is with H5Gget_objinfo().
 */
HDF5GroupID::HDF5GroupID( hid_t parentHid, char const * groupName )
  : m_parentHid( parentHid ),
    m_groupName( groupName )
{
    Trace t("HDF5GroupID::HDF5GroupID()");
    m_hid = H5Gopen( parentHid, groupName );
    if( m_hid < 0 )
    {
        t.Error( "Failed to open HDF5 group %s.", groupName );
        // But not to worry, HDF5 prints a whole lot of complaints of its own
        // if we get here.
    }
}

/** Closes the group */
HDF5GroupID::~HDF5GroupID()
{
    H5Gclose( m_hid );
}


bool
HDF5GroupID::ContainsItem( string itemName ) const
{
    Trace t("HDF5GroupID::ContainsItem()");
    
    string itemNameCopy( itemName );

    H5Giterate( m_parentHid, m_groupName.c_str(), 0, FindGroupItem,
                &itemNameCopy );
    if( itemNameCopy == string(Consts::unlikely_name) )
    {   
        return true;
    } else
    {   // H5Giterate discovers sub-groups and datasets but not attributes, so
        // we have to turn now to H5Aiterate:
        H5Aiterate( m_hid, 0, FindGroupItem,
                    &itemNameCopy );
        if( itemNameCopy == string(Consts::unlikely_name) )
        {
            return true;
        } else
        {
            return false;
        }
    }


    /* Here's a tempting way to do it, unfortunately it discovers sub-groups and
     * dataseta but not attributes:
    DisableHDF5ErrorPrinting();
    herr_t ret = H5Gget_objinfo( m_hid, itemName.c_str(), 0,0 );
    ReEnableHDF5ErrorPrinting();
    return (! ret);
    */    
}

/** Passed to H5Giterate, to help HDF5GroupID::ContainsItem() determine if a
 *  group contains an item of a certain name.
 *  Arg soughtItem is really a string* (and it comes through the last argument
 *  passed to H5Giterate).  If it matches currItem, then we set
 *  *soughtItem to Consts::unlikely_name -- that's how we indicate soughtItem
 *  has been found.  FIXME: There must be a more elegant way to do this; what
 *  role does the return value play??
*/
herr_t
FindGroupItem( hid_t loc_id, char const * currItem, void * soughtItem  )
{
    //cerr << "@@ Found group item:|" << currItem << "|" << endl;
    string * soughtItemAsString( static_cast<string *>(soughtItem) );

    if( string(currItem) == *soughtItemAsString )
    {
        *soughtItemAsString = string(Consts::unlikely_name);
    }

    return 0;
}


/** Returns a vector of the names of all the items under this group. */
vector<string>
HDF5GroupID::FindAllItems() const
{
    Trace t("HDF5GroupID::FindAllItems()");
    vector<string> result;
    H5Giterate( m_parentHid, m_groupName.c_str(), 0, AccumulateGroupItems,
                &result );
    return result;
}

/** FindAllItems() passes this to H5Giterate().
 *  Arg stringVect is actually a pointer to a vector<string>.  We use that to
 *  accumulate the currItem every time we enter this function.
 *  H5Giterate seems to find DATASET and GROUP items, but not ATTRIBUTE items.
*/
herr_t
AccumulateGroupItems( hid_t loc_id, char const * currItem, void * stringVect  )
{
    vector<string> * stringVectAsVector(
        static_cast<vector<string> *>(stringVect) );
    stringVectAsVector->push_back( currItem );
    return 0;
}



/** The HDF5 file uses an int to indicate data centering, while everywhere else
 *  in ChomboVis we use an Intvect.  Here's the conversion table for 3D.
 *   0 => 0,0,0 : cell-centered
 *   1 => 1,0,0 : x-face-centered
 *   2 => 0,1,0 : y-face-centered
 *   3 => 0,0,1 : z-face-centered
 *   4 => 0,1,1 : x-edge-centered
 *   5 => 1,0,1 : y-edge-centered
 *   6 => 1,1,0 : z-edge-centered
 *   7 => 1,1,1 : node-centered
 *
 * In 2D, the only legal choices are 0,1,2 and 7.
*/
Intvect DataCenteringInt2Intvect( int dc, int spaceDim )
{
    Trace t("DataCenteringInt2Intvect()");
    if( spaceDim == 2 )
    {
        switch( dc )
        {
            case 0 : return Intvect(0,0,0);
            case 1 : return Intvect(1,0,0);
            case 2 : return Intvect(0,1,0);
            case 7 : return Intvect(1,1,0);
            default: t.FatalError( "Illegal data centering code: %d.", dc );
        }
    }
    else /* spaceDim == 3 */
    {
        switch( dc )
        {
            case 0 : return Intvect(0,0,0);
            case 1 : return Intvect(1,0,0);
            case 2 : return Intvect(0,1,0);
            case 3 : return Intvect(0,0,1);
            case 4 : return Intvect(0,1,1);
            case 5 : return Intvect(1,0,1);
            case 6 : return Intvect(1,1,0);
            case 7 : return Intvect(1,1,1);
            default: t.FatalError( "Illegal data centering code: %d.", dc );
        }
    }
    return Intvect(-1,-1,-1); // never gets here
}

int DataCenteringIntvect2Int( Intvect const & iv, int spaceDim )
{
    Trace t("DataCenteringIntvect2Int()");
    if( iv == Intvect(0,0,0) ) return 0;
    if( iv == Intvect(1,0,0) ) return 1;
    if( iv == Intvect(0,1,0) ) return 2;
    if( iv == Intvect(0,0,1) ) return 3;
    if( iv == Intvect(0,1,1) ) return 4;
    if( iv == Intvect(1,0,1) ) return 5;
    if( iv == Intvect(1,1,0) )
    {
        if( spaceDim == 2 ) return 7;
        else                return 6;
    }
    if( iv == Intvect(1,1,1) ) return 7;
    /* else */
    t.FatalError( "Illegal datacentering: (%d,%d,%d).", iv.i(), iv.j(), iv.k());
    return -1; // never gets here.
}


/** Arg centering is one of our hdf5 data-centering codes.  See the comments
 *  above for what the codes mean.
*/
bool
IsLegalDataCentering( int centering )
{
    Trace t("ChomboHDF5::IsLegalDataCentering()");
    return (centering>=0 && centering<=7);
}


//
// Disable/enable HDF5's default printing of errors.  Useful for when you want
// to call H5Gget_objinfo() to find out if something exists.
//
H5E_auto_t g_defaultHDF5ErrorHandler;
int * g_defaultHDF5ErrorHandlerClientData(0);
void DisableHDF5ErrorPrinting()
{
    // First, save the defaults so you can later reenable them -- if you haven't
    // already saved the defaults.
    if( ! g_defaultHDF5ErrorHandlerClientData )
    {
        H5Eget_auto( &g_defaultHDF5ErrorHandler,
                    (void * *)&g_defaultHDF5ErrorHandlerClientData );
    }
    H5Eset_auto( 0,0 );
}

void ReEnableHDF5ErrorPrinting()
{
    if( g_defaultHDF5ErrorHandlerClientData )
    { // Otherwise, it hasn't been disabled.
        H5Eset_auto( g_defaultHDF5ErrorHandler,
                     g_defaultHDF5ErrorHandlerClientData );
    }
}


/** Opens an hdf5 file and loads all of its lightweight data, i.e. everything
 *  except the AMR data ChomboVis will summarize with slices, isosurfaces, etc.
 *  The heavyweight data gets loaded only as needed.
 *
 *  To extract information about what's in the file, use other member functions.
*/
template<class REAL_T>
ChomboHDF5<REAL_T>::ChomboHDF5(
    char const * infileName, ChomboHDF5FileType filetype )
  : m_globalMetadata(0),
    m_levelMetadataVector(0),
    m_particleMetadata(0),
    m_infileName( infileName )
{
    Trace t("ChomboHDF5::ChomboHDF5()");
}

template<typename REAL_T> void
ChomboHDF5<REAL_T>::Init( Status * status )
{
    Trace t("ChomboHDF5::Init()");

    // Initialize info that tells HDF5 how to load your compound data types
    // from disk.
    int iDummy;
    ChomboHDF5FileType ftDummy;
    ChomboHDF5DiscoverMetaparameters(
        m_infileName.c_str(), &m_hdf5Dimensionality, &iDummy, &ftDummy );
    InitializeHDF5Datatypes( m_hdf5Dimensionality );

    //
    // Open file.  Don't bother checking return code, as if we got here then
    // we took care of that in ChomboHDF5DiscoverMetaparameters().
    //
    m_fileHID = H5Fopen( m_infileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    struct stat stat_buf;
    stat( m_infileName.c_str(), &stat_buf );
    m_time = stat_buf.st_mtime;

    //
    // Load all the metadata.
    //
    LoadGlobalMetadata();

    m_levelMetadataVector = new vector<LevelMetadata<REAL_T> * >;
    int n = m_globalMetadata->m_numLevels;
    m_levelMetadataVector->resize( n );
    for( int lev=0;lev<n;++lev )
    {
        LoadLevelMetadata( lev );
    }

    LoadParticleMetadata();

    *status = STATUS_OK;
}


template<class REAL_T> ChomboHDF5<REAL_T>::~ChomboHDF5()
{
    Trace t("ChomboHDF5::~ChomboHDF5()");
    ReleaseHDF5Datatypes();
    H5Fclose( m_fileHID );

    for( int lev=0;lev<m_globalMetadata->m_numLevels;++lev )
    {
        delete (*m_levelMetadataVector)[lev];
    }

    delete m_levelMetadataVector;
    delete m_globalMetadata;
    delete m_particleMetadata;
    delete m_strConsts;
}


template<class REAL_T> GlobalMetadata<REAL_T> const &
ChomboHDF5<REAL_T>::GetGlobalMetadata() const
{
    Trace t("ChomboHDF5::GetGlobalMetadata()");
    return *m_globalMetadata;
}


/** Private.  User should call GetGlobalMetadata(). */
template<class REAL_T> void
ChomboHDF5<REAL_T>::LoadGlobalMetadata()        
{
    Trace t("ChomboHDF5::LoadGlobalMetadata()");
    m_globalMetadata = new GlobalMetadata<REAL_T>;

    int iDummy;
    Status status;

    //
    // Put dimensionality and precision onto the Heteromap.  We won't need to
    // look these up, but we want to verify they're there, in 
    // ValidateChomboHDF5Format().
    //
    m_globalMetadata->m_dimensionality = m_hdf5Dimensionality;
    if( sizeof(REAL_T) == sizeof(float) )
    {
        m_globalMetadata->m_precision = 1;
    } else
    {
        m_globalMetadata->m_precision = 2;
    }

    HeteroMap<string> hdf5GlobalAttributes; // includes group "/".

    //------------------- "/" (root) attribute group ---------------------
    //
    {
        HDF5GroupID root_GroupID( m_fileHID, "/" ); // dtor closes group ID
        OpdataForAttributeScan opdata( &hdf5GlobalAttributes, m_strConsts );
        H5Aiterate( root_GroupID.GetHid(), 0, AttributeScan<REAL_T>, &opdata );

        // origin.
        Triple<REAL_T> rvDummy;
        m_globalMetadata->m_origin =
            hdf5GlobalAttributes.Get( m_strConsts->origin,&rvDummy,&status);
        if( status == STATUS_NOT_FOUND )
        {
            m_globalMetadata->m_origin =
                Triple<REAL_T>(REAL_T(0),REAL_T(0),REAL_T(0));
        }

        // anisotropic.
        m_globalMetadata->m_anisotropic
            = hdf5GlobalAttributes.Get( m_strConsts->anisotropic,
                                        &rvDummy, &status);
        if( status == STATUS_NOT_FOUND )
        {
            m_globalMetadata->m_anisotropic =
                Triple<REAL_T>(REAL_T(1),REAL_T(1),REAL_T(1));
        }
        if( m_hdf5Dimensionality == 2 )
        {
            m_globalMetadata->m_anisotropic[2] = 1.0;
        }
    }

    m_globalMetadata->m_numLevels = hdf5GlobalAttributes.Get(
        m_strConsts->num_levels, &iDummy, &status );
    if( status != STATUS_OK ) { t.FatalError( "No numLevels in hdf5 file." ); }

    GatherComponentNamesAndCenterings( hdf5GlobalAttributes );

    //
    // Ghost data -- supplied by user, and if so, how deep?
    //
    HeteroMap<string> tempAttributes;
    ScanForGhostStatus( &tempAttributes );
    Intvect ivDummy;
    m_globalMetadata->m_outputGhost =
        tempAttributes.Get( m_strConsts->outputGhost, &ivDummy, &status );
    if( status != STATUS_OK )
    {
        // Can count on HDF5 to print diagnostic message for this case.
        m_globalMetadata->m_outputGhost = Intvect(0,0,0);
    }
    if( (m_globalMetadata->m_outputGhost == Intvect(0,0,0))
    ||  (status == STATUS_NOT_FOUND) )
    {
        m_globalMetadata->m_ghostCellsAreSuppliedByUser = false;
    } else
    {
        m_globalMetadata->m_ghostCellsAreSuppliedByUser = true;
    }

}


template<class REAL_T> LevelMetadata<REAL_T> const &
ChomboHDF5<REAL_T>::GetLevelMetadata( int level ) const
{
    Trace t("ChomboHDF5::ReadLevelsMetadata()");
    return *(*m_levelMetadataVector)[level];
}
                    
        
/* Private: user should call GetLevelMetadata().
 * Reads everything but the FABs.
 *  Warning: Must call ReadGlobalMetadata() first.
*/
template<class REAL_T> void
ChomboHDF5<REAL_T>::LoadLevelMetadata( int level )
{
    Trace t("ChomboHDF5::LoadLevelsMetadata()");
    assert( m_globalMetadata );
    assert( m_levelMetadataVector );

    LevelMetadata<REAL_T> * levelMeta = new LevelMetadata<REAL_T>;
    (*m_levelMetadataVector)[level] = levelMeta;

    REAL_T rDummy;
    Box bDummy;
    Status status;

    m_hyperslabOffsets.resize( m_globalMetadata->m_numLevels );

    HeteroMap<string> hdf5LevelAttributes;

    //
    // Open level's HDF5 group:
    //
    ostringstream groupName;
    groupName << m_strConsts->level_ << level;
    HDF5GroupID level_GroupHID( m_fileHID, groupName.str().c_str() );

    //
    // Scan the HDF5 file and fill the level's HeteroMap.
    //
    OpdataForAttributeScan opdata( &hdf5LevelAttributes, m_strConsts );
    H5Aiterate( level_GroupHID.GetHid(), 0, AttributeScan<REAL_T>, &opdata );
    {   // Pick up the "data_attributes" sub-group.  Put it into the same
        // map as the other attributes for this level.
        HDF5GroupID data_attributes_GroupHID( level_GroupHID.GetHid(),
                                              Consts::data_attributes );
        H5Aiterate( data_attributes_GroupHID.GetHid(), 0,
                    AttributeScan<REAL_T>, &opdata );
    }

    //
    // Fill the LevelMetadata.
    //
    levelMeta->m_dx = hdf5LevelAttributes.Get( m_strConsts->dx,
        &rDummy, &status );
    if( status != STATUS_OK ) t.FatalError(
        "No dx for level %d in HDF5 file.", level );

    levelMeta->m_dt = hdf5LevelAttributes.Get( m_strConsts->dt,
        &rDummy, &status );
    if( status != STATUS_OK )
    {
        t.Info( "No dt for level %d in HDF5 file.  Setting dt to 0.", level );
        levelMeta->m_dt = 0.0;
    }

    levelMeta->m_time = hdf5LevelAttributes.Get( Consts::time,
        &rDummy, &status );
    if( status != STATUS_OK )
    {
        t.Info( "No time for level %d in HDF5 file.  Setting time to 0.",
                   level );
        levelMeta->m_time = 0.0;
    }


    levelMeta->m_problemDomain = hdf5LevelAttributes.Get(
        m_strConsts->prob_domain, &bDummy, &status );
    if( status != STATUS_OK ) t.FatalError(
        "No prob_domain for level %d in HDF5 file.", level);
    levelMeta->m_problemDomain.Grow( GetDataCentering(), false );

    status = LoadBoxCornerData( 
        &(levelMeta->m_boxes), level_GroupHID.GetHid() );
    if( status != STATUS_OK ) t.FatalError(
        "No box data for level %d in HDF5 file.", level);

    levelMeta->m_numBoxes = levelMeta->m_boxes.size();

    // Figure the hyperslabOffsets -- the spots in the hdf5 file where data
    // for particular boxes begins.
    m_hyperslabOffsets[level].resize( levelMeta->m_numBoxes );
    m_hyperslabOffsets[level][0] = 0;
    bool usePaddedBox;
    if( m_globalMetadata->m_outputGhost == Intvect(0,0,0) )
    {
        usePaddedBox = false;
    } else
    {
        usePaddedBox = true;
    }
    for( int b=1; b<levelMeta->m_numBoxes; ++b )
    {
        int incr = GetHyperslabSize(level, b-1 )
                 * m_globalMetadata->m_numComponents;
        m_hyperslabOffsets[level][b] =
            m_hyperslabOffsets[level][b-1] + incr;
    }
}


template<class REAL_T> ParticleMetadata const &
ChomboHDF5<REAL_T>::GetParticleMetadata() const
{
    return *m_particleMetadata;
}


/** Private.  User should call GetParticleMetadata(). */
template<class REAL_T> void
ChomboHDF5<REAL_T>::LoadParticleMetadata()
{
    Trace t("ChomboHDF5::LoadParticleMetadata()");
    m_particleMetadata = new ParticleMetadata;

    // Check if there exists a particles group at all.
    HDF5GroupID root_GroupID( m_fileHID, "/" ); // dtor closes group ID
    if( ! root_GroupID.ContainsItem( Consts::particles ) )
    {
        m_particleMetadata->m_numParticles = 0;
        m_particleMetadata->m_numComponents = 0;
        t.Info("No particles group.  OK.");
        return;
    }

    HDF5GroupID particles_GroupID( m_fileHID, Consts::particles );
    //cerr << "*** Constructed particles_GroupID" << endl;

    // Pick up all the attributes and save them in a HeteroMap.
    HeteroMap<string> hdf5ParticleAttributes;

    OpdataForAttributeScan opdata( &hdf5ParticleAttributes, m_strConsts );
    H5Aiterate( particles_GroupID.GetHid(), 0, AttributeScan<REAL_T>, &opdata );

    // Caching for convenience.
    int iDummy;
    Status status;
    m_particleMetadata->m_numComponents =
        hdf5ParticleAttributes.Get( Consts::num_components, &iDummy, &status );
    if( status != STATUS_OK )
    {
        t.FatalError( "No particle num_components in hdf5 file." );
    }
    m_particleMetadata->m_numParticles =
        hdf5ParticleAttributes.Get( Consts::num_particles, &iDummy, &status );
    if( status != STATUS_OK )
    {
        t.FatalError( "No num_particles in hdf5 file." );
    }

    // Pick up the names of all the data components.
    // Initialize m_particleComponent's elements.
    m_particleMetadata->m_componentNames = particles_GroupID.FindAllItems();

    // Sanity checking.
    if( m_particleMetadata->m_numParticles <= 0 )
    {
        t.FatalError( "Illegal number of particles -- %d.",
                      m_particleMetadata->m_numParticles );
    }
    if( m_particleMetadata->m_numComponents 
    !=  int(m_particleMetadata->m_componentNames.size()) )
    {
        t.FatalError( "Advertised number of particle components -- %d -- "
            "is not equal to the number of listed particle component names "
            "-- %d.",
            m_particleMetadata->m_numComponents,
            int(m_particleMetadata->m_componentNames.size()));
    }
}


/** Load from HDF5 file the vector of data for the named component.
 *  Client is responsible for allocating enough space in arg data.
*/
template<class REAL_T> void
ChomboHDF5<REAL_T>::ReadParticleComponent( string componentName,
                                           REAL_T * data ) const
{
    Trace t("ChomboHDF5::LoadParticleComponent()");
    assert( m_particleMetadata->m_numParticles );

    if( HasBeenModified() )
    {
        t.FatalError( "The hdf5 file has been modified since it "
                      "was loaded." );
    }

    // Now read from the hdf5 file.
    HDF5GroupID particleGroup( m_fileHID, Consts::particles );
    ReadHDF5Vector( particleGroup, componentName.c_str(), 
                    data, m_particleMetadata->m_numParticles, 0, 0 );
}


/** For a single particle -- the particleNum-th in the order the particles'
 *  data are stored in the hdf5 file -- return the value of each of its
 *  components.
*/
template<class REAL_T> vector<REAL_T>
ChomboHDF5<REAL_T>::ReadParticleCoordinates( unsigned particleNum ) const
{
    Trace t("ChomboHDF5<REAL_T>::GetParticleCoordinates()");
    assert( m_particleMetadata->m_numParticles );

    HDF5GroupID particleGroup( m_fileHID, Consts::particles );

    vector<REAL_T> result;
    for( int i=0; i<m_particleMetadata->m_numComponents; ++i )
    {
        result.push_back( ReadParticleCoordinate(
            particleNum, m_particleMetadata->m_componentNames[i] ) );
    }

    return result;
}


/** For a single particle -- the particleNum-th in the order the particles'
 *  data are stored in the hdf5 file -- return its value along the named
 *  component.
*/
template<class REAL_T> REAL_T
ChomboHDF5<REAL_T>::ReadParticleCoordinate( unsigned particleNum,
                                            string componentName ) const
{
    Trace t("ChomboHDF5<REAL_T>::GetParticleCoordinate()");
    assert( m_particleMetadata->m_numParticles );

    HDF5GroupID particleGroup( m_fileHID, Consts::particles );

    REAL_T result;

    hid_t dataset = H5Dopen( particleGroup.GetHid(), componentName.c_str() );
    hid_t fileSpace = H5Dget_space( dataset );
    hsize_t  numItemsCopy( 1 );
    ch_offset_t offsetCopy( particleNum );
    H5Sselect_hyperslab( fileSpace, H5S_SELECT_SET,
                         &offsetCopy, 0, &numItemsCopy, 0 );

    hid_t memSpace = H5Screate_simple( 1, &numItemsCopy, 0 );

    if( sizeof(REAL_T) == sizeof(float) )
    {
        H5Dread( dataset, H5T_NATIVE_FLOAT, memSpace, fileSpace,
                 H5P_DEFAULT, &result );
    } else
    if( sizeof(REAL_T) == sizeof(double) )
    {
        H5Dread( dataset, H5T_NATIVE_DOUBLE, memSpace, fileSpace,
                 H5P_DEFAULT, &result );
    } else
    {
        t.FatalError( "Unsupported data type." );
    }

    H5Dclose( dataset );
    H5Sclose( memSpace );
    H5Sclose( fileSpace );

    return result;
}


/** Private.
 *  Read in the info, in this level, on all the box corners.  Put that info
 *  into arg levelBoxes (which is owned by the client).
 *  Adjust box sizes for data centering.
*/
template<class REAL_T> Status
ChomboHDF5<REAL_T>::LoadBoxCornerData(
    vector<Box> * levelBoxes,
    hid_t levelGroupHID )
{
    Status status( STATUS_OK );
    hid_t boxdataset, boxdataspace, memdataspace;
    hsize_t dims[1], maxdims[1];
    herr_t herror;

    boxdataset = H5Dopen( levelGroupHID, m_strConsts->boxes );
    if( boxdataset < 0 )
    {
        return STATUS_BAD_FILE;
    }
    boxdataspace = H5Dget_space( boxdataset );
    if( boxdataspace < 0 )
    {
        return STATUS_BAD_FILE;
    }

    H5Sget_simple_extent_dims( boxdataspace, dims, maxdims );
    memdataspace = H5Screate_simple( 1, dims, 0 );
    int n_boxes = dims[0];

    vector< BoxSimple > buf( n_boxes );
    herror = H5Dread( boxdataset, Box::s_HDF5Datatype, memdataspace,
                      boxdataspace, H5P_DEFAULT,
                      &buf[0] );

    // Warning: boxes read in as is, not adjusted for data centering or ghost
    // cells.  m_hyperslabOffsets do take centering and ghost cells into account
    // though.
    for( int i=0; i<n_boxes; ++i )
    {
        Box box( buf[i], i );
        box.Grow( GetDataCentering(), false );
        levelBoxes->push_back( box );
    }
        
    H5Dclose( boxdataset );
    H5Sclose( boxdataspace );
    H5Sclose( memdataspace );

    return status;
}


/** For now, until we support multiple data centerings. */
template<class REAL_T> Intvect
ChomboHDF5<REAL_T>::GetDataCentering() const
{
    return m_globalMetadata->m_dataCenterings[0];
}


/** Go through m_hdf5GlobalAttributes looking for attributes with keys that
 *  match "component_n", n=0,1,2,....
*/
template<class REAL_T> vector<string>
ChomboHDF5<REAL_T>::GetComponentNames( HeteroMap<string> const & hdf5Attributes)
{
    int compNum=0;
    ostringstream compName;
    compName << Consts::component_ << compNum;
    string dummy;
    Status status;
    int savedDebugLevel = Cmdline::DebugLevel();
    Cmdline::DebugLevel( 1 ); // Suppresses warning we get when we reach the
                              // end of the component name list.
    string compNameStr = hdf5Attributes.Get( compName.str().c_str(),
                                             &dummy, &status );
    vector<string> result;
    while( status == STATUS_OK )
    {
        result.push_back( compNameStr );

        ++compNum;
        compName.str("");
        compName << Consts::component_ << compNum;
        compNameStr = hdf5Attributes.Get( compName.str().c_str(),
                                          &dummy, &status );
    }

    Cmdline::DebugLevel( savedDebugLevel );
    return result;
}


/** Get the number of data points the HDF5 should have, per component, for
 *  the indicated box.  This of course takes account of the extra points due to
 *  non-cellcentered data and user-provided ghost cells.
*/
template<class REAL_T> int
ChomboHDF5<REAL_T>::GetHyperslabSize( int level, int boxNum ) const
{
    Box box( (*m_levelMetadataVector)[level]->m_boxes[boxNum] );
    box.Grow( m_globalMetadata->m_outputGhost, true );
    int result( box.GetNumCells() );
    return result;
}


/** Load the field data (in fortran order...) for the indicated level, box and
 *  component.
 *  Client owns memory in arg farray.  Use GetHyperslabSize() if unsure about
 *  how many REAL_T's to allocate.
*/
template<class REAL_T> void
ChomboHDF5<REAL_T>::ReadFArray( REAL_T * farray,
                                int level, int boxNum, int component ) const
{
    Trace t("ChomboHDF5::ReadFArray()");
    if( HasBeenModified() )
    {
        t.FatalError( "The hdf5 file has been modified since it "
                      "was loaded." );
    }

    ostringstream levelGroupName;
    levelGroupName << m_strConsts->level_ << level;
    HDF5GroupID levelGroupHID( m_fileHID, levelGroupName.str().c_str());
    ch_offset_t length = GetHyperslabSize( level, boxNum );
    ch_offset_t hyperslabOffset(
        m_hyperslabOffsets[level][boxNum] + length*component);
    ReadHDF5Vector( levelGroupHID, Consts::data_datatype0, farray, length,
                    hyperslabOffset, 0 );

    // Scan for infs and NaNs.
    if( CmdlineBase::DebugLevel() > 2 )
    {   
        for( unsigned int u=0;u<length;++u )
        {
            std::string numberType = "normal";
            if( isnan(farray[u]) )
            {
                numberType = "NaN";
            } else
            if( isinf( farray[u] ) )
            {
                numberType = "infinity";
            }

            if( numberType != "normal" )
            {
                Box box( (*m_levelMetadataVector)[level]->m_boxes[boxNum] );
                Intvect loCorner( box.GetLoCorner() );
                Intvect hiCorner( box.GetHiCorner() );
                int I = 1 + hiCorner[0] - loCorner[0];
                int J = 1 + hiCorner[1] - loCorner[1];
                int k = u/(I*J);
                int j = (u - k*I*J)/I;
                int i = u - k*I*J - j*I;

                t.Warning("Found a %s in level %d, box %d, component %d, "
                          "cell [%d,%d,%d].\n"
                          "Use the databrowser to track it down.\n"
                          "(To disable this warning, set the debug level to\n"
                          "2 or less.)",
                          numberType.c_str(),
                          level, boxNum, component,
                          i+loCorner[0], j+loCorner[1], k+loCorner[2]);
            }
        }
    }
}

/** Read n REAL_T numbers from an hdf5 file.
 *  Client is responsible for allocating enough memory in arg buf!
 *  Set the status if arg status!=0.
 */
template<class REAL_T> void
ChomboHDF5<REAL_T>::ReadHDF5Vector(
    HDF5GroupID const & group,
    char const * datasetName,
    REAL_T * buf,
    int numItems,
    ch_offset_t offset,
    herr_t * status ) const
{
    Trace t("ChomboHDF5::ReadHDF5Vector()");

    hid_t dataset = H5Dopen( group.GetHid(), datasetName );
    hid_t fileSpace = H5Dget_space( dataset );
    hsize_t  numItemsCopy( numItems );
    ch_offset_t offsetCopy( offset );
    H5Sselect_hyperslab( fileSpace, H5S_SELECT_SET,
                         &offsetCopy, 0, &numItemsCopy, 0 );

    hid_t memSpace = H5Screate_simple( 1, &numItemsCopy, 0 );

    if( sizeof(REAL_T) == sizeof(float) )
    {
        H5Dread( dataset, H5T_NATIVE_FLOAT, memSpace, fileSpace,
                 H5P_DEFAULT, buf );
    } else
    if( sizeof(REAL_T) == sizeof(double) )
    {
        H5Dread( dataset, H5T_NATIVE_DOUBLE, memSpace, fileSpace,
                 H5P_DEFAULT, buf );
    } else
    {
        t.FatalError( "Unsupported data type." );
    }
    
    H5Dclose( dataset );
    H5Sclose( memSpace );
    H5Sclose( fileSpace );
}


/** Tell HDF5 what it needs to know in order to load compound data from disk to
 *  memory.  Each C++ class that defines one of those compound data types needs
 *  to supply a static InitializeHDF5Datatype(hid_t*) function.
*/
template<class REAL_T> /*static*/ void
ChomboHDF5<REAL_T>::InitializeHDF5Datatypes( int dimensionality )
{
    Intvect::InitializeHDF5Datatype( dimensionality );
    Triple<REAL_T>::InitializeHDF5Datatype( dimensionality );
    Box::InitializeHDF5Datatype( dimensionality );
}
/** Avoid resource leaks.  Undoes work of InitializeHDF5Datatypes(). */
template<class REAL_T> void
ChomboHDF5<REAL_T>::ReleaseHDF5Datatypes()
{
    Intvect::ReleaseHDF5Datatype();
    Triple<REAL_T>::ReleaseHDF5Datatype();
    Box::ReleaseHDF5Datatype();
}

/** Check that all the mandatory attributes were found in the hdf5 file.
 *  If any are missing, better find a quick way out of any further operations
 *  on this hdf5 file, as certain other methods (getters, for instance) will
 *  assume the mandatory attributes are there and if that's not the case these
 *  functions will make the program exit.
 *
 *  The strategy in this function is to call the getters of class
 *  AttributeMaps with the optional Status* arg nonzero.  To keep this function
 *  easy to read, we don't print any errors; we just set the Status* arg.  And
 *  that would be not as specific as one might wish (it'll say STATUS_NOT_FOUND
 *  without saying which attribute was not found), but the getters themselves
 *  print those warnings for us, so we're OK.
 */
/*
template<class REAL_T> void
ChomboHDF5<REAL_T>::ValidateChomboHDF5Format( Status * status ) const
{
    Trace t("ChomboHDF5::ValidateChomboHDF5Format()");
    // FIXME: You should pass in the global and by-level metadatas.
}
*/


/* Return true if the hdf5 file has been modified since the first time we
 * opened it.  We check for this before reading any new hyperslabs, in case
 * the user, unaware of the fact we don't read everything in at init time,
 * carelessly deletes or modifies the hdf5 file.
*/
template<class REAL_T> bool
ChomboHDF5<REAL_T>::HasBeenModified() const
{
    if( getenv("FOOLHARDY_TIMESTAMP_HACK") )
    {
        return false;
    }

    struct stat stat_buf;
    stat( m_infileName.c_str(), &stat_buf );
    if( m_time != stat_buf.st_mtime )
    {
        return true;
    } else
    {
        return false;
    }
}
    

/** This function gets passed as an argument to H5AIterate, which goes through
 *  the hdf5 header and calls this function every time it reaches an attribute
 *  name.  So in here we take those attribute names, and their associated
 *  values, and put them in our HeteroMap (the attributes map, which we pass in
 *  through the opdata argument).
*/
template<class REAL_T> static herr_t
AttributeScan( hid_t locID, char const * attrName, void * opdata )
{
    Trace t("AttributeScan()");
    t.Info( "attrName=%s", attrName );

    herr_t ret = 0;
    hid_t attrHID   = H5Aopen_name(locID, attrName);
    hid_t atype  = H5Aget_type(attrHID);
    hid_t aclass = H5Tget_class(atype);

    // Unpack opdata.
    OpdataForAttributeScan * opdataStruct =
        static_cast<OpdataForAttributeScan *>( opdata );
    HeteroMap<string> * hdf5Attributes = opdataStruct->m_heteromap;
    Consts const * strConsts           = opdataStruct->m_strConsts;
    assert( strConsts );

    switch( aclass )
    {
        case H5T_INTEGER :
            t.Info( "Found H5T_INTEGER..." );
            int i;
            ret = H5Aread( attrHID, H5T_NATIVE_INT, &i );
            t.Info( "Read %d", i );
            hdf5Attributes->Put( attrName, i );
            break;
      
        case H5T_FLOAT:
            t.Info( "Found H5T_FLOAT..." );
            REAL_T x;
            ret = ChomboH5Aread<REAL_T>( attrHID, &x );
            t.Info( "Read %f", x );
            hdf5Attributes->Put( attrName, x );
            break;

        case H5T_STRING:
        {
            t.Info( "Found H5T_STRING..." );
            size_t size = H5Tget_size(atype);
            char * str = new char[size+1];
            ret = H5Aread(attrHID, atype, str );
            str[size] = 0;
            t.Info( "Read |%s|", str );
            hdf5Attributes->Put( attrName, string(str) );
            delete [] str;
            break;
        }

        case H5T_COMPOUND:
        {
            t.Info( "Found H5T_COMPOUND..." );
            // Intvect
            if( !strcmp( attrName, strConsts->output_ghost )
            ||  !strcmp( attrName, strConsts->outputGhost  )
            ||  !strcmp( attrName, strConsts->ghost ) )
            {
                t.Info( "Found output_ghost, outputGhost or ghost" );
                Intvect iv;
                ret = H5Aread( attrHID, Intvect::s_HDF5Datatype, &iv );
                hdf5Attributes->Put( attrName, iv );
            } else

            // BoxSimple
            if( !strcmp( attrName, strConsts->prob_domain ) )
            {
                t.Info( "Found prob_domain" );
                BoxSimple boxSimple;
                ret = H5Aread( attrHID, Box::s_HDF5Datatype, &boxSimple);
                hdf5Attributes->Put( attrName, Box(boxSimple,-1) );
                // 2nd arg to Box(BoxSimple) relevant only for grid boxes.
            } else

            // Triple<REAL_T>
            if( !strcmp( attrName, strConsts->origin ) 
            ||  !strcmp( attrName, strConsts->anisotropic ) )
            {
                t.Info( "Found origin or anisotropic" );
                Triple<REAL_T> rv;
                ret = H5Aread( attrHID, Triple<REAL_T>::s_HDF5Datatype, &rv);
                hdf5Attributes->Put( attrName, rv );
            } else

            // Invalid type
            {
                t.Warning( "Illegal HDF5 compound: %s.  Ignoring it...",
                           attrName );
            }
            break;
        }

        default:
            ret = -1;
    }

    H5Tclose( atype );
    H5Aclose( attrHID );
    return ret;
}


template<class REAL_T>
LevelMetadata<REAL_T>::LevelMetadata( LevelMetadata<REAL_T> const & that ) :
      m_dx(that.m_dx),
      m_dt(that.m_dt),
      m_time(that.m_time),
      m_problemDomain(that.m_problemDomain),
      m_numBoxes(that.m_numBoxes),
      m_boxes(that.m_boxes)
{
}


/** Change *this to what it would be for a dataset sliced through the indicated
 *  axis.
*/
template<class REAL_T> void
GlobalMetadata<REAL_T>::Slice( char axis )
{
    int indexPermutations[3][3] = {{1,2,0},{2,0,1},{0,1,2}};
    int axisNum( axis - 'x' );

    Triple<REAL_T> anisotropicCopy( m_anisotropic );
    Triple<REAL_T> originCopy( m_origin );
    Intvect        outputGhostCopy( m_outputGhost );
    vector<Intvect> dataCenteringCopy( m_dataCenterings );
    for( int i=0;i<3;++i )
    {
        m_anisotropic[i] = anisotropicCopy[indexPermutations[axisNum][i]];
        m_origin[i] = originCopy[indexPermutations[axisNum][i]];
        m_outputGhost[i] = outputGhostCopy[indexPermutations[axisNum][i]];
        for( unsigned c=0;c<m_dataCenterings.size();++c )
        {
            m_dataCenterings[c][i] = 
                dataCenteringCopy[c][indexPermutations[axisNum][i]];
        }
    }

    --m_dimensionality;
}


/** If arg croppedComponents!=NULL, then keep only the components mentioned
 *  therein.  It is an error for arg croppedComponents to contain a duplicate,
 *  or a string that is not an existing component name.
*/
template<class REAL_T> void
GlobalMetadata<REAL_T>::Crop( vector<string> const * croppedComponents /*=0*/ )
{
    Trace t("GlobalMetadata::Crop()");

    if( ! croppedComponents )
    {
        return;                                  // Early return.
    }

    //
    // Reject if there are duplicates
    //
    vector<string> copiedComps( croppedComponents->begin(),
                                croppedComponents->end() );
    std::sort( copiedComps.begin(), copiedComps.end() );
    std::unique( copiedComps.begin(), copiedComps.end() );
    if( croppedComponents->size() != copiedComps.size() )
    {
        t.FatalError( "Arg croppedComponents contains a duplicate." );
    }

    m_componentNames = *croppedComponents;
    m_numComponents = croppedComponents->size();
}


/** OK if cropBox goes outside problem domain. */
template<class REAL_T> void
LevelMetadata<REAL_T>::Crop( Box const & cropBox )
{
    Trace t("LevelMetadata::Crop()");
    vector<Box> croppedBoxes;
    croppedBoxes.reserve(m_numBoxes);
    Status status;
    for( int i=0;i<m_numBoxes;++i )
    {
        Box box( m_boxes[i].Intersect( cropBox, &status ) );
        if( status == STATUS_OK )
        {
            croppedBoxes.push_back( box );
        }
    }
    vector<Box>( croppedBoxes ).swap( croppedBoxes ); // shrink to fit
    m_numBoxes = croppedBoxes.size();
    m_boxes = croppedBoxes;
}


/** Arg slicedBoxesMap has one map for each level.  In the maps, the key is the
 *  box index into the sliced set of boxes, and the value is the box index into
 *  the original, unsliced, set of boxes.
 *
 *  Arg origin is same as GlobalMetadata.m_origin (the original -- not the
 *  you get in a sliced version of GlobalMetadata).
*/
template<class REAL_T> void
LevelMetadata<REAL_T>::Slice(
    char axis, REAL_T axisPosition,
    Triple<REAL_T> anisotropic, Triple<REAL_T> origin,
    typename vector<map<int,int> >::iterator slicedBoxesMap )
{
    m_problemDomain = Box( m_problemDomain, axis );

    int bSliced(0); // Index into sliced collection of boxes.
    vector<Box> slicedBoxes;

    Triple<double> dDx( m_dx*anisotropic[0], m_dx*anisotropic[1],
                        m_dx*anisotropic[2] );
    Triple<double> dOrigin( origin[0], origin[1], origin[2] );

    for( int b=0; b<m_numBoxes; ++b )
    {
        if( m_boxes[b].ContainsPlane( axis, axisPosition, dDx, dOrigin ) )
        {
            slicedBoxesMap->insert(std::make_pair(bSliced,b));
            slicedBoxes.push_back( Box( m_boxes[b], axis ) );
            ++bSliced;
        }
    }
    m_boxes = slicedBoxes;
    m_numBoxes = bSliced;
}

//
// Explicit template instantiations
//
template struct GlobalMetadata<float>;
template struct GlobalMetadata<double>;
template class LevelMetadata<float>;
template class LevelMetadata<double>;
template class ChomboHDF5<float>;
template class ChomboHDF5<double>;
template herr_t AttributeScan<float>(
    hid_t loc_id, char const * attr_name, void * opdata );
template herr_t AttributeScan<double>(
    hid_t loc_id, char const * attr_name, void * opdata );
