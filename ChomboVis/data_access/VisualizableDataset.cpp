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

#include <cstring>
#include <cmath>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <algorithm>
#include <cassert>
#include <list>
#if __GNUC__ < 3
#include <pair.h>
#else
using std::make_pair;
#endif
#include <iterator>
#include "../utils/Trace.h"
#include "../utils/Consts.h"
#include "ChomboHDF5.h"
#include "ChomboHDF5_subclasses.h"
#include "FABAccess.h"
#include "VisualizableDataset.h"
#include "../utils/BoxFinder.h"
#include "../utils/Timer.h"
#include "../utils/ostream_stdio_adaptor.h"
#include "../utils/cmdline.h"

// For stream IO with file descriptors.
// See http://www.google.com/groups?hl=en&lr=&ie=UTF-8&selm=Pine.GSO.4.50.0301191656580.19204-100000%40gaia.cc.gatech.edu
#if __GNUC__ < 3
#include <fstream>
#else
#include <iostream>
#include <ext/stdio_filebuf.h>
typedef __gnu_cxx::stdio_filebuf<char> FILEBUF;
#endif

using std::list;
using std::map;
using std::cerr;
using std::cout;
using std::cin;
using std::endl;

/** Opens an hdf5 file and loads all of its lightweight data, i.e. everything
 *  except the AMR data ChomboVis will summarize with slices, isosurfaces, etc.
 *  The heavyweight data gets loaded only as needed.
*/
template<class REAL_T>
VisualizableDataset<REAL_T>::VisualizableDataset( string infilename )
  : m_BLDs(0), m_realBLDs(0), m_paddedBLDs(0), m_paddedRealBLDs(0),
    m_boxFinder(0),
    m_realBoxFinder(0),
    m_skipBoxSubdivision(false),
    m_optimizationMode( 0 ),
    m_rawAscii(false)
{
    Trace t("VisualizableDataset::VisualizableDataset()");
    Status status;

    // Open the ChomboHDF5 -- our reader for data in HDF5 format.  But first
    // gotta find out what hdf5 format we're using.
    int dummy;
    int precision;
    ChomboHDF5FileType filetype;
    status = ChomboHDF5DiscoverMetaparameters(
        infilename.c_str(), &dummy, &precision, &filetype );
    if( filetype == old_chombo )
    {
        m_chomboHDF5 = boost::shared_ptr<ChomboHDF5<REAL_T> const>(
            new OldChomboHDF5<REAL_T>( infilename.c_str(), &status ));
    } else
    if( filetype == new_eb_chombo )
    {
        m_chomboHDF5 = boost::shared_ptr<ChomboHDF5<REAL_T> const>(
            new EBChomboHDF5<REAL_T>( infilename.c_str(), &status ));
    } else
    {
        t.FatalError( "Illegal filetype" );
    }
    if( status != STATUS_OK )
    {
        t.FatalError( "%s.", StatusName( status ).c_str() );
    }

    //
    // Load global, level and particle metadata structures.
    //
    m_globalMetadata = m_chomboHDF5->GetGlobalMetadata();
    m_levelMetadataVector.reserve( m_globalMetadata.m_numLevels );
    //cout << "Levels metadata:" << endl;
    for( int lev=0; lev<m_globalMetadata.m_numLevels; ++lev )
    {
        m_levelMetadataVector.push_back( m_chomboHDF5->GetLevelMetadata( lev ));
        //cout << "  Level " << lev << " metadata:\n"
        //     << m_levelMetadataVector[lev] << endl;
    }
    m_particleMetadata = m_chomboHDF5->GetParticleMetadata();
    for( unsigned c=0; c<m_particleMetadata.m_componentNames.size(); ++c )
    {
        string name = m_particleMetadata.m_componentNames[c];
        m_particleComponents[name].reset(0);
    }

    // We've now loaded all the metadata.  From here on we'll need to turn to
    // the ChomboHDF5 object only to ask it to load heavyweight data -- FABs and
    // particle components.


    m_userSuppliedGhost = GetOutputGhost();
    if( m_userSuppliedGhost == Intvect(0,0,0) )
    {   // Go with generated ghosts then.
        if( m_globalMetadata.m_dimensionality == 2 )
        {
            m_generatedGhost = Intvect(1,1,0);
        } else
        {
            m_generatedGhost = Intvect(1,1,1);
        }
    } else
    {
        m_generatedGhost = Intvect(0,0,0);
    }

    m_boxLayouts.reserve( GetNumLevels() );
    m_realBoxLayouts.reserve( GetNumLevels() );
    for( int lev=0;lev<GetNumLevels();++lev )
    {
        InitRealBoxLayout(lev);
    }

    for( int lev=0;lev<GetNumLevels();++lev )
    {
        InitBoxLayoutData( lev );
    }
}


/*
* We hold two caches of box information -- one for "real" boxes and one
* for "subdivided" boxes.  Real are the ones from the hdf5 file.
* Subdivided means what's left after carving out the zones covered by boxes
* on the next finer level.  In both cases, we grow the boxes to reflect
* data centering.
* This function initializes the real boxes.  We initialize the subdivided boxes
* in SubdivideBoxes().
*/
template<class REAL_T> void
VisualizableDataset<REAL_T>::InitRealBoxLayout( int lev )
{
    // Grow the problem domains by 1 in each direction we depart from
    // cell-centeredness.
    Box probDomain( GetProbDomain( lev ) );
    SetProbDomain( probDomain, lev );

    // Copy box data from levelMetadataVector.
    // Do it for this level and the one above, cuz we'll need that in
    // SubdivideBoxes().
    BoxLayout<REAL_T> tempBoxLayout( m_levelMetadataVector[lev].m_boxes,
                                     GetDx(lev), GetOrigin() );
    m_realBoxLayouts.push_back( tempBoxLayout );

    VerifyBoxAlignment( lev );
}


template<class REAL_T> void
VisualizableDataset<REAL_T>::InitBoxLayoutData( int lev )
{
    Trace t("InitBoxLayoutData()");

    // Assumes InitRealBoxLayout() has already been called for every level.
    // That initialized all the m_realBoxLayout's.  Now we need to initialize
    // the m_boxLayout for this level (which needs the m_realBoxLayout for the
    // level one finer that this).
    SubdivideBoxes( lev ); 

    // Initialize the m_BLDs, m_realBLDs, m_paddedBLDs and m_paddedRealBLDs
    // matrices.
    typedef boost::shared_ptr< BoxLayoutData<REAL_T> > * BLD_PTR;
    if( ! m_realBLDs )  // first time through
    {
        m_realBLDs =       new BLD_PTR[ GetNumLevels() ];
        m_BLDs =           new BLD_PTR[ GetNumLevels() ];
        m_paddedRealBLDs = new BLD_PTR[ GetNumLevels() ];
        m_paddedBLDs =     new BLD_PTR[ GetNumLevels() ];
    }

    m_realBLDs[ lev ] =       BoxLayoutData<REAL_T>::NewVector(
        GetNumComponents(), m_realBoxLayouts[lev], lev);
    m_BLDs[ lev ] =           BoxLayoutData<REAL_T>::NewVector(
        GetNumComponents(), m_boxLayouts[lev], lev);
    m_paddedRealBLDs[ lev ] = BoxLayoutData<REAL_T>::NewVector(
        GetNumComponents(), m_realBoxLayouts[lev], lev);
    m_paddedBLDs[ lev ] =     BoxLayoutData<REAL_T>::NewVector(
        GetNumComponents(), m_boxLayouts[lev], lev);

    for( int c=0;c<GetNumComponents();++c )
    {
        assert( m_BLDs[lev][c]->IsInitialized() );
        assert( m_realBLDs[lev][c]->IsInitialized() );
        assert( m_paddedBLDs[lev][c]->IsInitialized() );
        assert( m_paddedRealBLDs[lev][c]->IsInitialized() );
    }
}


/* Check if every box's edges coincide with a cell edge one level coarser.
 * If not, that's a violation of an AMR convention and ChomboVis will show
 * slit-like gaps in the slices.
*/
template<class REAL_T> void
VisualizableDataset<REAL_T>::VerifyBoxAlignment( int lev ) const
{
    Trace t("VisualizableDataset::VerifyBoxAlignment()");
    if( lev == 0 )
    {
        return;
    }

    REAL_T dx[3], dx_1[3];
    GetDx( dx, lev );
    GetDx( dx_1, lev-1 );
    for( int b=0; b<GetNumRealBoxes(lev); ++b )
    {
        Box box( m_realBoxLayouts[lev][b] );
        box.Shrink( GetDataCentering(), false ); // restores official size
        //t.Info( "dx[0]=%f, dx_1[0]=%f", dx[0], dx_1[0] );
        for( int a=0;a<GetDimensionality();++a )
        {
            double loExact = box.GetLoCorner(a) * dx[0] / dx_1[0];
            double hiExact = (box.GetHiCorner(a)+1) * dx[0] / dx_1[0];
            //t.Info( "loExact=%f, hiExact=%f", loExact, hiExact );
            if( ( fabs(loExact - int(loExact+0.5)) > dx[0]/(2*dx_1[0]) )
            ||  ( fabs(hiExact - int(hiExact+0.5)) > dx[0]/(2*dx_1[0]) ))
            {
                t.Error( "Box %d on level %d is not aligned with a coarse "
                         "cell boundary.  Your slices may show gaps. ",
                         b, lev );
            }
        }
    }
}


/** Returns cached data, or if not available calls LoadParticleComponent() to
 *  go to disk.
 *  This function is const, but it does result in a change to the mutable
 *  data member m_particleComponents.
*/
template<class REAL_T> boost::shared_array<REAL_T>
VisualizableDataset<REAL_T>::GetParticleComponent( string name ) const
{
    Trace t("VisualizableDataset::GetParticleComponent()");
    assert( m_particleComponents.find(name) != m_particleComponents.end() );
    if( !m_particleComponents[name] )
    {
        LoadParticleComponent( name );
    }
    return m_particleComponents[name];
}


/** Load from HDF5 file the vector of data for the named component.
 *  Sets corresponding element of m_particleComponents.
 *  This function is const, but it does result in a change to the mutable
 *  data member m_particleComponents.
*/
template<class REAL_T> void
VisualizableDataset<REAL_T>::LoadParticleComponent( string name ) const
{
    Trace t("VisualizableDataset::LoadParticleComponent()");
    int n( m_particleMetadata.m_numParticles );
    m_particleComponents[name].reset( new REAL_T[ n ] );

    // Now read from the hdf5 file.
    m_chomboHDF5->ReadParticleComponent( name,m_particleComponents[name].get());
}

/** Releases memory and sets element of m_particleComponents.second to zero. */
template<class REAL_T> void
VisualizableDataset<REAL_T>::ReleaseParticleComponent( string name )
{
    Trace t("VisualizableDataset::ReleaseParticleComponent()");
    assert( m_particleComponents[name] != 0 ); // Trap sloppy memory mgmt.
    m_particleComponents[name].reset( 0 );
}


/** For a single particle -- the particleNum-th in the order the particles'
 *  data are stored in the hdf5 file -- return the value of each of its
 *  components.
*/
template<class REAL_T> vector<REAL_T>
VisualizableDataset<REAL_T>::GetParticleCoordinates( unsigned particleNum )
    const
{
    Trace t("VisualizableDataset<REAL_T>::GetParticleCoordinates()");
    return m_chomboHDF5->ReadParticleCoordinates( particleNum );
}


/** For a single particle -- the particleNum-th in the order the particles'
 *  data are stored in the hdf5 file -- return its value along the named
 *  component.
*/
template<class REAL_T> REAL_T
VisualizableDataset<REAL_T>::GetParticleCoordinate( unsigned particleNum,
                                                    string componentName ) const
{
    Trace t("VisualizableDataset<REAL_T>::GetParticleCoordinate()");
    return m_chomboHDF5->ReadParticleCoordinate( particleNum, componentName );
}


template<class REAL_T> vector<string> const &
VisualizableDataset<REAL_T>::GetParticleComponentNames() const
{
    return m_particleMetadata.m_componentNames;
}

