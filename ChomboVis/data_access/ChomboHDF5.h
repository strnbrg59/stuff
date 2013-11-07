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

#ifndef INCLUDED_CHOMBOHDF5_H
#define INCLUDED_CHOMBOHDF5_H

#include "../utils/StatusCodes.h"
#include "../utils/HeteroMap.h"
#include "Box.h"
#include "BoxLayout.h"
#include "hdf5.h"
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

using std::ostream;
using std::istream;
using std::pair;
using std::map;
using std::set;
using std::string;
using std::vector;

class Box;
class HDF5GroupID;

#if ( H5_VERS_MAJOR == 1 && H5_VERS_MINOR > 6 )
    typedef hsize_t ch_offset_t;
#else
    #if ( H5_VERS_MAJOR == 1 && H5_VERS_MINOR == 6 && H5_VERS_RELEASE >= 4 )
        typedef hsize_t ch_offset_t;
    #else
        typedef hssize_t ch_offset_t;
    #endif
#endif


template<class REAL_T> struct GlobalMetadata
{
    void Slice( char axis );
    void Crop( vector<string> const * croppedComponents /*=0*/ );

    int m_precision;
    int m_dimensionality;
    int m_numLevels;
    int m_numComponents;
    Triple<REAL_T> m_anisotropic;
    vector<string>  m_componentNames;
    vector<Intvect> m_dataCenterings; // corresponding to m_componentNames
    bool m_ghostCellsAreSuppliedByUser;
    Intvect        m_outputGhost;
    Triple<REAL_T> m_origin;
};
template<class REAL_T, class OSTREAM_T> OSTREAM_T &
operator<<( OSTREAM_T & o, GlobalMetadata<REAL_T> const & meta )
{
    o << "  precision=" << meta.m_precision << '\n'
      << "  dimensionality=" << meta.m_dimensionality << '\n'
      << "  numLevels=" << meta.m_numLevels << '\n'
      << "  numComponents=" << meta.m_numComponents << '\n'
      << "  anisotropic=" << meta.m_anisotropic << '\n'
      << "  component names: ";
    for( vector<string>::const_iterator iter = meta.m_componentNames.begin();
         iter != meta.m_componentNames.end();
         ++iter )
    {
        o << (*iter) << " ";
    }
    o << '\n'
      << "  ghostCellsAreSuppliedByUser="
          << meta.m_ghostCellsAreSuppliedByUser << '\n'
      << "  outputGhost=" << meta.m_outputGhost << '\n'
      << "  dataCentering=" << meta.m_dataCenterings[0] << '\n'
      << "  origin=" << meta.m_origin << '\n';
    return o;
}


template<class REAL_T> struct LevelMetadata
{
    void Slice( char axis, REAL_T axisPosition, Triple<REAL_T> anisotropic,
                Triple<REAL_T> origin,
                typename vector<map<int,int> >::iterator slicedBoxesMap);
    void Crop( Box const & );
    LevelMetadata() { }
    LevelMetadata( LevelMetadata<REAL_T> const & that );

    REAL_T         m_dx;
    REAL_T         m_dt;
    REAL_T         m_time;
    Box            m_problemDomain; // Adjusted for data centering
    int            m_numBoxes;
    vector<Box>    m_boxes;         // Adjusted for data centering
};
template<class REAL_T, class OSTREAM_T> OSTREAM_T &
operator<<( OSTREAM_T & o, LevelMetadata<REAL_T> const & meta )
{
    o << "  dx=" << meta.m_dx << '\n'
      << "  dt=" << meta.m_dt << '\n'
      << "  dt=" << meta.m_time << '\n'
      << "  problemDomain=" << meta.m_problemDomain << '\n'
      << "  numBoxes=" << meta.m_numBoxes << '\n';
    for( unsigned i=0; i<meta.m_boxes.size(); ++i )
    {
        o << "  box " << i << ": " << meta.m_boxes[i] << '\n';
    }
    return o;
}


struct ParticleMetadata
{
    ParticleMetadata() : m_numParticles(0), m_numComponents(0) {}
    int m_numParticles;
    int m_numComponents;
    vector<string> m_componentNames;
};


enum ChomboHDF5FileType { old_chombo, new_eb_chombo };


