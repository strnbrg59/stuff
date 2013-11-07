
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
#include <cassert>
#include <iostream>
#include <algorithm>
#include "BoxFinder.h"
#ifndef DO_DEMO
#include "Misc.h"
#include "Trace.h"
#endif

using std::endl;
using std::set;
using std::cerr;

BoxFinder::BoxFinder( int dimensionality )
  : m_nDims( dimensionality ),
    m_lastLevelFound( k_noBox )
{
}


/** Returns true if the BoxFinder has been loaded with BoxCorner data for the
 *  indicated level.  Only once primed is the BoxFinder able to find the box
 *  that encloses a given point.
*/
bool
BoxFinder::IsPrimed( int level )
{
    return m_byLevelByAxisCutoffs.find( level ) != m_byLevelByAxisCutoffs.end();
}


/** Assemble all the box cutoff information you'll ever need, to identify a
 *  box on the indicated level.  
 *
 *  Arg cornerMap is a map whose keys are box ID numbers (within this level)
 *  and whose values are extents in {i_lo, j_lo, k_lo, i_hi, j_hi, k_hi}
 *  format.  Extents are always 6 long, regardless of the dimensionality of the
 *  underlying data.
 *
 *  BoxFinder has no access to the hdf5 file or any of the routines that access
 *  it; arg cornerMap is the only place to tell BoxFinder about the structure
 *  of the hdf5 data.
*/
void
BoxFinder::Prime( int level, map<int, vector<int> > cornerMap )
{
#ifndef DO_DEMO
    Trace t("BoxFinder::Prime()");
#endif

    m_cornerMaps[level] = cornerMap;
    typedef map<int, BoxPhases> BoxPhaseMap;  // One axis-worth
    vector<BoxPhaseMap> axisCutoffs(m_nDims);
        // We'll insert these into m_byLevelByAxisCutoffs after filling them.
    for( int m=0; m<m_nDims; ++m )
    {
        if( cornerMap.size() == 0 )
        {   // Happens if one level completely covers next-coarser level (we're
            // working with subdivided boxes here).
            m_byLevelByAxisCutoffs[level].push_back(
                CutoffVector( axisCutoffs[m].begin(), axisCutoffs[m].end() ) );
            continue;
        }

        // For each box at this level, check if there's already a BoxPhases
        // object with associated position equal to the box's lo_corner or
        // hi_corner.  (Of course, initially, there will be none.)  If there is,
        // then insert box's lo_corner or hi_corner into the appropriate member
        // of the BoxPhases object.  If there isn't, then construct a
        // BoxPhases object and put it in the collection.
        for( unsigned iBox=0; iBox < cornerMap.size(); ++iBox )
        {
            assert( cornerMap.find(iBox) != cornerMap.end() );

            for( int lohi=0; lohi<2; ++lohi )
            {
                int notch = cornerMap.find(iBox)->second[m + lohi*THREE];
                if( axisCutoffs[m].find( notch ) == axisCutoffs[m].end() )
                {
                    axisCutoffs[m].insert(
                        std::make_pair(notch, BoxPhases()));
                }
                if( lohi == 0 )
                {
                    axisCutoffs[m][ notch ].m_boxStarts.insert( iBox);
                } else
                {
                    axisCutoffs[m][ notch ].m_boxEnds.insert( iBox);
                }
            }
        }

        // Now fill in m_boxContinuations: calculate it as the set difference
        // between the union of m_boxStarts and m_boxContinuations as of the
        // previous position, and the union of m_boxEnds as of the previous and
        // the current position.
        BoxPhaseMap::iterator prevIter, curIter;
        for( prevIter = axisCutoffs[m].begin(),
             curIter  = ++(axisCutoffs[m].begin());

             curIter != axisCutoffs[m].end();

             ++prevIter, ++curIter )
        {
            assert( curIter->second.m_boxContinuations.empty() );

            set<int> beginContinuationUnion; // union of m_begin & m_cont...
            set_union(
                prevIter->second.m_boxStarts.begin(),
                prevIter->second.m_boxStarts.end(),
                prevIter->second.m_boxContinuations.begin(),
                prevIter->second.m_boxContinuations.end(),
                inserter( beginContinuationUnion,
                          beginContinuationUnion.begin() ));

            set<int> endEndUnion; // union of prev.m_end & cur.m_end...
            set_union(
                prevIter->second.m_boxEnds.begin(),
                prevIter->second.m_boxEnds.end(),
                curIter->second.m_boxEnds.begin(),
                curIter->second.m_boxEnds.end(),
                inserter( endEndUnion, endEndUnion.begin() ));

            set<int> unionEndsDiffs;
            set_difference(
                beginContinuationUnion.begin(),
                beginContinuationUnion.end(),
                endEndUnion.begin(),
                endEndUnion.end(),
                inserter( unionEndsDiffs, unionEndsDiffs.begin() ));

            curIter->second.m_boxContinuations.insert(
                unionEndsDiffs.begin(), unionEndsDiffs.end() );
        }

        // Now copy all the pairs in axisCutoffs into the vector
        // m_byLevelByAxisCutoffs[level].  We use a vector because STL's
        // upper_bound function is much faster on a (sorted) vector than on a
        // map.  But we packed the data into a map, above, because it was more
        // convenient to navigate the elements of a map.
        m_byLevelByAxisCutoffs[level].push_back(
            CutoffVector( axisCutoffs[m].begin(), axisCutoffs[m].end() ) );
    }
}