/** Subdivide the boxes into rectangular pieces that are not covered by
 *  higher-level boxes.
 *  Loop over boxes one level finer, until you find one that overlaps
 *  this one (or you get to the end of the boxes at this level).
 *  Subtract out that finer box.  Append the resulting subboxes
 *  to the end of this level's boxes.  Remove the original.
 *  Continue iterating over the current level's boxes until you reach
 *  past the end (and keep in mind the end is expanding).
 * 
 *  We'll start with a list<Box> so that our iterators don't get invalidated
 *  when we grow the collection with new subboxes at the end.  Later, we'll
 *  copy back to a vector<BoxLayout>.
 *
 *  The algorithm as stated took about 9:15 minutes to load miniati-gravity.hdf5
 *  (20,000 level-1 boxes).  Cut that down to 11 seconds by caching, for each
 *  real coarse box, the fine boxes that intersect it.  That's going to be
 *  pretty good as long as we don't have a huge number of fine boxes in a single
 *  coarse box.  At that point, the right algorithm is a scan-line algorithm
 *  (move left to right, keeping track of boxes in an "active set"...)
 *
*/
template<class REAL_T> void
VisualizableDataset<REAL_T>::SubdivideBoxes( int lev )
{
    Trace t("VisDat::SubdivideBoxes()");

    // Gotta create an m_boxLayouts even if we don't want to actually do any
    // box subdividing.
    if( (GetNumLevels() == 1) || (lev == GetNumLevels()-1 ) )
    {
        m_boxLayouts.push_back( m_realBoxLayouts[lev] );
        return;
    }

    //
    // Copy real Boxes to temporary structure that will grow into the collection
    // of subdivided Boxes.
    // Shrink the boxes by the amount of the data centering (we'll grow them
    // back at the end of this function) so that they're just the size of
    // the regions we will render, i.e. we cut out the "overhang".  This
    // way we carve the boxes in a way that results in no overlap between
    // (subdivided) boxes at different levels.
    list< Box > subboxLayout;
    subboxLayout.assign( m_realBoxLayouts[lev].begin(),
                         m_realBoxLayouts[lev].end() );
    for( list<Box>::iterator i = subboxLayout.begin();
         i != subboxLayout.end();
         ++i )
    {
        i->Shrink( GetDataCentering(), false );
    }

    // Now make a similar copy of the lev+1 boxes.
    // We don't actually subdivide lev+1 in here, but we do need to iterate
    // over its real boxes to subtract them out of the boxes at this level, and
    // we do need to subtract out data-centering-adjusted boxes.
    list< Box > fineboxLayout;
    fineboxLayout.assign( m_realBoxLayouts[lev+1].begin(),
                          m_realBoxLayouts[lev+1].end() );
    for( list<Box>::iterator i = fineboxLayout.begin();
         i != fineboxLayout.end();
         ++i )
    {
        i->Shrink( GetDataCentering(), false );
    }


    //
    // Start subdividing Boxes.
    //
    list<Box>::iterator cIter = subboxLayout.begin();
        // 'c' for coarse.
    map<int, vector<Box> > intersectorMap;
    REAL_T dx_l[THREE];
    GetDx( dx_l, lev );
    REAL_T dx_l_plus_1[THREE];
    GetDx( dx_l_plus_1, lev+1 );

    // For some debugging purposes it'll be useful to skip the box-
    // subdivision thing.  We'll still have two sets of boxes, but they'll
    // all be real boxes.
    if( m_skipBoxSubdivision )
    {
        goto SKIP_BOX_SUBDIVISION;
    }

    //
    // Identifies, for each coarse box, which fine boxes intersect it.
    // The keys are the real-box numbers.  So when we go to fill this map,
    // we don't need to do anything if the current coarse box is already a
    // subdivided box; we just look at its real-box number.

    while( cIter != subboxLayout.end() )
    {
        // Fill intersectorMap:
        if( intersectorMap.find( cIter->GetRealIdNum()) == intersectorMap.end())
        {
            vector<Box> intersectors;
            for( list<Box>::iterator fIter = fineboxLayout.begin();
                 fIter != fineboxLayout.end();
                 ++fIter )
            {
                Box scaledFineBox( *fIter );
                scaledFineBox *= dx_l_plus_1[0]/dx_l[0];
                Status status;
                cIter->Intersect( scaledFineBox, &status );
                if( status == STATUS_OK )
                {
                    intersectors.push_back( *fIter );
               }
            }
            intersectorMap[ cIter->GetRealIdNum() ] = intersectors;
        }

        //
        // Here, instead of going through *all* the fine boxes, go over only
        // the fine boxes that we previously found to intersect the current
        // coarse box.
        //
        bool foundIntersectingFineBox = false;
        vector<Box> partition;
        for( vector<Box>::iterator fIter = intersectorMap[ cIter->GetRealIdNum()
                                                       ].begin();
             ((fIter != intersectorMap[ cIter->GetRealIdNum() ].end())
          && (foundIntersectingFineBox == false));
          ++fIter )
        {
            Box scaledFineBox( *fIter );
            scaledFineBox *= dx_l_plus_1[0]/dx_l[0];
            partition = *cIter - scaledFineBox; // Box::operator-()

            if( (partition.size() != 1)
            ||  (partition[0] != *cIter) )  
            {
                foundIntersectingFineBox = true;
            }
        }

        if( ! foundIntersectingFineBox )
        {
            ++cIter; // Should go back to top of cIter loop now.
        } else
        {
            // Check each new box against all existing ones; maybe
            // it can be combined with an existing box to form a
            // single rectangle?  If so, combine the two and append them
            // to the list, erasing the existing coarse box.  Otherwise,
            // just append the new box to list.
            // Do not combine across boundaries of real boxes; we want every
            // subbox to know which real box it comes from.
            for( unsigned int k=0;k<partition.size();++k )
            {
                list<Box>::iterator inner_cIter =subboxLayout.begin();
                bool foundCombiningBox = false;

                // If you don't want your boxes combined (a good idea when
                // debugging Box::operator-()), comment out everything from
                // here down to "Comment-out to here".  To furthermore see
                // the box outlines in ChomboVis, comment out the
                // SetAlwaysUseRealBoxes(1) line in src_py/vtk_grid.py.
        /*
                while( !foundCombiningBox
                &&     (inner_cIter != subboxLayout.end()) )
                {
                    if( inner_cIter->GetRealIdNum()==cIter->GetRealIdNum() )
                    {
                        if( inner_cIter->CanCombineWith(partition[k]) )
                        {
                            Box tempBox( *inner_cIter );
                            tempBox += partition[k];
                            subboxLayout.push_back( tempBox );
                            foundCombiningBox = true;
                        } else
                        if( partition[k].CanCombineWith( *inner_cIter ) )
                        {
                            Box tempBox( partition[k] );
                            tempBox += *inner_cIter;
                            subboxLayout.push_back( tempBox );
                            foundCombiningBox = true;
                        }
                    }
                    if( ! foundCombiningBox )
                    {
                        ++inner_cIter;
                    } // else: drops out of this loop anyway
                }
                // Comment-out to here, if you don't want your boxes
                // combined.
        */

                if( !foundCombiningBox )
                {
                    subboxLayout.push_back( partition[k] );
                } else
                {
                    assert( inner_cIter != cIter );
                    subboxLayout.erase( inner_cIter );
                }
            }

            // Remove *cIter, as we've broken it up and appended the pieces.
            subboxLayout.erase( cIter++ );
        }
    }        

SKIP_BOX_SUBDIVISION:

    // Assign box ID numbers.  Real box ID numbers were assigned in
    // Box::operator-(Box).
    // Also, build the map that associates each real box with the collections of
    // subdivided boxes it's been carved up into.
    cIter = subboxLayout.begin();
    int b=0;
    assert( m_real2subdividedBoxes.size() == unsigned(lev) );
    map<int, vector<int> > boxMap;
    m_real2subdividedBoxes.push_back( boxMap );
    while( cIter != subboxLayout.end() )
    {
        cIter->SetIdNum( b );
        m_real2subdividedBoxes[lev][cIter->GetRealIdNum()].push_back( b );
        ++cIter;
        ++b;
    }

    // Check m_real2subdividedBoxes.
    /*
    cout << "m_real2subdividedBoxes[" << lev << "]:\n";
    map<int, vector<int> > const & table(m_real2subdividedBoxes[lev]);
    for( map<int,vector<int> >::const_iterator iter = table.begin();
         iter != table.end(); ++iter )
    {
        vector<int> const & v( iter->second );
        for( unsigned i=0; i<v.size(); ++i )
        {
            cout << v[i] << " ";
        }
        cout << endl;
    }
    */

    // subdivTimer.Stop( "SubdivideBoxes(): time subdividing level" );

    //
    // Copy subboxLayout into m_boxLayouts -- just copy the list into a vector.
    //

    // You shrank the boxes by the data centering.  Now grow them back.
    for( list<Box>::iterator i = subboxLayout.begin();
         i != subboxLayout.end();
         ++i )
    {
        i->Grow( GetDataCentering(), false );
    }

    BoxLayout<REAL_T> temp;
    m_boxLayouts.push_back( temp );
    m_boxLayouts[lev].assign( subboxLayout.begin(),
                              subboxLayout.end(),
                              GetDx(lev), GetOrigin() );
}


template<class REAL_T> VisualizableDataset<REAL_T>::~VisualizableDataset()
{
    Trace t("VisualizableDataset::~VisualizableDataset()");

    // Delete BLDs.
    for( int l=0; l<GetNumLevels(); ++l )
    {
        assert( m_BLDs[l] );
        assert( m_paddedBLDs[l] );
        assert( m_realBLDs[l] );
        assert( m_paddedRealBLDs[l] );

        for( int c=0;c<GetNumComponents();++c )
        {
            assert( m_BLDs[l][c] );
            assert( m_paddedBLDs[l][c] );
            assert( m_realBLDs[l][c] );
            assert( m_paddedRealBLDs[l][c] );
        }

    
        delete [] m_BLDs[l];
        delete [] m_paddedBLDs[l];
        delete [] m_realBLDs[l];
        delete [] m_paddedRealBLDs[l];

        m_BLDs[l] = m_paddedBLDs[l] = m_realBLDs[l] = m_paddedRealBLDs[l] = 0;
    }
    assert( m_BLDs );
    assert( m_paddedBLDs );
    assert( m_paddedRealBLDs );
    assert( m_realBLDs );
    delete [] m_BLDs;
    delete [] m_paddedBLDs;
    delete [] m_paddedRealBLDs;
    delete [] m_realBLDs;
    m_BLDs = m_paddedBLDs = m_realBLDs = m_paddedRealBLDs = 0;

    delete m_boxFinder;
    delete m_realBoxFinder;

}

//--------------- VisualizableDataset accessors ------------------
//
// Some getters take a Status * arg, some don't.  The ones that do are getters
// for optional attributes.  The ones that don't are for mandatory attributes,
// and it can be assumed that we checked for their existence in 
// ValidateVisualizableDatasetFormat().
//
template<class REAL_T> string
VisualizableDataset<REAL_T>::GetComponentName( int componentNum ) const
{
    Trace t("VisualizableDataset::GetComponentName()");
    return m_globalMetadata.m_componentNames[ componentNum ];
}


template<class REAL_T> vector<string>
VisualizableDataset<REAL_T>::GetComponentNames() const
{
    return m_globalMetadata.m_componentNames;
}


/** Return the ordinal number of the component indicated by name by arg
 *  componentName.
 *  If not found, return -1.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetComponentNum( string componentName ) const
{
    Trace t("VisualizableDataset::GetComponentNum()");

    vector<string> const & names( m_globalMetadata.m_componentNames );
    vector<string>::const_iterator iter =
        find( names.begin(), names.end(), componentName );
    if( iter == names.end() )
    {
        t.Error( "Failed to find component named %s.", componentName.c_str() );
        return -1;
    }

    return iter - names.begin();
}


/** The origin is the spatial (x,y,z) location of the lower corner of the
 *  [0,0,0] cell.
*/
template<class REAL_T> Triple<REAL_T> const &
VisualizableDataset<REAL_T>::GetOrigin() const
{
    return m_globalMetadata.m_origin;
}


/** Returns vector<double>, instead of vector<REAL_T> (or even Triple<REAL_T>),
 *  because we want to call this from Python.  Returns 2 elements if dataset
 *  is 2D, 3 otherwise.
*/
template<class REAL_T> vector<double>
VisualizableDataset<REAL_T>::GetOriginAsVector() const
{
    Trace t("VisualizableDataset::GetOriginAsVector()");
    Triple<REAL_T> const & o( m_globalMetadata.m_origin );
    vector<double> result( m_globalMetadata.m_dimensionality );
    result.assign( o.m_data, o.m_data+m_globalMetadata.m_dimensionality );
    return result;
}


/** Returns vector<double>, instead of vector<REAL_T> (or even Triple<REAL_T>),
 *  because we want to call this from Python.  Returns 2 elements if dataset
 *  is 2D, 3 otherwise.
*/
template<class REAL_T> vector<double>
VisualizableDataset<REAL_T>::GetDxAsVector( int level ) const
{
    Trace t("VisualizableDataset::GetDxAsVector()");
    Triple<REAL_T> const & dx( GetDx(level) );
    int dim(m_globalMetadata.m_dimensionality);
    vector<double> result( dim );
    result.assign( dx.m_data, dx.m_data+dim );
    return result;
}

template<class REAL_T> double
VisualizableDataset<REAL_T>::GetDt( int level ) const
{
    Trace t("VisualizableDataset::GetDt()");
    return m_levelMetadataVector[level].m_dt;
}


template<class REAL_T> double
VisualizableDataset<REAL_T>::GetTime( int level ) const
{
    Trace t("VisualizableDataset::GetTime()");
    return m_levelMetadataVector[level].m_time;
}


template<class REAL_T> int
VisualizableDataset<REAL_T>::GetNumLevels() const
{
    return m_globalMetadata.m_numLevels;
}

template<class REAL_T> int
VisualizableDataset<REAL_T>::GetNumComponents() const
{
    return m_globalMetadata.m_numComponents;
}

template<class REAL_T> int
VisualizableDataset<REAL_T>::GetDimensionality() const
{
    return m_globalMetadata.m_dimensionality;
}

template<class REAL_T> int
VisualizableDataset<REAL_T>::GetNumParticles() const
{
    return m_particleMetadata.m_numParticles;
}

template<class REAL_T> int
VisualizableDataset<REAL_T>::GetNumParticleComponents() const
{
    return m_particleMetadata.m_numComponents;
}

//
// Accessors for by-level data.
//
template<class REAL_T> void
VisualizableDataset<REAL_T>::GetDx( REAL_T * dx, int level ) const
{
    REAL_T scalar_dx( m_levelMetadataVector[level].m_dx );
    for( int i=0;i<THREE;++i )
    {
        dx[i] = scalar_dx * m_globalMetadata.m_anisotropic[i];
    }
}


template<class REAL_T> Triple<REAL_T>
VisualizableDataset<REAL_T>::GetDx( int level ) const
{
    REAL_T dx[3];
    GetDx( dx, level );
    return Triple<REAL_T>( dx );
}


template<class REAL_T> int
VisualizableDataset<REAL_T>::SetDebugLevel( int d ) const
{
    Cmdline::DebugLevel( d );
    return 0; // For benefit of python wrapping
}


/** We only support one data centering, for now. */
template<class REAL_T> Intvect const &
VisualizableDataset<REAL_T>::GetDataCentering() const
{
    Trace t("VisualizableDataset::GetDataCentering()");
    return m_globalMetadata.m_dataCenterings[0];
}


template<class REAL_T> Intvect const &
VisualizableDataset<REAL_T>::GetOutputGhost() const
{
    return m_globalMetadata.m_outputGhost;
}

template<class REAL_T> Box const &
VisualizableDataset<REAL_T>::GetProbDomain( int level ) const
{
    return m_levelMetadataVector[ level ].m_problemDomain;
}


template<class REAL_T> void
VisualizableDataset<REAL_T>::SetProbDomain( Box const & pd, int level )
{
    m_levelMetadataVector[ level ].m_problemDomain = pd;
}