/** Represents one hdf5 file and its Chombo dataset.
*/
template<class REAL_T> class ChomboHDF5
{
  public:
    ChomboHDF5( char const * infileName, ChomboHDF5FileType );
    virtual void Init( Status * );
    virtual ~ChomboHDF5();

    GlobalMetadata<REAL_T> const & GetGlobalMetadata() const;
    LevelMetadata<REAL_T>  const & GetLevelMetadata( int level ) const;
    ParticleMetadata       const & GetParticleMetadata() const;

    void   ReadFArray( REAL_T * farray,
                       int level, int boxNum, int component ) const;
    int    GetHyperslabSize( int level, int boxNum ) const;

    void ReadParticleComponent( string componentName, REAL_T * data ) const;
    vector<REAL_T> ReadParticleCoordinates( unsigned particleNum ) const;
    REAL_T ReadParticleCoordinate( unsigned particleNum, string componentName )
        const;

    static void InitializeHDF5Datatypes( int dimensionality );
        // Public because Ascii2HDF5 needs to call it.

  protected:

    virtual void LoadGlobalMetadata();
    void LoadLevelMetadata( int level );
    void LoadParticleMetadata();

    void ReleaseHDF5Datatypes();
    Status LoadBoxCornerData( vector<Box> * levelBoxes,
                              hid_t levelGroupHID );
    vector<string> GetComponentNames( HeteroMap<string> const & );
    void ReadHDF5Vector( HDF5GroupID const & group, char const * datasetName,
                         REAL_T * buf, int numItems, ch_offset_t offset,
                         herr_t * status = 0 ) const;

    bool HasBeenModified() const;
    Intvect GetDataCentering() const;

    virtual void GatherComponentNamesAndCenterings(
        HeteroMap<string> const & ) = 0;
    virtual void ScanForGhostStatus( HeteroMap<string> * ) = 0;

    //
    // Data
    //
    int m_hdf5Dimensionality; // 2 or 3
    hid_t m_fileHID;
    mutable vector< vector<ch_offset_t> > m_hyperslabOffsets;

    GlobalMetadata<REAL_T> * m_globalMetadata;
    vector< LevelMetadata<REAL_T> * > * m_levelMetadataVector;
    ParticleMetadata * m_particleMetadata;
    int m_time; // as in stat(2)
    string m_infileName;


    Consts const * m_strConsts;
};

template<class REAL_T, class OSTREAM_T> OSTREAM_T &
operator<<( OSTREAM_T &, ChomboHDF5<REAL_T> const & );


Status ChomboHDF5DiscoverMetaparameters(
    char const * infileName, int * dim, int * precision,
    ChomboHDF5FileType * filetype );


/** Opens, closes and gives access to and information about an HDF5 group ID. */
class HDF5GroupID
{
  public:
    HDF5GroupID( hid_t parentID, char const * groupName );
    ~HDF5GroupID();
    hid_t GetHid() const { return m_hid; }
    bool ContainsItem( string itemName ) const;
    vector<string> FindAllItems() const;
  private:
    hid_t m_parentHid;
    string m_groupName;
    hid_t m_hid;
};
// H5Giterate function arguments used by some methods of HDF5GroupID.
herr_t FindGroupItem( hid_t loc_id, char const * name, void * soughtItem );
herr_t AccumulateGroupItems( hid_t loc_id, char const * name, void *stringVect);


bool IsLegalDataCentering( int );
Intvect DataCenteringInt2Intvect( int dc, int spaceDim );
int DataCenteringIntvect2Int( Intvect const & iv, int spaceDim );

string ReadHDF5StringAttribute( HDF5GroupID const &, string attributeName,
                                Status * status );

template<class REAL_T> herr_t ChomboH5Aread( hid_t attr, REAL_T * x );

void DisableHDF5ErrorPrinting();
void ReEnableHDF5ErrorPrinting();


struct OpdataForAttributeScan
{
    OpdataForAttributeScan( HeteroMap<string>       * heteromap,
                            Consts            const * strConsts )
      : m_heteromap( heteromap ),
        m_strConsts( strConsts )
    {
    }
    HeteroMap<string>       * m_heteromap;
    Consts            const * m_strConsts;
};

template<class REAL_T> herr_t
AttributeScan( hid_t loc_id, char const * attr_name, void * opdata );

#endif // INCLUDED_CHOMBOHDF5_H