/** Function object we pass into FindBox(). */
struct CutoffComparator
{
    inline bool operator()( Cutoff const & i, Cutoff const & j )
    {
        return (i.first <= j.first);
    }
};


/** Returns the number (within the indicated level) of the box that contains
 *  the cell having the indicated coordinates.
 *  Returns -1 if there is no box (at least on this level) that contains the
 *  cell.
 *
 *  If m_nDims==2, we ignore the third coordinate.
 *
 *  Arg guess is optional.  It's where you can indicate which box you think the
 *  point is in -- big speedup when filling in ghost cells.  Even if you don't
 *  supply a guess, we try m_lastBoxFound[level] as a guess.
 *
 *  Must not be called before Prime( level ).
*/
int
BoxFinder::FindBox( int level, int const coords[THREE], int const * guess )
{
#ifndef DO_DEMO
    Trace t("BoxFinder::FindBox()");
#endif
    assert( IsPrimed( level ) );

    // See if the guess is correct:
    if( guess )
    {
        if( CoordsAreInBox( coords, level, *guess ) )
        {
            m_lastBoxFound[level] = *guess;
            return *guess;                                // Early return
        }
    }
    // Nope, guess wasn't correct.  Try m_lastBoxFound[level] as a second guess.
    if( m_lastBoxFound.find( level ) != m_lastBoxFound.end() )
    {
        int guess2 = m_lastBoxFound[level];
        //cerr << "Guessing level " << level << ", box " << guess2 << endl;
        if( CoordsAreInBox( coords, level, guess2 ) )
        {
            //cerr << "Guessed right." << endl;
            return guess2;                                // Early return
        }
        //cerr << "Guessed wrong." << endl;
    }
    // Wrong again.  OK, gotta do some real work then.

    //
    // Using the BoxPhases data, find the enclosing box.
    //
    set<int> candidate_boxNums[ m_nDims ]; // We'll take the intersection.
    map<int, vector<CutoffVector> >::const_iterator level_iter =
        m_byLevelByAxisCutoffs.find( level );
    for( int axis=0; axis<m_nDims; ++axis )
    {
        //
        // From here up to (but not including) the call to upper_bound is
        // about 60 milliseconds.
        //
        // Find the lowest cutoff whose position is >= coords[axis].
        CutoffVector::const_iterator axisIter =
            (level_iter->second)[axis].begin();
        CutoffVector::const_iterator axisIterEnd =
            (level_iter->second)[axis].end();

        if( coords[axis] < axisIter->first )
        {   // Totally out of bounds.
            return -1;                                      // Early return
        }

/*
        //
        // This linear search accounts for about 40 milliseconds.  It beats
        // the use of upper_bound for all but the biggest datasets.
        //
        while( (axisIter->first < coords[axis]) // first is the position
          &&   (axisIter != axisIterEnd ) )
        {
            ++axisIter;
        }
*/

        Cutoff temp;
        temp.first = coords[axis];
        // This call to upper_bound accounts for about 40 milliseconds, every
        // time we update streamlines at level 10 of the rings dataset, with
        // 100 seeds.
        axisIter = upper_bound( (level_iter->second)[axis].begin(),
                                (level_iter->second)[axis].end(),
                                temp,
                                CutoffComparator() );

        // Take the boxes that, at the upper bound cutoff we just obtained,
        // either end or continue, but exclude those (one-cell-wide or high)
        // boxes that also begin there.  If the upper bound is exactly at
        // coords[axis], then take also the boxes that begin there.
        if( axisIter != axisIterEnd )
        {
            set<int> cont_ends_union;
            set_union(
                axisIter->second.m_boxContinuations.begin(),
                axisIter->second.m_boxContinuations.end(),                
                axisIter->second.m_boxEnds.begin(),
                axisIter->second.m_boxEnds.end(),
                inserter( cont_ends_union, cont_ends_union.begin() ) );
            set_difference(
                cont_ends_union.begin(),
                cont_ends_union.end(),
                axisIter->second.m_boxStarts.begin(),
                axisIter->second.m_boxStarts.end(),
                inserter( candidate_boxNums[axis],
                          candidate_boxNums[axis].begin() ) );
            if( axisIter->first == coords[axis] )
            {
                candidate_boxNums[axis].insert(
                    axisIter->second.m_boxStarts.begin(),
                    axisIter->second.m_boxStarts.end() );
            }
        }
    }

    //
    // The rest of this function accounts for about 35 milliseconds.
    //
    // Assemble the result -- intersection of the axis-specific sets.
    set<int> result;
    set_intersection(
        candidate_boxNums[0].begin(), candidate_boxNums[0].end(),
        candidate_boxNums[1].begin(), candidate_boxNums[1].end(),
        inserter( result, result.begin() ) );

    if( m_nDims == 3 )
    {
        set<int> temp;
        set_intersection(
            result.begin(), result.end(),
            candidate_boxNums[2].begin(), candidate_boxNums[2].end(),
            inserter( temp, temp.begin() ));
        result.clear();
        result.insert( temp.begin(), temp.end() );
    }
    if( result.size() == 0 )
    {
        return -1;
    }
    if( result.size() > 1 )
    {
#ifndef DO_DEMO
        t.Info( "Found more than one enclosing box, returning first one." );
#endif
    }

    int retval = *(result.begin());
    m_lastBoxFound[level] = retval;

    return retval;
}