template<class REAL_T> int
VisualizableDataset<REAL_T>::GetNumBoxes( int level ) const
{
    return m_boxLayouts[ level ].size();
}

template<class REAL_T> Triple<REAL_T>
VisualizableDataset<REAL_T>::GetAnisotropic() const
{
    return m_globalMetadata.m_anisotropic;
}

template<class REAL_T> void
VisualizableDataset<REAL_T>::SetAnisotropic( REAL_T x, REAL_T y, REAL_T z )
{
    Triple<REAL_T> t( x, y, z );
    m_globalMetadata.m_anisotropic = t;
}

template<class REAL_T> int
VisualizableDataset<REAL_T>::GetNumRealBoxes( int level ) const
{
    return m_realBoxLayouts[ level ].size();
}

template<class REAL_T> int
VisualizableDataset<REAL_T>::GetTotalNumRealBoxes() const
{
    int result = 0;
    for( int l=0; l<GetNumLevels(); ++l )
    {
        result += GetNumRealBoxes( l );
    }
    return result;
}

template<class REAL_T> int
VisualizableDataset<REAL_T>::GetTotalNumBoxes() const
{
    int result = 0;
    for( int l=0; l<GetNumLevels(); ++l )
    {
        result += GetNumBoxes( l );
    }
    return result;
}

/** We call it Gets, rather than Get, so it's easy to tell it apart from 
 *  FAB::GetBox().  The two have different semantics now that FABs grow their
 *  boxes to include the padding.
*/
template<class REAL_T> Box
VisualizableDataset<REAL_T>::GetsBox( int level, int boxNum, bool padded ) const
{
    Box result( m_boxLayouts[level][boxNum] );
    if( padded )
    {
        if( ! GetGhostCellsSuppliedByUser() )
        {
            result.Grow( m_generatedGhost, true );
        } else
        {
            result.Grow( m_userSuppliedGhost, true );
        }
    }
    return result;
}


template<class REAL_T> Box
VisualizableDataset<REAL_T>::GetsRealBox( int level, int boxNum, bool padded )
    const
{
    Box result( m_realBoxLayouts[level][boxNum] );
    if( padded )
    {
        if( m_userSuppliedGhost == Intvect(0,0,0) )
        {
            result.Grow( m_generatedGhost, true );
        } else
        {
            result.Grow( m_userSuppliedGhost, true );
        }
    }
    return result;
}
//
//--------------- end of VisualizableDataset accessors ------------------

template<class REAL_T> bool
VisualizableDataset<REAL_T>::FabIsEmpty(
    int level, int boxNum, int component, bool padded ) const
{
    boost::intrusive_ptr<FAB<REAL_T> > fab;
    if( padded )
    {
        fab = m_paddedBLDs[level][component]->GetFAB(boxNum);

    } else
    {
        fab = m_BLDs[level][component]->GetFAB(boxNum);
    }

    return !fab;
}
template<class REAL_T> bool
VisualizableDataset<REAL_T>::RealFabIsEmpty(
    int level, int boxNum, int component, bool padded ) const
{
    boost::intrusive_ptr<FAB<REAL_T> > fab;
    if( padded )
    {
        fab = m_paddedRealBLDs[level][component]->GetFAB(boxNum);
    } else
    {
        fab = m_realBLDs[level][component]->GetFAB(boxNum);
    }
    
    return !fab;
}


/** Returns pieces of the main data -- the stuff of which ChomboVis displays
 *  colorcoded slices, isosurfaces, etc.
 *  
 *  When a particular FAB has already been loaded with data, this function
 *  simply returns that FAB.  Otherwise, this function first loads the data
 *  from the corresponding element of m_realFabs or m_paddedRealFabs, according
 *  to arg padded.  Then it cuts out just enough data for itself.
 *
 *  This is const but it modifies the (non-const) elements of
 *  m_fabs or m_paddedFabs.
*/
template<class REAL_T> boost::intrusive_ptr<FAB<REAL_T> >
VisualizableDataset<REAL_T>::GetFAB(
    int level, int boxNum, int component, bool padded ) const
{
    Trace t("VisualizableDataset::GetFAB()");
    //cerr << "Loading unreal FAB, level " << level << ", box " << boxNum
    //     << ", comp " << component << ", padded " << padded << endl;

    assert( m_BLDs );
    assert( m_BLDs[level] );
    assert( m_BLDs[level][component] );
    assert( m_paddedBLDs );
    assert( m_paddedBLDs[level] );
    assert( m_paddedBLDs[level][component] );

    boost::intrusive_ptr<FAB<REAL_T> > fab;
    if( padded )
    {
        fab = m_paddedBLDs[level][component]->GetFAB(boxNum);
    } else
    {
        fab = m_BLDs[level][component]->GetFAB(boxNum);
    }

    if( fab )
    {
        return fab;                                       // Early return
    }

    Box box( GetsBox( level, boxNum, padded ) );
    boost::intrusive_ptr<FAB<REAL_T> > realFab(
        GetRealFAB( level, box.GetRealIdNum(), component, padded ));

    //
    // Now cut out the data we need for this little box.
    // FIXME: We should be able to do this with a ::operator-(FAB,FAB).
    //
    Box realBox( realFab->GetBox() );
    Intvect realLo = realBox.GetLoCorner();
    Intvect realHi = realBox.GetHiCorner();

    Intvect lo = box.GetLoCorner();
    Intvect hi = box.GetHiCorner();

    boost::shared_array<REAL_T> buf( new REAL_T[ box.GetNumCells() ] );

    int nI = 1 + hi.i() - lo.i();
    int nJ = 1 + hi.j() - lo.j();
    int nRealI = 1 + realHi.i() - realLo.i();
    int nRealJ = 1 + realHi.j() - realLo.j();

    for( int k=lo.k(); k<=hi.k(); ++k )
    {
        for( int j=lo.j(); j<=hi.j(); ++j )
        {
            int offset = (j-lo.j()) * nI + (k-lo.k())*nI*nJ;
            int realOffset = lo.i() - realLo.i()
                           + (j-realLo.j()) * nRealI
                           + (k-realLo.k()) * nRealI * nRealJ;

            assert(   offset + nI <= box.GetNumCells() );
            assert( realOffset + nI <= realBox.GetNumCells() );

            memcpy( buf.get() + offset, realFab->GetFArray().get() + realOffset,
                    nI*sizeof(REAL_T) );
        }
    }

    typename FAB<REAL_T>::CtorArgs fabCtorArgs;
    fabCtorArgs.box = box;
    fabCtorArgs.fieldData = buf;
    fabCtorArgs.dx = GetDx(level);
    fabCtorArgs.origin = GetOrigin();
    fabCtorArgs.level = level;
    fabCtorArgs.component = component;
    fabCtorArgs.real = false;
    fabCtorArgs.padded = padded;
    if( padded )
    {
        if( m_userSuppliedGhost == Intvect(0,0,0) )
        {
            fabCtorArgs.ghost = m_generatedGhost;
        } else
        {
            fabCtorArgs.ghost = m_userSuppliedGhost;
        }
    } else
    {
        fabCtorArgs.ghost = Intvect(0,0,0);
    }

    boost::intrusive_ptr<FAB<REAL_T> > result( new FAB<REAL_T>( fabCtorArgs ));
    if( padded )
    {
        m_paddedBLDs[level][component]->SetFAB(boxNum, result);
    } else
    {
        m_BLDs[level][component]->SetFAB(boxNum, result);
    }
    return result;
}


/** Real FABs are FABs that correspond to real Boxes, i.e. the ones mentioned
 *  in the hdf5 file.  The "non-real" kind are the smaller pieces we've broken
 *  the Boxes (and FABs) into for ChomboVis' purpose of displaying multilevel
 *  data while leaving out those parts of the coarse data that is covered by
 *  finer data.
 *
 *  Arg padded refers to the other FAB dichotomy -- that between padded (with
 *  ghost cells) and unpadded.
 *
 *  Arg boxNum refers to the numbering of the real boxes.  (See the m_idNum and
 *  m_realIdNum members of class Box.)
 * 
 *  Note, in class VisualizableDataset, the parallel structures:
 *    m_fabs and m_realFabs
 *    m_boxLayouts and m_realBoxLayouts.
 *
 *  This function returns the real FAB if it's already loaded, otherwise it goes
 *  to the hdf5 file and loads it.  If arg padded==true and there are
 *  user-supplied ghost cells in the file, then we use those.  Otherwise we
 *  generate our own.
*/
template<class REAL_T> boost::intrusive_ptr<FAB<REAL_T> >
VisualizableDataset<REAL_T>::GetRealFAB( int level, int boxNum, int component,
                                         bool padded ) const
{
    Trace t("VisualizableDataset::GetRealFAB()");
    //t.Info( "level=%d, boxNum=%d, component=%d, padded=%d.",
    //        level, boxNum, component, int(padded) );

    assert( m_realBLDs );
    assert( m_realBLDs[level] );
    assert( m_paddedRealBLDs );
    assert( m_paddedRealBLDs[level] );

    typename FAB<REAL_T>::CtorArgs fabCtorArgs;

    if( padded )
    {
        boost::intrusive_ptr<FAB<REAL_T> > paddedFab(
            m_paddedRealBLDs[level][component]->GetFAB(boxNum) );
        if( paddedFab )
        {
            return paddedFab;                             // Early return
        }

        boost::shared_array<REAL_T> paddedFArray;
        Intvect ghost;
        if( ! GetGhostCellsSuppliedByUser() )
        {
            // Generate ghost cells.
            boost::intrusive_ptr<FAB<REAL_T> > unpaddedFab(
                GetRealFAB( level, boxNum, component, false ) );
            paddedFArray = MakePaddedFArray( unpaddedFab, level, component);
            ghost = m_generatedGhost;
        } else
        {
            // Padded FAB is either on disk, or needs to be entirely
            // synthesized.
            if( m_newComponentSynthesisInfo.ContainsName(
                GetComponentName( component ) ))
            {
                paddedFArray = SynthesizeFArray( level, boxNum, component );
            } else
            {
                paddedFArray = LoadFArrayFromDisk( level, boxNum, component );
            }
            ghost = m_userSuppliedGhost;
        }

        fabCtorArgs.box = GetsRealBox(level,boxNum,true);
        fabCtorArgs.fieldData = paddedFArray;
        fabCtorArgs.dx = GetDx(level);
        fabCtorArgs.origin = GetOrigin();
        fabCtorArgs.level = level;
        fabCtorArgs.component = component;
        fabCtorArgs.real = true;
        fabCtorArgs.padded = true;
        fabCtorArgs.ghost = ghost;

        boost::intrusive_ptr<FAB<REAL_T> > result(new FAB<REAL_T>(fabCtorArgs));
        m_paddedRealBLDs[level][component]->SetFAB(boxNum, result);
        return result;
    } else
    {
        boost::intrusive_ptr<FAB<REAL_T> > unpaddedFab(
            m_realBLDs[level][component]->GetFAB(boxNum) );
        if( unpaddedFab )
        {
            return unpaddedFab;                             // Early return
        }

        boost::shared_array<REAL_T> unpaddedFArray;
        Intvect ghost;
        if( ! GetGhostCellsSuppliedByUser() )
        {
            if( m_newComponentSynthesisInfo.ContainsName(
                GetComponentName( component )) )
            {
                unpaddedFArray = SynthesizeFArray( level, boxNum, component);
            } else
            {
                unpaddedFArray = LoadFArrayFromDisk( level, boxNum, component);
            }
            ghost = m_generatedGhost;
        } else
        {
            boost::intrusive_ptr<FAB<REAL_T> > paddedFab(
                GetRealFAB( level, boxNum, component, true ) );
            unpaddedFArray =
                MakeGhostStrippedFArray( m_userSuppliedGhost, level, paddedFab);
            ghost = m_userSuppliedGhost;
        }

        fabCtorArgs.box = GetsRealBox(level,boxNum,false);
        fabCtorArgs.fieldData = unpaddedFArray;
        fabCtorArgs.dx = GetDx(level);
        fabCtorArgs.origin = GetOrigin();
        fabCtorArgs.level = level;
        fabCtorArgs.component = component;
        fabCtorArgs.real = true;
        fabCtorArgs.padded = false;
        fabCtorArgs.ghost = ghost;

        boost::intrusive_ptr<FAB<REAL_T> > result(new FAB<REAL_T>(fabCtorArgs));
        m_realBLDs[level][component]->SetFAB(boxNum, result);
        return result;
    }
}


/** Go to disk.  If there are user-supplied ghost cells, the result is a padded
 *  FAB.  Otherwise the result is an unpadded FAB.
*/
template<class REAL_T> boost::shared_array<REAL_T>
VisualizableDataset<REAL_T>::LoadFArrayFromDisk(
    int level, int boxNum, int component ) const
{
    Trace t("LoadFArrayFromDisk");
    t.Info( "Loading real FAB from disk: level %d, box %d, comp %d",
            level, boxNum, component );

    int hyperslabSize = m_chomboHDF5->GetHyperslabSize( level, boxNum );
    boost::shared_array<REAL_T> buf( new REAL_T[ hyperslabSize ] );
    m_chomboHDF5->ReadFArray( buf.get(), level, boxNum, component );
    return buf;
}


/** Counterpart to LoadFArrayFromDisk().  This is for components that are not
 *  on disk, i.e. in the HDF5 file, at all; these are components brought into
 *  being by DefineNewComponent().  We need to compute their values.
*/
template<class REAL_T> boost::shared_array<REAL_T>
VisualizableDataset<REAL_T>::SynthesizeFArray(
    int level, int boxNum, int component ) const
{
    Trace t("SynthesizeFArray");
    t.Info( "Synthesizing FArray: level %d, box %d, comp %d",
            level, boxNum, component );

    // Get what we need to know about generating this component.
    string componentName( GetComponentName( component ) );
    vector<string> const & argNames(
        m_newComponentSynthesisInfo.GetArgNames( componentName ) );
    VectorFunctor<REAL_T> const * callable(
        m_newComponentSynthesisInfo.GetCallable( componentName ) );

    // Assemble the arguments' FArrays.
    vector<boost::shared_array<REAL_T> > farrays;
    farrays.reserve( argNames.size() );
    for( unsigned c=0; c<argNames.size(); ++c )
    {
        int nc = GetComponentNum( argNames[c] );
        farrays.push_back( GetRealFAB(level,boxNum,nc,
            GetGhostCellsSuppliedByUser())->GetFArray() );
    }

    int nValues = GetHyperslabSize( level, boxNum );
    boost::shared_array<REAL_T> resultFArray((*callable)( farrays, nValues ));
    // That was it; that generated the entire FAB's worth of data.
    // But if anything went wrong, resultFArray is NULL.
    if( ! resultFArray )
    {
        resultFArray.reset( new REAL_T[nValues] );
        memset( resultFArray.get(), 0, nValues*sizeof(REAL_T) );
        t.Error( "Failure computing generated component %s: do not use it. "
                 "(Data has been set to zero.)", componentName.c_str() );
    }

    return resultFArray;
}


