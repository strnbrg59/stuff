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
#ifndef INCLUDED_BOXFINDER_H
#define INCLUDED_BOXFINDER_H

/** Facility for finding which box encloses a given point in 2D or 3D.  The
 *  point has integer coordinates that correspond to the number of cells from
 *  low corner of the AMR domain.
 *
 *  Example usage from a client class:
 *  int Client::FindEnclosingBox( int level, int const coords[] )
 *  {
 *      if( ! this->m_boxFinder.IsPrimed( level ) )
 *      {
 *          map<int, std::vector<int> > cornerData =
                this->AssembleCornerData( level);
 *          this->m_boxFinder.Prime( level, cornerData );
 *      }
 *      return this->m_boxFinder.FindBox( level, coords );
 *  }
 *
 *
 * See BoxFinder.cpp for a more fully-worked-out driver function (search for
 * "main()").
 *
 *
 * Algorithm:
 * For each axis, we construct a table (a std::map).  The keys are all the
 * positions, along the axis, where any box begins or ends -- notches on the
 * axis, as it were.  The values are collections of the ID numbers of all the
 * boxes you'd see if you looked perpendicular to that axis, at the
 * associated position.  BoxFinder::Prime() is where we construct this table;
 * this needs to be done only the first time you need to find a box, for each
 * level.
 * 
 * So, given a point (i,j,k), we identify the box that contains it by finding
 * where i, then j, then k fall among the "notches".  That gives us a
 * collection of boxes that lie opposite i on the x axis, another collection
 * opposite j on the y axis, and (for 3D data) a third collection opposite k
 * on the z axis.  The intersection of these 2 (or 3) collections gives us
 * the one box.  If the intersection contains more than one box (as it would,
 * if the point is at a box boundary) we select the first box, arbitrarily.
 * 
 * For 3D data and a domain divided into n boxes, exhaustive search takes a
 * number of steps that's proportional to n.  This method is proportional to
 * log(pow(n,1/3)) in the best case, and log(n) in the worst case.  The log
 * comes courtesy of binary search.  The best case is where the boxes are
 * arranged like a checkerboard.  The worst case is where no two boxes share
 * a common edge when viewed along any axis.
*/

#include <vector>
#include <map>
#include <set>
#include <fstream>

using std::vector;
using std::map;
using std::set;
using std::pair;
using std::ostream;

#ifndef THREE
#define THREE 3
#endif

/** Used by class BoxFinder.  Not for external use.
 *  A BoxPhases is associated with a particular position (the
 *  key of the map of which a BoxPhases is always the value), in 
 *  cells, along a particular axis.  The data members tell us which boxes
 *  begin (i.e. have their lo_corner) at that position, which boxes end there,
 *  and which continue there (i.e. their lo_corner is less than the position and
 *  their hi_corner is more than the position).
 *  For each level and each axis, we keep a vector of BoxPhases's -- one
 *  for every position along that axis where any box has its lo_corner or its
 *  hi_corner.
*/
struct BoxPhases
{
    set<int> m_boxStarts;         // Boxes whose lo_corner is here.
    set<int> m_boxContinuations;  // Boxes continuing here.
    set<int> m_boxEnds;           // Boxes whose hi_corner is here.
};
ostream & operator<<( ostream &, BoxPhases const & );

typedef pair<int, BoxPhases> Cutoff;  // Key is position along an axis
typedef vector<Cutoff> CutoffVector;  // Data for one axis

/** Consider this class to be the only public part of this module.
 * Finds the box that encloses a given position in cell-space.
*/
class BoxFinder
{
  public:
    BoxFinder( int dimensionality );
    
    bool IsPrimed( int level );
    void Prime( int level, map<int, vector<int> > boxCorners );

    int FindBox( int level, int const coords[THREE], int const * guess = 0 );

    int GetLastBoxFound( int level ) const;
    int GetLastLevelFound() const;

    enum { k_noBox=-1 };

  private:

    map<int, vector<CutoffVector> > m_byLevelByAxisCutoffs;
    map<int, map<int, vector<int> > > m_cornerMaps; // by level by box.
    int m_nDims; // 2 or 3.
    map<int,int> m_lastBoxFound; // Used as a guess.  Key=level, val=boxnum.
    int m_lastLevelFound; // Level in which last box was found.
    bool CoordsAreInBox( int const * coords, int level, int boxNum ) const;

    // Deliberately left unimplemented:
    BoxFinder();
    BoxFinder( BoxFinder const & );
    BoxFinder & operator=( BoxFinder const & );
};

std::ostream& operator<<( ostream&, map<int,vector<CutoffVector> > const& );

#endif // BOXFINDER_H