/** Returns true if the coordinates lie within the indicated box in the
 *  indicated level.
*/
bool
BoxFinder::CoordsAreInBox( int const * coords, int level, int boxNum ) const
{
    map<int, map<int, vector<int> > >::const_iterator iter1 =
        m_cornerMaps.find( level );
    assert( iter1 != m_cornerMaps.end() );
    map<int, vector<int> >::const_iterator iter2 =
        iter1->second.find( boxNum );
    assert( iter2 != iter1->second.end() );

    if( ( (iter2->second)[0] <= coords[0] ) 
    &&  ( coords[0] <= (iter2->second)[3] )
    &&  ( (iter2->second)[1] <= coords[1] ) 
    &&  ( coords[1] <= (iter2->second)[4] )
    &&  ( (iter2->second)[2] <= coords[2] )
    &&  ( coords[2] <= (iter2->second)[5] ) )
    {
        return true;
    } else
    {
        return false;
    }
}


/** Returns the box-number of the last box found on the indicated level.  Useful
 *  if you want to pass a guess argument to BoxFinder::FindBox().
 *  If we haven't returned any box at the indicated level yet, then we return
 *  the enum k_noBox.
*/
int
BoxFinder::GetLastBoxFound( int level ) const
{
    map<int,int>::const_iterator iter = m_lastBoxFound.find(level);
    if( iter == m_lastBoxFound.end() )
    {
        return k_noBox;
    } else
    {
        return iter->second;
    }
}


/** Returns the level on which the last invocation of BoxFinder::FindBox() found
 *  a box.  If BoxFinder::FindBox() hasn't found any boxes yet, returns k_noBox.
*/
int
BoxFinder::GetLastLevelFound() const
{
    // m_lastLevelFound should have been initialized to k_noBox in the
    // BoxFinder ctor.
    return m_lastLevelFound;
}

#ifndef DO_DEMO  // operator<<(std::set) is defined elsewhere in ChomboVis,
                 // in a file we'd rather not make the unittest depend on.