/** Retrieves the value in one particular cell.
 *  Arg boxNum refers to a real (as opposed to subdivided -- that's a ChomboVis
 *  graphics trick) box.  Compare to vtkChomboReaderImpl_cxx::GetDatum(),
 *  which switches between real and subdivided boxes according to how the
 *  particular instance of vtkChomboReader has been set.  In the Data API,
 *  though, we have no interest in subdivided boxes.
 *
 *  Arg coords is relative to the box, i.e. (0,0,0) is the lower left-hand
 *  corner of this box, not of the problem domain.
*/
template<class REAL_T> REAL_T
VisualizableDataset<REAL_T>::GetDatum( int level, int boxNum, int component,
                                       Intvect coords ) const
{
    boost::intrusive_ptr<FAB<REAL_T> > fab(
        GetRealFAB( level, boxNum, component, GetGhostCellsSuppliedByUser() ));
    REAL_T result( fab->GetArrayItem(coords.m_data) );
    return result;
}


/*
 *  Arg status, if not NULL, gets set to STATUS_NOT_FOUND if the coordinates
 *  are outside 
*/
template<class REAL_T> REAL_T
VisualizableDataset<REAL_T>::GetDatumXYZ(
    int finestLevelToSearch, int component, Triple<REAL_T> coords,
    Status * status ) const
{
    int levelFound, boxNum;
    Intvect ijk( GetEnclosingCellXYZ( finestLevelToSearch, &levelFound,
                                      &boxNum, coords.m_data, true ) );

    if( status )
    {
        if( levelFound == -1 )
        {
            *status = STATUS_NOT_FOUND;
            return 0; // Doesn't mean anything; client better check status!
        } else
        {
            *status = STATUS_OK;
            return GetDatum( levelFound, boxNum, component, ijk );
        }
    } else
    {
        return GetDatum( levelFound, boxNum, component, ijk );
    }
}


/** Set arg ghostVal to the value of the cell under globalCoords.  Look first
 *  for a cell at arg level.  If there's no box there at that level, look at
 *  progressively coarser boxes.  If there's no box there, at any level, return
 *  STATUS_NOT_FOUND.
*/
template<class REAL_T> Status
VisualizableDataset<REAL_T>::GenerateGhostValue( REAL_T * ghostVal,
    Intvect const & globalCoords,
    int finestLevelToSearch, int component ) const
{
//  Trace t("VisualizableDataset::GenerateGhostValue()");

    int levelAtWhichFoundBox;
    int boxNum = GetEnclosingBoxNum( finestLevelToSearch,
        &levelAtWhichFoundBox, globalCoords.m_data, true );
    if( boxNum == -1 )
    {
        return STATUS_NOT_FOUND;
    } else
    {
        boost::intrusive_ptr<FAB<REAL_T> > fab(
            GetRealFAB( levelAtWhichFoundBox, boxNum, component, false ));
        Intvect scaledGlobalCoords( ScaleGlobalCoords(
            globalCoords, finestLevelToSearch, levelAtWhichFoundBox)
            - fab->GetBox().GetLoCorner() );
        *ghostVal = fab->GetArrayItem( scaledGlobalCoords.m_data );
        return STATUS_OK;
    }
}


/** Return the box ID number, within the indicated level, for the box that
 *  contains the point specified by arg coordsXYZ.  CoordsXYZ are in dx units.
 *
 *  Returns -1 if coords is not over any box at this level.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetEnclosingBoxNumXYZ(
    int level, REAL_T const coordsXYZ[], bool useRealBoxes ) const
{
//  Trace t("VisualizableDataset::GetEnclosingBoxNumXYZ()");

    Intvect coordsIJK( XYZ2IJK( level, Triple<REAL_T>(coordsXYZ) ) );
    return GetEnclosingBoxNum( level, coordsIJK.m_data, useRealBoxes );
}


/** Return the cell that contains the given physical coordinates.
 *  If optional arg status != 0 and resulting ijk falls outside the problem
 *  domain, then set *status to STATUS_NOT_FOUND.
 */
template<class REAL_T> Intvect
VisualizableDataset<REAL_T>::XYZ2IJK( int level, Triple<REAL_T> xyz,
                                      Status * status /*=0*/ ) const
{
    Intvect ijk;
    REAL_T dx[THREE];
    REAL_T dx0[THREE];
    GetDx( dx, level );
    GetDx( dx0, 0 );
    Triple<REAL_T> origin( m_globalMetadata.m_origin );
    ijk[2] = 0;
    for( int m=0; m<m_globalMetadata.m_dimensionality; ++m )
    {
        ijk[m] =
            int( (xyz[m] - origin[m]
                 + 0.5 * GetDataCentering()[m] * (dx[m] - dx0[m]))
                 / dx[m] );
    }

    if( status )
    {
        Box probDomain( GetProbDomain( level ) );
        Intvect loCorner( probDomain.GetLoCorner() );
        Intvect hiCorner( probDomain.GetHiCorner() );
        if( (ijk[0] < loCorner[0])
        ||  (ijk[0] > hiCorner[0])
        ||  (ijk[1] < loCorner[1])
        ||  (ijk[1] > hiCorner[1])
        ||  (ijk[2] < loCorner[2])
        ||  (ijk[2] > hiCorner[2]) )
        {
            *status = STATUS_NOT_FOUND;
        } else
        {
            *status = STATUS_OK;
        }
    }

    return ijk;
}


/** Return the physical coordinates of the center of the ijk-th cell. */
template<class REAL_T> Triple<REAL_T>
VisualizableDataset<REAL_T>::IJK2XYZ( int level, int boxNum, Intvect ijk ) const
{
    Triple<REAL_T> result;
    REAL_T dx[THREE];
    REAL_T dx0[THREE];
    GetDx( dx, level );
    GetDx( dx0, 0 );
    Triple<REAL_T> origin( m_globalMetadata.m_origin );
    for( int m=0; m<THREE; ++m )
    {
        result[m] = ijk[m]*dx[m] + origin[m] -
                  0.5 * GetDataCentering()[m] * (dx[m] - dx0[m]);
    }
    return result;
}


/** Return the coordinates, relative to the enclosing box.  Set arg *boxNum
 *  to that enclosing box's number, within its level.
 *  If coordsXYZ is outside the domain, sets levelAtWhichFoundBox to -1, and
 *  return value is undefined.
*/
template<class REAL_T> Intvect
VisualizableDataset<REAL_T>::GetEnclosingCellXYZ(
    int finestLevelToSearch,
    int * levelAtWhichFoundBox, int * boxNum,
    REAL_T const coordsXYZ[],
    bool useRealBoxes  ) const
{
    Trace t("VisualizableDataset::GetEnclosingCellXYZ()");
    *boxNum = GetEnclosingBoxNumXYZ( finestLevelToSearch,
                                     levelAtWhichFoundBox, coordsXYZ,
                                     useRealBoxes );
    if( *boxNum == -1 )
    {
        return Intvect(-1,-1,-1);
    }

    //
    // Now convert coordsXYZ to ijk, and subtract from that the box's lower
    // corner.
    //

    Box box;
    if( useRealBoxes )
    {
        box = GetsRealBox( *levelAtWhichFoundBox, *boxNum,
                           GetGhostCellsSuppliedByUser() );
    } else
    {
        box = GetsBox( *levelAtWhichFoundBox, *boxNum,
                       GetGhostCellsSuppliedByUser() );
    }

    int coordsIJK[THREE];
    REAL_T dx[THREE];
    REAL_T dx0[THREE];
    GetDx( dx, *levelAtWhichFoundBox );
    GetDx( dx0, 0 );
    Triple<REAL_T> origin = m_globalMetadata.m_origin;
    for( int m=0; m<THREE; ++m )
    {
        coordsIJK[m] =
            int( (coordsXYZ[m] - origin[m]
                  + 0.5 * GetDataCentering()[m] * (dx[m] - dx0[m]))
                 / dx[m] )
            - box.GetLoCorner(m);
    }

    Intvect result( coordsIJK );
    return result;
}

/** Return the box ID number, within the indicated level, for the box that
 *  contains the point specified by arg coords.  The units of coords are cells.
 *  Returns -1 if coords is not over any box at this level.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetEnclosingBoxNum(
    int level, int const coords[], bool useRealBoxes /* = false */ ) const
{
    Trace t("VisualizableDataset::GetEnclosingBoxNum()");
    assert( level >= 0 );
    if( level >= GetNumLevels() )
    {
        t.FatalError( "Illegal level -- %d -- dataset only goes up to %d.",
                      level, GetNumLevels() - 1 );
    }

    BoxFinder * boxFinder;

    if( useRealBoxes  )
    {
        if( !m_realBoxFinder )
        {
            m_realBoxFinder = new BoxFinder( GetDimensionality() );
        }
        boxFinder = m_realBoxFinder;
    } else
    {
        if( !m_boxFinder )
        {
            m_boxFinder = new BoxFinder( GetDimensionality() );
        }
        boxFinder = m_boxFinder;
    }

    bool padded( GetGhostCellsSuppliedByUser() );

    if( ! boxFinder->IsPrimed( level ) )
    {
        // Assemble all box corner data and pass to boxFinder->Prime().

        map<int, vector<int> > cornerMap;

        int iterLimit = GetNumBoxes(level);
        if( useRealBoxes )
        {
            iterLimit = GetNumRealBoxes(level);
        }
        for( int iBox=0; iBox<iterLimit; ++iBox )
        {
            Box box;
            if( useRealBoxes )
            {
                box = GetsRealBox( level, iBox, padded );
            } else
            {
                box = GetsBox( level, iBox, padded );
            }
            vector<int> corners( 2*THREE );
            for( int m=0; m<GetDimensionality(); ++m )
            {
                corners[ m ] = box.GetLoCorner(m);
                corners[ THREE + m ] = box.GetHiCorner(m);
            }

            cornerMap[ iBox ] = corners;
        }
        boxFinder->Prime( level, cornerMap );
    }

    int result = boxFinder->FindBox( level, coords );
    return result;
}


/** Return the box ID number, within the finest level (but not finer than arg
 *  level), for the box that contains the point specified by arg coords.  The
 *  units of coords are cells, and the coordinate system is the one anchored
 *  at the problem domain's lower corner.
 *  Arg levelAtWhichFoundBox gets set to the level (arg finestLevelToSearch or
 *  coarser) at which we found a box under arg coords.
 *  Returns -1 if coords is not over any box at this level or coarser.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetEnclosingBoxNum(
    int finestLevelToSearch,
    int * levelAtWhichFoundBox, 
    int const coords[],
    bool useRealBoxes /* = false */ ) const
{
    Trace t("VisualizableDataset::GetEnclosingBoxNum()");
    assert( levelAtWhichFoundBox );

    int result;
    Intvect scaledCoords;
    for( int l = finestLevelToSearch; l>=0; --l )
    {
        scaledCoords = ScaleGlobalCoords( Intvect(coords),
                                          finestLevelToSearch, l );
        result = GetEnclosingBoxNum( l, scaledCoords.m_data, useRealBoxes );
        if( result != -1 )
        {
            *levelAtWhichFoundBox = l;
            return result;
        }
    }
    *levelAtWhichFoundBox = -1;
    return -1;
}


/** See other GetEnclosingBoxNum and GetEnclosingBoxNumXYZ for comments. */
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetEnclosingBoxNumXYZ(
    int finestLevelToSearch,
    int * levelAtWhichFoundBox, 
    REAL_T const coordsXYZ[],
    bool useRealBoxes /* = false */ ) const
{
    //Trace t("VisualizableDataset::GetEnclosingBoxNumXYZ()");
    assert( levelAtWhichFoundBox );

    int result;
    for( int l = finestLevelToSearch; l>=0; --l )
    {
        result = GetEnclosingBoxNumXYZ( l, coordsXYZ, useRealBoxes );
        if( result != -1 )
        {
            *levelAtWhichFoundBox = l;
            return result;
        }
    }
    *levelAtWhichFoundBox = -1;
    return -1;
}


/** Arg coords is global (i.e. anchored at domain loCorner) at the scale of
 *  arg fromLevel.  Return the global coordinates at the scale of toLevel.
*/
template<class REAL_T> Intvect
VisualizableDataset<REAL_T>::ScaleGlobalCoords(
    Intvect coords, int fromLevel, int toLevel ) const
{
    Box fromDomain( GetProbDomain(fromLevel) );
    Box toDomain( GetProbDomain(toLevel) );
    REAL_T dxFrom[THREE], dxTo[THREE];
    GetDx( dxFrom, fromLevel );
    GetDx( dxTo, toLevel );
    Intvect result( toDomain.GetLoCorner()
                  + MultiplyAndRoundDown( coords - fromDomain.GetLoCorner(),
                                          dxFrom[0]/dxTo[0] ) );
    return result;
}


/** Return paddedFab's data, minus the ghost cells, in a format appropriate for
 *  an unpadded version of paddedFab.
*/
template<class REAL_T> boost::shared_array<REAL_T>
VisualizableDataset<REAL_T>::MakeGhostStrippedFArray( 
    Intvect const & userSuppliedGhosts, int level,
    boost::intrusive_ptr< FAB<REAL_T> > const & paddedFab ) const
{
    assert( paddedFab->IsReal() );

    Box unpaddedBox( GetsRealBox( level, paddedFab->GetBoxNum(), false ) );
    int boxDims[3];
    unpaddedBox.GetDims( boxDims );
    int const I( boxDims[0] );
    int const J( boxDims[1] );
    int const K( boxDims[2] );

    boost::shared_array<REAL_T> paddedRep( paddedFab->GetFArray() );
    boost::shared_array<REAL_T> unpaddedRep;  // We'll return this.
    if( GetDimensionality() == 2 )
    {
        unpaddedRep.reset( new REAL_T[I*J] );
    } else
    {
        unpaddedRep.reset( new REAL_T[I*J*K] );
    }

    int const ghostI( userSuppliedGhosts.i() );
    int const ghostJ( userSuppliedGhosts.j() );
    int const ghostK( userSuppliedGhosts.k() );

    for( int k=0; k<K; ++k )
    {
        for( int j=0; j<J; ++j )
        {
            if( GetDimensionality() == 2 )
            {
                memcpy( unpaddedRep.get() + I*j,
                        paddedRep.get() + (I+2*ghostI)*(j+ghostJ) + ghostI,
                        I*sizeof(REAL_T) );
            } else
            {
                int unpaddedOffset = I*J*k + I*j;
                int paddedOffset = (I+2*ghostI)*(J+2*ghostJ)*(k+ghostK) 
                                 + (I+2*ghostI)*(j+ghostJ) + ghostI;
                assert(unpaddedOffset+I <= I*J*K );
                assert(paddedOffset+I <=(I+2*ghostI)*(J+2*ghostJ)*(K+2*ghostK));
                memcpy( unpaddedRep.get() + unpaddedOffset,
                        paddedRep.get() + paddedOffset,
                        I*sizeof(REAL_T) );
            }            
        }
    }

    return unpaddedRep;
}


/** Returns 1 if hdf5 file contains ghost cell data.  Returns 0 otherwise. */
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetGhostCellsSuppliedByUser() const
{
    if( m_userSuppliedGhost == Intvect(0,0,0) )
    {
        return 0;
    } else
    {
        return 1;
    }
}


/** Return the FArray that would go with the padded counterpart of the
 *  arg unpaddedFab.
*/
template<class REAL_T> boost::shared_array<REAL_T>
VisualizableDataset<REAL_T>::MakePaddedFArray(
    boost::intrusive_ptr< FAB<REAL_T> > const & unpaddedFab,
    int level, int component ) const
{
    Trace t("VisualizableDataset::MakePaddedFArray()");

    // We deal only with real boxes here.  Once we put ghosts around a real box,
    // all its subboxes will find all the normal and ghost data they need from
    // the one real box they are subboxes of.
    assert( unpaddedFab->IsReal() );

    int boxDims[3];
    unpaddedFab->GetBox().GetDims( boxDims );
    int const I( boxDims[0] );
    int const J( boxDims[1] );
    int const K( boxDims[2] );
    boost::shared_array<REAL_T> unpaddedRep = unpaddedFab->GetFArray();

    boost::shared_array<REAL_T> paddedRep;  // We'll return this.
    if( GetDimensionality() == 2 )
    {
        paddedRep.reset( new REAL_T[(I+2)*(J+2)] );
    } else
    {
        paddedRep.reset( new REAL_T[(I+2)*(J+2)*(K+2)] );
    }

    // First, copy what's already in unpaddedRep.
    for( int k=0; k<K; ++k )
    {
        for( int j=0; j<J; ++j )
        {
            if( GetDimensionality() == 2 )
            {
                memcpy( paddedRep.get() + (I+2)*(j+1) + 1,
                        unpaddedRep.get() + I*j, I*sizeof(REAL_T) );
            } else
            {
                memcpy( paddedRep.get() + (I+2)*(J+2)*(k+1) + (I+2)*(j+1) + 1,
                        unpaddedRep.get() + I*J*k + I*j, I*sizeof(REAL_T) );
            }
        }
    }

    //
    // Add the ghosts.
    //
    REAL_T ghostVal;
    int localCoords[3];   // Relative to lower corner of unpaddedFab.
    Intvect globalCoords; // Relative to problem domain at unpaddedFab's level.
    int validCell[3];     // In unpaddedFab, nearest cell to desired ghost cell.
    int kMin, kMax;
    if( GetDimensionality() == 2 )
    {
        kMin = 0; kMax = 0;
    } else
    {
        kMin = -1; kMax = K;
    }

    for( int k=kMin; k<=kMax; ++k )
    {
        for( int j=-1; j<J+1; ++j )
        {
            // Left margin (bottom to top).
            localCoords[0] = -1; localCoords[1] = j; localCoords[2] = k;
            globalCoords = Intvect(localCoords)
                         + unpaddedFab->GetBox().GetLoCorner();
            if( GenerateGhostValue( &ghostVal, globalCoords, level, component )
            == STATUS_NOT_FOUND )  // globalCoords not in any box
            {
                validCell[0]=0;
                validCell[1]=std::max(std::min(j,J-1),0);
                validCell[2]=std::max(std::min(k,K-1),0);
                if( GetDimensionality() == 2 ) validCell[2] = 0;

                ghostVal = unpaddedFab->GetArrayItem( validCell );
            }

            if( GetDimensionality() == 2 )
            {
                FabAccess::SetArrayItemFast( 0,j+1,0,I+2,J+2, paddedRep.get(),
                                             ghostVal );
            } else
            {
                FabAccess::SetArrayItemFast( 0,j+1,k+1,I+2,J+2, paddedRep.get(),
                                             ghostVal );
            }
        }

        // We're looping over the same j's again because this way we ensure lots
        // of correct guesses in BoxFinder::FindBox().
        for( int j=-1; j<J+1; ++j )
        {   
            // Right margin (bottom to top).
            localCoords[0] = I; localCoords[1] = j; localCoords[2] = k;
            globalCoords = Intvect(localCoords)
                         + unpaddedFab->GetBox().GetLoCorner();
            if( GenerateGhostValue( &ghostVal, globalCoords, level, component )
            == STATUS_NOT_FOUND ) // globalCoords not in any box
            {
                validCell[0]=I-1;
                validCell[1]=std::max(std::min(j,J-1),0);
                validCell[2]=std::max(std::min(k,K-1),0);
                if( GetDimensionality() == 2 ) validCell[2] = 0;
                ghostVal = unpaddedFab->GetArrayItem( validCell );
            }

            if( GetDimensionality() == 2 )
            {
                FabAccess::SetArrayItemFast(I+1,j+1,0,I+2,J+2, paddedRep.get(),
                                            ghostVal );
            } else
            {
                FabAccess::SetArrayItemFast(I+1,j+1,k+1,I+2,J+2,paddedRep.get(),
                                            ghostVal );
            }
        }

        for( int i=0; i<I; ++i )
        {
            // Top margin.
            localCoords[0] = i; localCoords[1] = J; localCoords[2] = k;
            globalCoords = Intvect(localCoords)
                         + unpaddedFab->GetBox().GetLoCorner();
            if( GenerateGhostValue( &ghostVal, globalCoords, level, component )
            == STATUS_NOT_FOUND )  // globalCoords not in any box
            {
                validCell[0]=std::max(std::min(i,I-1),0);
                validCell[1]=J-1;
                validCell[2]=std::max(std::min(k,K-1),0);
                if( GetDimensionality() == 2 ) validCell[2] = 0;
                ghostVal = unpaddedFab->GetArrayItem( validCell );
            }

            if( GetDimensionality() == 2 )
            {
                FabAccess::SetArrayItemFast(i+1,J+1,0,I+2,J+2, paddedRep.get(),
                                            ghostVal );
            } else
            {
                FabAccess::SetArrayItemFast(i+1,J+1,k+1,I+2,J+2,paddedRep.get(),
                                            ghostVal );
            }
        }

        // We're looping over the same i's again because this way we ensure lots
        // of correct guesses in BoxFinder::FindBox().
        for( int i=0; i<I; ++i )
        {
            // Bottom margin.
            localCoords[0] = i; localCoords[1] = -1; localCoords[2] = k;
            globalCoords = Intvect(localCoords)
                         + unpaddedFab->GetBox().GetLoCorner();
            if( GenerateGhostValue( &ghostVal, globalCoords, level, component )
            == STATUS_NOT_FOUND )
            {
                validCell[0]=std::max(std::min(i,I-1),0);
                validCell[1]=0;
                validCell[2]=std::max(std::min(k,K-1),0);
                if( GetDimensionality() == 2 ) validCell[2] = 0;
                ghostVal = unpaddedFab->GetArrayItem( validCell );
            }

            if( GetDimensionality() == 2 )
            {
                FabAccess::SetArrayItemFast(i+1,0,0,I+2,J+2, paddedRep.get(),
                                            ghostVal );
            } else
            {
                FabAccess::SetArrayItemFast( i+1,0,k+1,I+2,J+2, paddedRep.get(),
                                             ghostVal );
            }
        }
    }

    // Front & back, if 3D data.
    if( GetDimensionality() == 3 )
    {
        for( int i=0; i<I; ++i )
        {
            for( int j=0; j<J; ++j )
            {
                // Front.
                localCoords[0] = i; localCoords[1] = j; localCoords[2] = -1;
                globalCoords = Intvect(localCoords)
                             + unpaddedFab->GetBox().GetLoCorner();
                if( GenerateGhostValue( &ghostVal, globalCoords, level,
                                        component )
                == STATUS_NOT_FOUND )
                {
                    validCell[0]=std::max(std::min(i,I-1),0);
                    validCell[1]=std::max(std::min(j,J-1),0);
                    validCell[2]=0;
                    ghostVal = unpaddedFab->GetArrayItem( validCell );
                }
                FabAccess::SetArrayItemFast(i+1,j+1,0,I+2,J+2, paddedRep.get(),
                                             ghostVal );
            }
            // We're looping over the same j's again because this way we
            // ensure lots of correct guesses in BoxFinder::FindBox().
            for( int j=0; j<J; ++j )
            {
                // Back.
                localCoords[0] = i; localCoords[1] = j; localCoords[2] = K;
                globalCoords = Intvect(localCoords)
                             + unpaddedFab->GetBox().GetLoCorner();
                if( GenerateGhostValue( &ghostVal, globalCoords, level,
                                        component )
                == STATUS_NOT_FOUND )
                {
                    validCell[0]=std::max(std::min(i,I-1),0);
                    validCell[1]=std::max(std::min(j,J-1),0);
                    validCell[2]=K-1;
                    ghostVal = unpaddedFab->GetArrayItem( validCell );
                }

                FabAccess::SetArrayItemFast( i+1, j+1, K+1, I+2, J+2,
                                              paddedRep.get(), ghostVal );
            }
        }
    }

    return paddedRep;
}


/** Creates a new data component, appending it to the others and making it
 *  accessible just like the others.
 *
 *  Arg name is the name of the new component.
 *  Arg pythonFunction is a function defined at the Python layer.
 *  Arg argNames is names of existing components that, point-wise, are to be
 *     arguments of pythonFunc.  Be sure argNames.size() is the same as the
 *     number of arguments that pythonFunc expects.
 *
 *  We don't actually generate any values of the new component; that happens
 *  on demand, as individual FABs are needed.  Still, we have quite a lot to do
 *  here; we need to expand the BLD matrices and do some bookkeeping.
 *
 *  Returns 0 if OK, -1 if not.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::DefineNewComponent(
    string name, VectorFunctorBase * callable,
    vector<string> const & argNames )
{
    Trace t("VisualizableDataset::DefineNewComponent()");
    
    if( m_newComponentSynthesisInfo.ContainsName( name ) )
    {
        t.Warning( "Component %s already exists.  Use RedefineNewComponent().",
                   name.c_str() );
        return -1;
    }
    m_newComponentSynthesisInfo.Insert(
        name,
        static_cast<VectorFunctor<REAL_T> *>(callable),
        argNames );

    //
    // Expand m_fabs and friends.
    //

    // Copy existing stuff into an expanded m_BLD array.
    for( int l=0; l<GetNumLevels(); ++l )
    {
        BoxLayoutData<REAL_T>::GrowVector(
            &(m_realBLDs[l]),       GetNumComponents() );
        BoxLayoutData<REAL_T>::GrowVector(
            &(m_paddedRealBLDs[l]), GetNumComponents() );
        BoxLayoutData<REAL_T>::GrowVector(
            &(m_BLDs[l]),           GetNumComponents() );
        BoxLayoutData<REAL_T>::GrowVector(
            &(m_paddedBLDs[l]),     GetNumComponents() );
    }

    // Add stuff to m_globalMetaData.
    m_globalMetadata.m_numComponents += 1;
    m_globalMetadata.m_componentNames.push_back( name );

    return 0;
}


/** Overloading -- takes arg nums, instead of names. */
template<class REAL_T> int
VisualizableDataset<REAL_T>::DefineNewComponent(
    string name, VectorFunctorBase * callable,
    vector<int> const & argNums )
{
    vector<string> argNames;
    for( unsigned i=0; i<argNums.size(); ++i )
    {
        unsigned num = argNums[i];
        assert( num < m_globalMetadata.m_componentNames.size() );
        argNames.push_back( m_globalMetadata.m_componentNames[ num ] );
    }
    return DefineNewComponent( name, callable, argNames );
}