ostream & operator<<( ostream & ost, BoxPhases const & b )
{
    ost << "      m_boxStarts = " << b.m_boxStarts << endl;
    ost << "      m_boxContinuations = " << b.m_boxContinuations << endl;
    ost << "      m_boxEnds = " << b.m_boxEnds;
    return ost;
}

std::ostream& operator<<( ostream & ost,
                          map<int,vector<CutoffVector> > const & mcv )
{
    for( map<int,vector<CutoffVector> >::const_iterator miter=mcv.begin();
         miter != mcv.end();
         ++miter )
    {
        ost << "level " << miter->first << ":\n";
        int v_ndx(0);
        for( vector<CutoffVector>::const_iterator viter = miter->second.begin();
             viter != miter->second.end();
             ++viter, ++v_ndx )
        {
            ost << "  axis " << v_ndx << ":\n";
            for( CutoffVector::const_iterator citer = viter->begin();
                 citer != viter->end();
                 ++citer )
            {
                ost << "    (" << citer->first << "; \n"
                               << citer->second << ")\n";
            }
        }
    }
    return ost;
}
#endif


#ifdef DO_DEMO

/* Standalone demo for the BoxFinder class.  To build, just say
 * "g++ -DDO_DEMO BoxFinder.cpp".
 * Modify parameters (int const variables) to taste.
*/

#include <cstdlib>
#include <iostream>
#include <vector>

int main()
{
    int const nBoxesCubeRoot = 10; // # of boxes will be this to 3rd power.
    int const domainSize = 400;
    int const nPointsToLocate = 10;

    // Generate box data in 3D.  To keep the box-generating algorithm simple,
    // we'll make all the boxes the same size.  Of course, the BoxFinder
    // class can handle variable sized boxes -- any partition of any subset of
    // a rectangular domain into nonoverlapping rectangles.

    map<int, vector<int> > cornerData;  // For a single level.
    int boxSize = domainSize / nBoxesCubeRoot;
    vector<int> extents(6);
    
    int m[3];   // Dimension-specific index.
    for( m[0]=0; m[0]<nBoxesCubeRoot; ++m[0] )
    {
        for( m[1]=0; m[1]<nBoxesCubeRoot; ++m[1] )
        {
            for( m[2]=0; m[2]<nBoxesCubeRoot; ++m[2] )
            {
                for( int i=0; i<3; ++i )
                {
                    extents[i]   = boxSize * m[i];
                    extents[3+i] = boxSize * (m[i] + 1) - 1;
                }
                cornerData[  m[0]*nBoxesCubeRoot*nBoxesCubeRoot
                           + m[1]*nBoxesCubeRoot
                           + m[2] ] = extents;
            }
        }
    }
    cerr << "Done generating boxes." << endl;

    BoxFinder boxFinder(3);
    boxFinder.Prime( 0, cornerData );
    cerr << "Done priming box finder with box data.  Ready now to search for\n"
         << "the boxes enclosing " << nPointsToLocate <<  " random points..."
         << endl;

    // Locate boxes enclosing random points in the domain.
    for( int p=0; p<nPointsToLocate; ++p )
    {
        int point[3];
        for( int m=0;m<3;++m )
        {
            point[m] = rand()%(domainSize+2) - 1;
        }  // That'll generate some out-of-bounds coordinates too.

        int boxNum = boxFinder.FindBox( 0, point );
        cerr << "[" << point[0] << ", " << point[1] << ", " << point[2] << "] "
             << "is in box " << boxNum << ", whose extents are [";
        if( boxNum != -1 )
        {
            for( int i=0;i<5;++i )
            {
                 cerr << cornerData[boxNum][i] << ", ";
            }
            cerr << cornerData[boxNum][5] << "]." << endl;
        } else
        {
            cerr << "[(not a box)]" << endl;
        }
    }

    // Test against reappearance of a bug fixed on 8/18/2005.
    {
        map<int, vector<int> > cornerData;
        int corn0[6] = {1,2,0,1,2,0};
        cornerData[0] = vector<int>(corn0,corn0+6);
        int corn1[6] = {2,0,0,2,0,0};
        cornerData[1] = vector<int>(corn1,corn1+6);
        BoxFinder boxFinder(2);
        boxFinder.Prime( 0, cornerData );
        int coords[3] = {1,1,0};
        assert( boxFinder.FindBox(0,coords) == -1 );
        // If assertion fails, old bug is back.
    }

    return 0;
}
#endif // DO_DEMO