/** Deletes the highest-numbered component, which by the way must be synthetic.
 *  Useful for replacing a synthetic
 *  component with another one.  And frankly, this is a hack because if we
 *  really had our memory management under control, it would have been no
 *  problem to just run all the FABs' refcounts to zero.
 *
 *  We won't try to provide a way to delete other components, as renumbering
 *  the remaining components will cause too many problems in too many places.
 *
 *  Returns 0 on success, -1 on error.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::PopSyntheticComponent( string name )
{
    Trace t("VisualizableDataset::PopSyntheticComponent()");
    if( ! m_newComponentSynthesisInfo.ContainsName( name ) )
    {
        t.Error( "Component %s does not exist or is not synthetic.",
                 name.c_str() );
        return -1;
    }
    if( GetComponentNum( name ) != GetNumComponents()-1 )
    {
        t.Error( "Component %s is not the last component.", name.c_str() );
        return -1;
    }

    for( int l=0; l<GetNumLevels(); ++l )
    {
        BoxLayoutData<REAL_T>::ShrinkVectorByOne(
            &(m_realBLDs[l]),       GetNumComponents() );
        BoxLayoutData<REAL_T>::ShrinkVectorByOne(
            &(m_paddedRealBLDs[l]), GetNumComponents() );
        BoxLayoutData<REAL_T>::ShrinkVectorByOne(
            &(m_BLDs[l]),           GetNumComponents() );
        BoxLayoutData<REAL_T>::ShrinkVectorByOne(
            &(m_paddedBLDs[l]),     GetNumComponents() );
    }

    m_newComponentSynthesisInfo.Remove( name );
    m_globalMetadata.m_numComponents -= 1;
    m_globalMetadata.m_componentNames.pop_back();

    return 0;
}


/** Redefines a component previously introduced by DefineNewComponent().
 *  Doesn't work for components that we pull out of the hdf5 file.
 *  Returns 0 on success, -1 on failure.
 *
 *  Won't have any effect unless we take care to release existing data; FABs
 *  will think they're already loaded with the right data.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::RedefineNewComponent(
    string name, VectorFunctorBase * callable,
    vector<string> const & argNames )
{
    Trace t("VisualizableDataset::RedefineNewComponent()");
    if( ! m_newComponentSynthesisInfo.ContainsName( name ) )
    {
        t.Error( "Component %s doesn't exist.  Use DefineNewComponent() "
                 "instead.", name.c_str() );
        return -1;
    }
    PopSyntheticComponent( name );
    DefineNewComponent( name, callable, argNames );
    return 0;
}


/** Overloading -- takes arg nums, instead of names. */
template<class REAL_T> int
VisualizableDataset<REAL_T>::RedefineNewComponent(
    string name, VectorFunctorBase * callable,
    vector<int> const & argNums )
{
    vector<string> argNames;
    for( unsigned i=0; i<argNums.size(); ++i )
    {
        unsigned num = argNums[i];
        assert( num < m_globalMetadata.m_componentNames.size() );
        argNames.push_back( m_globalMetadata.m_componentNames[ num ] );
    }
    return RedefineNewComponent( name, callable, argNames );
}


/** Dumps all the data to an HDF5 file that, when loaded by the ctor,
 *  reconstitutes this VisualizableDataset instance.
 *
 *  Forks.  Parent uses operator<< to dump *this in ascii to a pipe.
 *  Child execs ascii2hdf5, with that pipe as stdin.
 *
 *  Optional arg ascii, when true, causes the data to be dumped out in ascii,
 *  and skips the whole business of piping through the ascii2hdf5 program.
*/
template<class REAL_T> Status
VisualizableDataset<REAL_T>::SaveToHDF5( string outfilename,
                                         bool ascii /*=false*/ ) const
{
    Trace t("VisualizableDataset::SaveToHDF5()");

    // When arg ascii==true, it's very simple:
    if( ascii )
    {
        std::ofstream outfile( outfilename.c_str() );
        operator<<( outfile, *this );
        return STATUS_OK;                            // Early return
    }

    //
    // Arg ascii==false.
    //

    int fd[2];
    if( pipe( fd ) != 0 )
    {
        t.FatalError("(pipe) %s", strerror( errno) );
    }

    pid_t pid = fork();
    if( pid < 0 )
    {
        t.FatalError("(fork) %s", strerror( errno) );
    } else
    if( pid == 0 )      // Child: spawn ascii2hdf5
    {
        close( fd[1] );
        if( -1 == dup2( fd[0], STDIN_FILENO ) )
        {
            t.FatalError("(dup2) %s", strerror( errno) );
        }

        //
        // Comment out this stuff, if you want to see stderr output from the
        // child process.
        //
        int devnull = open( "/dev/null", O_APPEND );
        if( -1 == dup2( devnull, STDERR_FILENO ) )
        {
            t.FatalError("(dup2) %s", strerror( errno) );
        }

        // Better have $CHOMBOVIS_HOME/libexec/ChomboVis on the PATH.
        string program = string(Consts::ascii2hdf5);
        string outfileEquals = string("outfile=") + outfilename;
        string rawMode = string("ascii2hdf5_raw_mode=true");
        ostringstream debugEquals;
        debugEquals << "debug_level=" << Cmdline::DebugLevel();
        execlp( program.c_str(), program.c_str(), outfileEquals.c_str(),
                debugEquals.str().c_str(), rawMode.c_str(), NULL );
        t.FatalError( "(execlp) %s", strerror( errno ) );
    }
    else              // Parent: call operator<<(*this).
    {
/*
        if( -1 == dup2( fd[1], STDOUT_FILENO ) )
        {
            t.FatalError("(dup2) %s", strerror( errno ) );
        }
*/
        close( fd[0] );

        OstreamStdioAdaptor osa( fd[1] );
        SetRawAscii( true ); // Causes operator<< to write field data as raw
                             // floating-point numbers.  Much faster writing
                             // (and, subsequently by the child process
                             // ascii2hdf5, reading).
        operator<<( osa, *this );
        SetRawAscii( false );
        close( fd[1] );       // send EOF to child

        waitpid( pid, 0, 0 ); // prevent zombie
    }

    return STATUS_OK;
}


/** Used by std::transform() below. */
char Blank2Underscore( char c )
{
    return c == ' ' ? '_':c;
}

/** Dumps all the data to the ASCII format our ascii2hdf5 tool accepts.  Writes
 *  to stdout.
 *  In future releases, this function will take arguments to allow us to dump
 *  subsets of the data.
*/
template<class REAL_T, class OSTREAM_T> OSTREAM_T &
operator<<( OSTREAM_T & out, VisualizableDataset<REAL_T> const & visdat )
{
    Trace t("operator<<(VisDat)");

    OldConsts strConsts;
    
    int dim = visdat.GetDimensionality();

    if( dim == 1 )
    {
        out.precision(7);
    } else
    {
        out.precision(16);
    }

    out << Consts::space_dim << ": " << dim << endl;
    out << strConsts.num_levels << ": " << visdat.GetNumLevels() << endl;
    out << Consts::num_components << ": " << visdat.GetNumComponents() << endl;

    out << Consts::component_names << ": ";
    for( int c=0;c<visdat.GetNumComponents();++c )
    {
        string name( visdat.GetComponentName(c) );
        std::transform( name.begin(),name.end(),name.begin(),Blank2Underscore);
        out << name << ' ';
    }
    out << endl;

    out << strConsts.data_centering << ": "
        << DataCenteringIntvect2Int( visdat.GetDataCentering(), dim ) << endl;

    Box prob_domain( visdat.GetProbDomain(0) );
    prob_domain.Shrink( visdat.GetDataCentering(), false );

    out << strConsts.prob_domain << ": ";
    for( int i=0;i<dim;++i )
    {
        out << prob_domain.GetLoCorner(i) << " ";
    }
    for( int i=0;i<dim;++i )
    {
        out << prob_domain.GetHiCorner(i) << " ";
    }
    out << endl;

    out << strConsts.origin << ": ";
    for( int i=0;i<dim;++i )
    {
        out << visdat.GetOrigin()[i] << " ";
    }
    out << endl;

    out << strConsts.anisotropic << ": ";
    for( int i=0;i<dim;++i )
    {
        out << visdat.GetAnisotropic()[i] << " ";
    }
    out << endl;

    for( int level=0;level<visdat.GetNumLevels();++level )
    {
        ostringstream ost;
        ost << strConsts.level_ << level;
        out << ost.str() << ": " << endl;

        REAL_T dx[3];
        visdat.GetDx( dx, level );
        out << "  " << strConsts.dx << ": " << dx[0] << endl;

        out << "  " << strConsts.dt << ": " << visdat.GetDt( level ) << endl;

        out << "  " << Consts::time << ": " << visdat.GetTime( level ) << endl;

        out << "  " << strConsts.output_ghost << ": ";
        Intvect ghost( visdat.GetOutputGhost() );
        for( int i=0;i<dim;++i ) out << ghost[i] << " ";
        out << endl;

        int padded = visdat.GetGhostCellsSuppliedByUser();

        out << "  " << strConsts.boxes << ": " << endl;
        for( int b=0;b<visdat.GetNumRealBoxes(level);++b )
        {
            Box box( visdat.GetsRealBox(level,b,padded) );
            box.Shrink( visdat.GetDataCentering(), false );
            box.Shrink( visdat.GetOutputGhost(), true );
            out << "    ";
            for( int i=0;i<dim;++i ) out << box.GetLoCorner(i) << " ";
            for( int i=0;i<dim;++i ) out << box.GetHiCorner(i) << " ";
            out << endl;
        }

        out << "  " << Consts::data << ": " << endl;
        for( int b=0;b<visdat.GetNumRealBoxes(level);++b )
        {
            for( int c=0; c<visdat.GetNumComponents();++c )
            {
                boost::intrusive_ptr<FAB<REAL_T> > fab(
                    visdat.GetRealFAB(level,b,c,padded) );
                int numCells = fab->GetBox().GetNumCells();
                boost::shared_array<REAL_T> farray = fab->GetFArray();
                // This is the data.  It's in Fortran order, i.e. fastest-
                // turning index is i, then j, then k.

                out << "    ";
                if( visdat.GetRawAscii() )
                {
                    out << '|'; // Delimiter -- makes reading program's job
                                // easier.
                    char * temp = new char[numCells*sizeof(REAL_T)];
                    memcpy( temp, farray.get(), numCells*sizeof(REAL_T) );
                    out.write( temp, numCells*sizeof(REAL_T) );
                    delete [] temp;
                    out << endl;
                } else
                {
                    for( int i=0;i<numCells;++i )
                    {
                        out << farray[i] << " ";
                    }
                    out << endl;
                }
            }
        }
    }

    //
    // Particles
    //
    if( visdat.GetNumParticles() == 0 )
    {
        t.Info( "No particles, returning." );
        return out;                                   // Early return.
    }
    int n( visdat.GetNumParticleComponents() );
    out << Consts::particles << ":" << endl;
    out << "  " << Consts::num_components << ": " << n << endl;
    out << "  " << Consts::num_particles << ": " << visdat.GetNumParticles()
        << endl;
    for( int c=0; c<visdat.GetNumParticleComponents(); ++c )
    {
        out << "  " << Consts::component_ << c << ":" << endl;
        out << "    " << Consts::name << ": " 
            << visdat.GetParticleComponentNames()[c] << endl;
        out << "    " << Consts::values << ": ";
        boost::shared_array<REAL_T> data( visdat.GetParticleComponent(
                                    visdat.GetParticleComponentNames()[c]) );
        for( int i=0;i<visdat.GetNumParticles();++i )
        {
            out << data[i] << " ";
        }
        out << endl;
    }

    return out;
}


//
// Members of private interior class 
// VisualizableDataset::NewComponentSynthesisInfo.
// Purpose of this class is for VisualizableDataset to know how to synthesize
// data for components that are not to be found in the hdf5 file but rather
// have been created on the fly with the DefineNewComponent() method.
// The class is not much more than a friendly interface to a
// map<string,pair<VectorFunctor*,vector<string>>>.
// 


template<class REAL_T>
VisualizableDataset<REAL_T>::NewComponentSynthesisInfo::~NewComponentSynthesisInfo()
{
    // Delete VectorFunctors (if any).
    for( typename SynthesisInfoRep::const_iterator iter = m_rep.begin();
        iter != m_rep.end();
        ++iter )
    {
        delete iter->second.first;
    }
}


/** Store the info needed to synthesize the component named "name". */
template<class REAL_T> void
VisualizableDataset<REAL_T>::NewComponentSynthesisInfo::Insert(
    string name, VectorFunctor<REAL_T> const * callable,
    vector<string> argNames )
{
    assert( ! ContainsName( name ) );
    m_rep[name] = make_pair<VectorFunctor<REAL_T> const *,
                            vector<string> >( callable, argNames );
}


/** Replace the info needed to synthesize the component named "name". */
template<class REAL_T> void
VisualizableDataset<REAL_T>::NewComponentSynthesisInfo::Replace(
    string name, VectorFunctor<REAL_T> const * callable,
    vector<string> argNames )
{
    assert( ContainsName( name ) );
    m_rep[name] = make_pair<VectorFunctor<REAL_T> const *,
                            vector<string> >( callable, argNames );
}


template<class REAL_T> void
VisualizableDataset<REAL_T>::NewComponentSynthesisInfo::Remove( string name )
{
    assert( ContainsName( name ) );
    m_rep.erase(name);
}


template<class REAL_T> bool
VisualizableDataset<REAL_T>::NewComponentSynthesisInfo::ContainsName(
    string name ) const
{
    return m_rep.find(name) != m_rep.end();
}


template<class REAL_T> VectorFunctor<REAL_T> const *
VisualizableDataset<REAL_T>::NewComponentSynthesisInfo::GetCallable(
    string name ) const
{
    assert( ContainsName( name ) );
    typename SynthesisInfoRep::const_iterator iter = m_rep.find( name );
    return iter->second.first;
}


template<class REAL_T> vector<string> const &
VisualizableDataset<REAL_T>::NewComponentSynthesisInfo::GetArgNames(
    string name ) const
{
    assert( ContainsName( name ) );
    typename SynthesisInfoRep::const_iterator iter = m_rep.find( name );
    return iter->second.second;
}


/** Arg real refers to our real/subdivided distinction.  Subdivided BoxLayouts
 *  are only of interest in visualization, where we often want to cut out zones
 *  of coarse data that are covered by fine data.  Thus arg real defaults to
 *  true.
 *
 *  Arg contrapad, when true, means we want FABs that have ghost cells when the
 *  hdf5 file doesn't provide them, or FABs that do not have ghost cells when
 *  the hdf5 file does provide them.  Default value is false.
 *
 *  The return value is a reference to one of the BoxLayoutDatas that make up
 *  this VisualizableDataset, so -- watch out -- modify it and you will get
 *  different pictures in ChomboVis.
 *
 *  To obtain an independent BoxLayoutData object (as opposed to just a shared
 *  reference) call BoxLayoutData::Clone() on the pointer this function returns.
 *
 *  Before returning the BoxLayoutData&, this function makes sure all the
 *  FABs are filled with data.
*/
template<class REAL_T> boost::shared_ptr< BoxLayoutDataInterfaceForPython >
VisualizableDataset<REAL_T>::GetBoxLayoutData(
    int level, int component,
    bool real/*=true*/,
    bool contrapad/*=false*/ ) const
{
    Trace t("VisDat::GetBoxLayoutData()");

    bool padded( GetGhostCellsSuppliedByUser() ^ contrapad );
    int nBoxes( real ? GetNumRealBoxes(level) : GetNumBoxes(level) );

    // Return the element of m_[padded|][Real|]BLDs.
    boost::shared_ptr< BoxLayoutData<REAL_T> > result;
    if( real )
    {
        if( padded )
        {
            result = m_paddedRealBLDs[level][component];
        } else
        {
            result = m_realBLDs[level][component];
        }
    } else
    {
        if( padded )
        {
            result = m_paddedBLDs[level][component];
        } else
        {
            result = m_BLDs[level][component];
        }
    }

    // Call Get*FAB() to load those FABs, of the BoxLayoutData we want to
    // return, that haven't been loaded yet.  Otherwise, we don't want to
    // increment the FAB refcounts.  We increment the BoxLayoutData refcount.
    // BoxLayoutData::Release() decrements the BoxLayoutData ref count but not
    // the ref counts of the FABs in the BoxLayoutData.

    boost::intrusive_ptr< FAB<REAL_T> > tempFab;
    for( int b=0;b<nBoxes;++b )
    {
        if( real && RealFabIsEmpty(level, b, component, padded) )
        {
            result->SetFAB(b, GetRealFAB( level, b, component, padded));
            assert( !RealFabIsEmpty(level, b, component, padded) );
        } else
        if( (!real) && FabIsEmpty(level, b, component, padded) )
        {
            result->SetFAB(b, GetFAB( level, b, component, padded));
            assert( !FabIsEmpty(level, b, component, padded) );
        }
    }
    return result;
}


/** BoxLayouts are always returned unpadded, whether there are ghost cells
 *  supplied in the data or not.
*/
template<class REAL_T> BoxLayout<REAL_T> const *
VisualizableDataset<REAL_T>::GetBoxLayout(
    int level,
    bool real/*=true*/ ) const
{
    if( real )
    {
        return &m_realBoxLayouts[level];
    } else
    {
        return &m_boxLayouts[level];
    }
}


/** This is of interest to the Python wrapping, where we convert these nested
 *  vectors into correspondingly nested tuples.
 *  Dimensionality assumed to be 2, if locorner[k]==hicorner[k].
 *  The nested vectors represent...
 *  ...                layout     lo&hi corners    corner (i,j,[k])      */
template<class REAL_T> vector<    vector<          vector<int> > >
VisualizableDataset<REAL_T>::GetBoxLayoutAsNestedVectors(
    int level,
    bool real/*=true*/ ) const
{
    return this->GetBoxLayout(level,real)->GetAsNestedVectors();
}


/** Returns 1 if REAL_T is a float, 2 if it's a double.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetPrecision() const
{
    Trace t("VisualizableDataset::GetPrecision()");
    assert( sizeof(float) != sizeof(double) );
    if( sizeof(REAL_T) == sizeof(double) )
    {
        return 2;
    } else
    if( sizeof(REAL_T) == sizeof(float) )
    {
        return 1;
    } else
    {
        t.FatalError( "REAL_T is neither float nor double.  Yikes!" );
        return -1;  // prevents compiler warning
    }
}


/** Private.  Does not result in a usable object.  For use in Slice(). */
template<class REAL_T>
VisualizableDataset<REAL_T>::VisualizableDataset()
  : m_BLDs(0), m_realBLDs(0), m_paddedBLDs(0), m_paddedRealBLDs(0),
    m_boxFinder(0),
    m_realBoxFinder(0),
    m_skipBoxSubdivision(false),
    m_userSuppliedGhost(0,0,0),
    m_generatedGhost(0,0,0),
    m_optimizationMode( 0 ),
    m_rawAscii(false)
{
}


/** Produces a new VisualizableDataset that's the same as this one but is made
 *  up of only those boxes, or parts of boxes, that lie within arg croppingBox
 *  (whose corners are interpreted in the index space of level 0).  Arg
 *  cropping Box refers to values, not (necessarily) to cells; this is of rele-
 *  vance for non-cell-centered data.  It's consistent with the design principle
 *  that as much as possible we pretend all data is cell-centered.  Right after
 *  we load the data (in this->InitRealBoxLayout()) we grow the boxes (i.e.
 *  their dimensions as loaded from the hdf5 file -- where they do refer to
 *  cells) by one cell in each direction the data is not cell-centered.  After
 *  that, we can pretend it's all cell-centered.  Knowledge of data-centering
 *  then should only appear (1) at the user interface (notice, for example, how
 *  in visualizable_dataset.py the cropping box *does* refer to cells), and (2)
 *  in visualization (where we clip away half-cell overhangs, shift things,
 *  etc).
 *
 *  The problem domain, and the origin, remain unchanged: this is to make it
 *  easy to recall which part of the problem domain the data comes from.
 *
 *  The resulting VisualizableDataset is connected to the same HDF5 file as
 *  is *this.  But, the HDF5 file (i.e. the m_chomboHDF5 member) is only called
 *  upon for retrieving particle data.  All its field data -- the real
 *  uncontrapadded BoxLayoutData members that is -- are loaded up with data.
 *
 *  Returns NULL, if croppingBox doesn't intersect any boxes at all, or if 
 *  croppedLevels mentions all the levels, or if croppedComponents mentions all
 *  the components.  If croppingBox doesn't intersect boxes beyond a certain
 *  level, then the result is a VisualizableDataset with fewer levels than
 *  *this.
 *
 *  If optional arg croppedLevels!=NULL and croppedLevels->size()>0, then we
 *  crop to only the levels mentioned in *croppedLevels.  Otherwise (i.e.
 *  croppedLevels==NULL or croppedLevels->size()==0) we save all the levels.
 *  I know it looks bad and inconsistent that croppedLevels->size()==0 means
 *  "save all the levels", but it reflects a compromise with consistency goals
 *  down in python/module_templates.cpp.
 *
 *  Optional arg croppedComponents works analogously to croppedLevels: if it's
 *  NULL, we leave in all the components.  Otherwise, we throw away all those
 *  components not mentioned in the argument.
*/
template<class REAL_T> boost::shared_ptr<VisualizableDatasetInterfaceForPython>
VisualizableDataset<REAL_T>::CropToGeneralBox(
    Box const & croppingBox,
    vector<int> const * croppedLevels /*=0*/,
    vector<string> const * croppedComponents /*=0*/ ) const
{
    Trace t("VisualizableDataset::CropToGeneralBox()");

    boost::shared_ptr<VisualizableDataset<REAL_T> > result(
        new VisualizableDataset<REAL_T> );

    if( croppedLevels && (croppedLevels->size() == 0) )
    {
        croppedLevels = 0;
    }
    if( croppedComponents && (croppedComponents->size() == 0) )
    {
        croppedComponents = 0;
    }

    //
    // Sanity check on optional arg croppedLevels: must be consecutive, and
    // must not refer to any nonexistent levels.
    //
    if( croppedLevels )
    {
        int lo = (*croppedLevels)[0];
        if( lo < 0 )
        {
            t.FatalError( "Illegal level: %d", lo );
        }
        for( unsigned i=0;i<croppedLevels->size();++i )
        {
            assert( (*croppedLevels)[i] == lo+int(i) );
        }
        assert( (*croppedLevels)[croppedLevels->size()-1]
                < m_globalMetadata.m_numLevels );

        if( croppedLevels->size() == 0 )
        {
            croppedLevels = 0;
        }
    }

    //
    // Sanity check on optional arg croppedComponents: must not refer to any
    // nonexisting components, and must not contain duplicates.
    //
    if( croppedComponents )
    {
        for( unsigned i=0; i<croppedComponents->size(); ++i )
        {
            assert( GetComponentNum( (*croppedComponents)[i] ) != -1 );
        }
        vector<string> copiedComps( croppedComponents->begin(),
                                    croppedComponents->end() );
        std::sort( copiedComps.begin(), copiedComps.end() );
        std::unique( copiedComps.begin(), copiedComps.end() );
        if( croppedComponents->size() != copiedComps.size() )
        {
            t.FatalError( "Arg croppedComponents contains a duplicate." );
        }
    }

    //
    // Metadata
    //
    result->m_globalMetadata = m_globalMetadata;
    result->m_globalMetadata.Crop( croppedComponents );
    if( result->m_globalMetadata.m_numComponents == 0 )
    {
        boost::shared_ptr<VisualizableDatasetInterfaceForPython> nullValue;
        return nullValue;                                   // Early return
    }
    if( croppedLevels )
    {
        result->m_globalMetadata.m_numLevels = croppedLevels->size();
    }

    int numCroppedLevels(0);
    for( int lev=0; lev<GetNumLevels(); ++lev )
    {
        if( croppedLevels )
        {
            if( find(croppedLevels->begin(), croppedLevels->end(), lev )
            ==  croppedLevels->end() )
            {
                continue;                    // Early return to head of the loop
            }
        }

        LevelMetadata<REAL_T> levelMeta( m_levelMetadataVector[lev] );
        int refinementFactor(1);
        if( lev > 0 )
        {
            refinementFactor = int(
                m_levelMetadataVector[0].m_dx/m_levelMetadataVector[lev].m_dx
              + 0.5 );
        }
        Box scaledCroppingBox( croppingBox );
        scaledCroppingBox.Scale( refinementFactor );
        levelMeta.Crop( scaledCroppingBox );
        if( levelMeta.m_numBoxes == 0 )
        {
            result->m_globalMetadata.m_numLevels = numCroppedLevels;
            t.Info( "Breaking out of slicing loop at level %d.", lev );
            if( numCroppedLevels == 0 )
            {
                boost::shared_ptr<VisualizableDatasetInterfaceForPython>
                    nullValue;
                return nullValue;                        // Early return
            }
            break;                               // Early break out of loop
        } else
        {
            ++numCroppedLevels;
        }

        result->m_levelMetadataVector.push_back( levelMeta );
    }

    result->m_particleMetadata = m_particleMetadata;


    //
    // Ghosts
    //
    result->m_userSuppliedGhost = result->GetOutputGhost();
    if( result->m_userSuppliedGhost == Intvect(0,0,0) )
    {   // Go with generated ghosts then.
        result->m_generatedGhost = Intvect(1,1,0);
    } else
    {
        result->m_generatedGhost = Intvect(0,0,0);
    }


    // Load data into the real non-contrapadded FABs.  We're not doing the
    // out-of-core thing here; result is unconnected to any HDF5 file.
    result->m_boxLayouts.reserve( result->GetNumLevels() );
    result->m_realBoxLayouts.reserve( result->GetNumLevels() );
    for( int lev=0;lev<result->GetNumLevels();++lev )
    {
        result->InitRealBoxLayout(lev);
    }
    for( int lev=0;lev<result->GetNumLevels();++lev )
    {
        result->InitBoxLayoutData( lev );

        // lev indexes levels in the cropped VisualizableDataset we are building
        // up, while thisLev indexes into *this.
        int thisLev;
        if( croppedLevels )
        {
            thisLev = (*croppedLevels)[lev];
        } else
        {
            thisLev = lev;
        }

        int c1(-1); // Index into components of *this.
        int c2(-1); // Index into components of result -- will be different from
                    // from c1 if we're cropping away some components.
        for( c1=0;c1<GetNumComponents();++c1 )
        {
            if( (!croppedComponents)
            ||  (std::find( croppedComponents->begin(),
                            croppedComponents->end(),
                            GetComponentNames()[c1] )
                    != croppedComponents->end() ) )
            {
                ++c2;
            } else
            {
                continue;
            }
                
            boost::shared_ptr< BoxLayoutDataInterfaceForPython > croppedBLD(
                GetBoxLayoutData( thisLev,c1 )->Clone() );

            int refinementFactor(1);
            if( thisLev > 0 )
            {
                refinementFactor = int(
                     m_levelMetadataVector[0].m_dx
                    /m_levelMetadataVector[thisLev].m_dx
                      + 0.5 );
            }
            Box scaledCroppingBox( croppingBox );
            scaledCroppingBox.Scale( refinementFactor );
            croppedBLD->CropToGeneralBox( scaledCroppingBox );

            boost::shared_ptr< BoxLayoutData<REAL_T> > * bldptr;
            if( result->GetGhostCellsSuppliedByUser() )
            {
                bldptr = &(result->m_paddedRealBLDs[lev][c2]);
            } else
            {
                bldptr = &(result->m_realBLDs[lev][c2]);
            }
            *bldptr = boost::static_pointer_cast< BoxLayoutData<REAL_T> >(
                                                                    croppedBLD);
            (*bldptr)->SetLevel( thisLev );
        }
    }

    //
    // Particles
    //
    for( unsigned c=0; c<m_particleMetadata.m_componentNames.size(); ++c )
    {
        string name = m_particleMetadata.m_componentNames[c];
        result->m_particleComponents[name].reset(0);
    }
    if( m_chomboHDF5 ) // False, if this is the result of a Slice().
    {
        result->m_chomboHDF5 = m_chomboHDF5; // Only used for particles.
    }

    return result;    
}

/** Produces a VisualizableDataset that's a 2D slice of this one -- sliced
 *  through arg axis at arg axisPosition -- and returns a pointer to it.
 *
 *  Returns 0 if there is no data at all, at this slice's position.
 *  
 *  The new VisualizableDataset is unconnected to any HDF5 file.  All its
 *  data, except for generated components, is filled.
*/
template<class REAL_T> boost::shared_ptr<VisualizableDatasetInterfaceForPython>
VisualizableDataset<REAL_T>::Slice( char axis, double axisPosition ) const
{
    Trace t("VisualizableDataset::Slice()");
    assert( m_globalMetadata.m_dimensionality == 3 );
    
    boost::shared_ptr<VisualizableDataset<REAL_T> > result(
        new VisualizableDataset<REAL_T> );

    //
    // Metadata
    //
    result->m_globalMetadata = m_globalMetadata;
    result->m_globalMetadata.Slice( axis );
    result->m_levelMetadataVector.reserve( GetNumLevels() );
    vector<map<int,int> > slicedBoxesMaps( GetNumLevels() );
    typename vector<map<int,int> >::iterator sbmIter
        = slicedBoxesMaps.begin();
    for( int lev=0; lev<GetNumLevels(); ++lev )
    {
        LevelMetadata<REAL_T> levelMeta( m_levelMetadataVector[lev] );
        levelMeta.Slice( axis, axisPosition,
                         m_globalMetadata.m_anisotropic,
                         m_globalMetadata.m_origin, sbmIter );
        if( levelMeta.m_numBoxes == 0 )
        {
            result->m_globalMetadata.m_numLevels = lev;
            t.Info( "Breaking out of slicing loop at level %d.", lev );
            break;                               // Early break out of loop
        }

        result->m_levelMetadataVector.push_back( levelMeta );
        ++sbmIter;
    }

    // Deal with the case where there is no 2D data at all, at this
    // axisPosition:
    if( result->m_globalMetadata.m_numLevels == 0 )
    {
        t.Error( "There is no 2D data at %f on the %c axis.",
                 axisPosition, axis );
        boost::shared_ptr<VisualizableDatasetInterfaceForPython> nullValue;
        return nullValue;                                     // Early return
    }


    //
    // Ghosts
    //
    result->m_userSuppliedGhost = result->GetOutputGhost();
    if( result->m_userSuppliedGhost == Intvect(0,0,0) )
    {   // Go with generated ghosts then.
        result->m_generatedGhost = Intvect(1,1,0);
    } else
    {
        result->m_generatedGhost = Intvect(0,0,0);
    }

    // Load sliced data into the FABs; we aren't doing the out-of-core thing
    // here.
    result->m_boxLayouts.reserve( result->GetNumLevels() );
    result->m_realBoxLayouts.reserve( result->GetNumLevels() );
    for( int lev=0;lev<result->GetNumLevels();++lev )
    {
        result->InitRealBoxLayout(lev);
    }

    typename FAB<REAL_T>::CtorArgs fabCtorArgs;

    for( int lev=0;lev<result->GetNumLevels();++lev )
    {
        fabCtorArgs.level = lev;
        result->InitBoxLayoutData(lev);

        for( int c=0;c<GetNumComponents();++c )
        {
            fabCtorArgs.component = c;
            for( int b=0;b<result->GetNumRealBoxes(lev);++b )
            {
                boost::intrusive_ptr< FAB<REAL_T> > slicedFab;
                boost::intrusive_ptr< FAB<REAL_T> > unslicedFab;

                fabCtorArgs.box = (*(result->GetBoxLayout(lev,true)))[b];
                fabCtorArgs.dx = result->GetBoxLayout(lev,true)->GetDx();
                fabCtorArgs.origin = result->GetOrigin();

                if( result->GetGhostCellsSuppliedByUser() )
                {
                    // Get padded, real FAB
                    unslicedFab =
                        GetRealFAB( lev, slicedBoxesMaps[lev][b], c, true );
                    boost::shared_array<REAL_T> slicedFArray(
                        unslicedFab->GetSlicedFArray( axis, axisPosition) );

                    fabCtorArgs.fieldData = slicedFArray;
                    fabCtorArgs.real = true;
                    fabCtorArgs.padded = true;
                    fabCtorArgs.ghost = result->m_userSuppliedGhost;

                    result->m_paddedRealBLDs[lev][c]->SetFAB(
                        b,
                        boost::intrusive_ptr<FAB<REAL_T> >(
                            new FAB<REAL_T>( fabCtorArgs ) ) );
                    slicedFab = result->m_paddedRealBLDs[lev][c]->GetFAB(b);

                } else
                {
                    // Get unpadded, real FAB
                    unslicedFab =
                        GetRealFAB( lev, slicedBoxesMaps[lev][b], c, false );
                    boost::shared_array<REAL_T> slicedFArray(
                        unslicedFab->GetSlicedFArray( axis, axisPosition) );

                    fabCtorArgs.fieldData = slicedFArray;
                    fabCtorArgs.real = true;
                    fabCtorArgs.padded = false;
                    fabCtorArgs.ghost = Intvect(0,0,0);

                    result->m_realBLDs[lev][c]->SetFAB(
                        b,
                        boost::intrusive_ptr<FAB<REAL_T> >(
                            new FAB<REAL_T>( fabCtorArgs ) ) );
                    slicedFab = result->m_realBLDs[lev][c]->GetFAB(b);
                }
            }
        }
    }

    return result;
}


/** Does not go to m_chomboHDF5.GetHyperslabSize(), because in sliced
 *  VisualizableDataset's m_chomboHDF5 is NULL.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::GetHyperslabSize( int level, int boxNum ) const
{
    Box box( GetsRealBox( level, boxNum, GetGhostCellsSuppliedByUser() ));
    return box.GetNumCells();
}


/** Return n pairs of equally spaced points on the line connecting p0 and p1
 *  (where the first and last points are p0 and p1 themselves).  In each pair,
 *  the first element is the fraction of the distance along the line
 *  (0,1/(n-1), 2/(n-1),...,1), and the second element is the value, at that
 *  point, of the indicated field component.
 *
 *  Args p0 and p1 are points in 3D space.
 *  Arg n must be greater than 1.
 *
 *  In the event that a coordinate falls out of bounds, we skip it and just go
 *  on.  We issue a Warning (visible at debug_level 3 and above) about it.  If
 *  you view your plot without connecting the dots you'll see a gap in the dots.
*/
template<class REAL_T> vector<pair<REAL_T,REAL_T> >
VisualizableDataset<REAL_T>::GetLinePlotGeneric(
                           Triple<REAL_T> const & p0, Triple<REAL_T> const & p1,
                           int n, int component, int finestLevel ) const
{
    Trace t("VisualizableDataset::GetLinePlotGeneric()");
    assert( n>1 );

    vector<pair<REAL_T,REAL_T> > result;
    result.reserve( n );
    Status status;
    for( int i=0;i<n;++i )
    {
        REAL_T u( i/(n-1.0) );
        Triple<REAL_T> p( p0*(1-u) + p1*u );
        REAL_T v( GetDatumXYZ( finestLevel, component, p, &status ) );
        if( status != STATUS_OK )
        {
            t.Warning( "Coordinates (%f,%f,%f) are out of bounds",
                       p[0],p[1],p[2] );
        } else
        {
            result.push_back( std::make_pair( u,v ) );
        }
    }

    return result;
}


/** We call this from the Python interface; can't make that generic,
 *  unfortunately.
*/
template<class REAL_T> vector<pair<double,double> >
VisualizableDataset<REAL_T>::GetLinePlot(
                       Triple<double> const & dp0, Triple<double> const & dp1,
                       int n, int component, int finestLevel ) const
{
    // Convert from double to REAL_T and then call the generic version.
    Triple<REAL_T> rp0, rp1;
    for( int i=0;i<3;++i )
    {
        rp0.m_data[i] = dp0.m_data[i];
        rp1.m_data[i] = dp1.m_data[i];
    }
    vector<pair<REAL_T,REAL_T> > rResult(
        GetLinePlotGeneric( rp0,rp1,n,component,finestLevel) );

    if( rResult.empty() ) // All coordinates out of bounds!
    {
        vector<pair<double,double> > dummy;
        return dummy;                              // Early return
    }

    vector<pair<double,double> > dResult;
    dResult.reserve( rResult.size() );
    for( unsigned int i=0;i<rResult.size();++i )
    {
        dResult.push_back( std::make_pair( double(rResult[i].first),
                                           double(rResult[i].second) ) );
    }
    return dResult;
}


/** For physical location xyz, find the signed distance to the ebID-th embedded
 *  boundary, using data at the indicated level.  Use the component_<ebID>
 *  component of the hdf5 file, and interpolate to your exact spot.
*/
template<class REAL_T> REAL_T
VisualizableDataset<REAL_T>::GetDistanceToEB(
    REAL_T x, REAL_T y, REAL_T z,
    int level,
    int ebID ) const
{
    assert(0); // Not ready for prime time!

    Trace t("VisualizableDataset::GetDistanceToEB()");
    REAL_T const hugeNum( 1E100 );

    ostringstream ost; // FIXME: don't figure this out every time.
    ost << "component_" << ebID;
    int componentNum( GetComponentNum( ost.str() ) );
    Status status;
    Intvect ijk( XYZ2IJK(level, Triple<REAL_T>(x,y,z), &status) );
    if( status == STATUS_NOT_FOUND )
    {
        t.Warning( "(%d,%d,%d) outside domain, returning hugeNum",
                    ijk[0], ijk[1], ijk[2] );
        return hugeNum;
    }
    if( m_globalMetadata.m_dimensionality == 2 )
    {
        ijk[2] = 0;
    }

    int boxNum( GetEnclosingBoxNum( level, ijk.m_data, true ) );
    if( boxNum == -1 ) // ijk outside prob domain at this level
    {
        return hugeNum;
    }

    for( int m=0;m<m_globalMetadata.m_dimensionality;++m )
    {
        ijk[m] -= GetsRealBox( level, boxNum, false ).GetLoCorner()[m];
    }

    REAL_T result;
    result = GetDatum( level, boxNum, componentNum, ijk );
    t.Warning( "distance(%f,%f,%f)=%f", x, y, z, result );
    return result;
}


/** Copies arg to a vector, to keep its ref count one more than it would
 *  otherwise be.
 *  Returns item "refcount" (not including this call) on m_pinnedFabs.
 *
 *  Const but modifies mutable m_pinnedFabs.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::PinFAB(
    boost::intrusive_ptr< FAB<REAL_T> > const & fab ) const
{
    return m_pinnedFabs.Push( fab );
}


/** See PinFAB().
 *  Returns item "refcount" (not including this call) on m_pinnedFabs.
 *
 *  Const but modifies mutable m_pinnedFabs.
*/
template<class REAL_T> int
VisualizableDataset<REAL_T>::UnpinFAB(
    boost::intrusive_ptr< FAB<REAL_T> > const & fab ) const
{
    return m_pinnedFabs.Pop( fab );
}


/** 0=memory, 1=speed */
template<class REAL_T> void
VisualizableDataset<REAL_T>::SetOptimizationMode( int memory_or_speed )
{
    m_optimizationMode = memory_or_speed;
}


/** Delete FABs whose use_count==1. */
template<class REAL_T> void
VisualizableDataset<REAL_T>::FreeUnusedFABs()
{
    Trace t("VisualizableDataset::FreeUnusedFABs()");
    if( m_optimizationMode == 0 )
    {
        for( int lev=0;lev<GetNumLevels();++lev )
        {
            for( int comp=0;comp<GetNumComponents();++comp )
            {
//                t.Info( "m_BLDs[lev=%d][comp=%d]->FreeUnusedFABs()...",
//                        lev, comp );
                m_BLDs[lev][comp]->FreeUnusedFABs();

//                t.Info( "m_realBLDs[lev=%d][comp=%d]->FreeUnusedFABs()...",
//                        lev, comp );
                m_realBLDs[lev][comp]->FreeUnusedFABs();

//                t.Info( "m_paddedBLDs[lev=%d][comp=%d]->FreeUnusedFABs()...",
//                        lev, comp );
                m_paddedBLDs[lev][comp]->FreeUnusedFABs();

//              t.Info("m_paddedRealBLDs[lev=%d][comp=%d]->FreeUnusedFABs()...",
//                        lev, comp );
                m_paddedRealBLDs[lev][comp]->FreeUnusedFABs();
            }
        }
    }   
}


template<class REAL_T> void
VisualizableDataset<REAL_T>::PrintFabUseCounts() const
{
    Trace t("VisualizableDataset::PrintFabUseCounts()");
    if( m_optimizationMode == 0 )
    {
        for( int lev=0;lev<GetNumLevels();++lev )
        {
            for( int comp=0;comp<GetNumComponents();++comp )
            {
                if( m_BLDs[lev][comp]->GetTotalFabUseCount() > 0 )
                {
                    t.Info( "m_BLDs[lev=%d][comp=%d]->PrintFabUseCounts():",
                            lev, comp );
                    m_BLDs[lev][comp]->PrintFabUseCounts();
                }

                if( m_realBLDs[lev][comp]->GetTotalFabUseCount() > 0 )
                {
                    t.Info( "m_realBLDs[lev=%d][comp=%d]->PrintFabUseCounts():",
                            lev, comp );
                    m_realBLDs[lev][comp]->PrintFabUseCounts();
                }


                if( m_paddedBLDs[lev][comp]->GetTotalFabUseCount() > 0 )
                {
                    t.Info( "m_paddedBLDs[lev=%d][comp=%d]->PrintFabUseCounts():",
                            lev, comp );
                    m_paddedBLDs[lev][comp]->PrintFabUseCounts();
                }

                if( m_paddedRealBLDs[lev][comp]->GetTotalFabUseCount() > 0 )
                {
                    t.Info("m_paddedRealBLDs[lev=%d][comp=%d]->PrintFabUseCounts():",
                            lev, comp );
                    m_paddedRealBLDs[lev][comp]->PrintFabUseCounts();
                }
            }
        }
    }
}


template<class REAL_T> vector<int> const &
VisualizableDataset<REAL_T>::GetConstituentSubdividedBoxnums(
    int level, int realBoxNum ) const
{
    map<int, vector<int> > const & boxMap( m_real2subdividedBoxes[level] );
    map<int, vector<int> >::const_iterator fIter( boxMap.find(realBoxNum) );
    static vector<int> emptyVector;
    if( fIter == boxMap.end() )
    {
        return emptyVector;
    } else
    {
        return fIter->second;
    }
}


//
// Explicit template instantiations
//
template class VisualizableDataset<float>;
template class VisualizableDataset<double>;
using std::ostream;
template ostream & operator<<( ostream &, VisualizableDataset<float> const & );
template ostream & operator<<( ostream &, VisualizableDataset<double> const & );
