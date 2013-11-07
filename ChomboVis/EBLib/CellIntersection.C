// ============================================================
//    CellIntersection
//
//       Small library for calculating the intersection
//       between a hexahedron and a plane. Can return
//       other information to help with volume calculations
//       and intersection connectivity info.
//
//    (c) Christopher S. Co
// ============================================================

#include "CellIntersection.h"

#include <cassert>
#include <list>
#include <vector>
#include <queue>
#include <math.h>
#include "Plane.h"
#include "Tetrahedron.h"
#include "Hexahedron.h"
#include "ConnectedHexahedron.h"
#include "CVPolygon.h"
#include "Point.h"
#include "KIJVector.h"

using std::cerr;
using std::endl;

// ============================================================
//    Internal data structures
// ============================================================

class DistanceVertex
{
   public:

      double   distance ;
      int      vertex ;

      DistanceVertex()
         {
            distance = 0.0 ;
            vertex = -1 ;
         }

      DistanceVertex& operator= ( const DistanceVertex& a )
         {
            if( &a == this ) return *this ;

            distance = a.distance ;
            vertex = a.vertex ;

            return *this ;
         }
   
      friend ostream& operator<< ( ostream& co, const DistanceVertex& a )
         {
            co << a.vertex << ": " << a.distance ;
            return co ;
         }

      friend int operator< ( const DistanceVertex& a, const DistanceVertex& b )
         {
            return a.distance < b.distance ;
         }
} ;

class ActiveEdge
{
   public:

      int   active ;
      DistanceVertex *  endPt ;
      int   edge ;
      std::list<Point>* leftPoly ;
      std::list<Point>* rightPoly ;

      ActiveEdge()
         {
            active = 0 ;
            endPt = NULL ;
            edge  = -1 ;
            leftPoly = NULL ;
            rightPoly = NULL ;
         }

      ActiveEdge( const ActiveEdge& a )
         {
            active = a.active ;
            endPt = a.endPt ;
            edge  = a.edge ;
            leftPoly = a.leftPoly ;
            rightPoly = a.rightPoly ;
         }

      virtual ~ActiveEdge() { }

      friend ostream& operator<< ( ostream& co, const ActiveEdge& a )
         {
            co << "{ " ;
            co << ( a.IsActive() ? "ACTIVE" : "inactive" ) ;
            co << ", " ;

            co << a.endPt << " " ;
            if( a.endPt )
               co << *(a.endPt) ;
            else
               co << "NULL" ;

            co << ", " ;

            co << a.leftPoly ;
            co << ", " ;
            co << a.rightPoly ;
            co << ", " ;

            co << ((char) (a.edge + 'a') ) ;

            co << " }" ;
            return co ;
         }

      friend int operator< ( const ActiveEdge& a, const ActiveEdge& b )
         {
            if( a.endPt && b.endPt )
               return *(a.endPt) < *(b.endPt) ;
            return 0 ;
         }

      int IsActive() const
         {
            if( active )   return 1 ;
            else           return 0 ;
         }
      int IsNotActive() const
         {
            if( active )   return 0 ;
            else           return 1 ;
         }
} ;

typedef std::list<ActiveEdge*> AEList ;

class CompareCubeEdgeIters
{
   public:
      int operator()
         ( AEList::iterator a, AEList::iterator b )
         {
            return (**a) < (**b) ;
         }
} ;

// ============================================================
//    Internal Routines for internal data structures
// ============================================================

int StillHaveExpiredEdges( AEList::iterator top )
   {
      assert( *top );
      assert( (*top)->endPt );
      if( (*top)->endPt->distance >= 0 )
      {
         return 1 ;
      }
      return 0 ;
   }

      //  Two routines for circular linked list traversals.
      //  It basically ignores STL's artificial end() node.

template<typename T>
typename std::list<T>::iterator Next ( 
    typename std::list<T>::iterator curr,
    std::list<T>& theList )
   {
      typename std::list<T>::iterator temp ;

      temp = curr ;
      if( temp == theList.end() )
         return theList.begin() ;
      temp++ ;
      if( temp == theList.end() )
         return theList.begin() ;

      return temp ;
   }

template<class T>
typename std::list<T>::iterator Previous (
    typename std::list<T>::iterator curr, std::list<T>& theList )
   {
      typename std::list<T>::iterator temp ;

      temp = curr ;
      if( temp == theList.begin() )
         return --theList.end() ;
      temp-- ;

      return temp ;
   }

// ============================================================
//    Implementation of library
// ============================================================

// ============================================================
//    Constructors, Destructor
// ============================================================

CellIntersection :: CellIntersection ()
   {
      _mode = 0 ; // Compute intersection only (default)
   }

CellIntersection :: ~CellIntersection ()
   {
   }

// ============================================================
//    Intersection Routines
// ============================================================

//
//  Assumptions:
//       The cell is a valid convex hexahedron.
//       The plane intersects the cell.
//
//  This routine can calculate the polygon that intersects the
//  hexahedron and all the polygons that form the remaining
//  polyhedron after slicing.
//
//  "mode" controls how much is computed
//
//  Bugs:
//    -  This algorithm can return the "intersection" when the 
//       plane IS on one of the faces of the cell AND the normal
//       points INTO the cell.
//       If the normal points AWAY from the cell and the plane IS
//       on one of the faces of the cell, then the algorithm breaks.
//
//  Returns the polygon intersecting the cell.
//

// TODO: Optimization inquiry
//
//  Assumptions:
//       The cell is a valid convex hexahedron.
//       The plane intersects the cell.
//
//  This routine can calculate the polygon that intersects the
//  hexahedron and all the polygons that form the remaining
//  polyhedron after slicing.
//
//  "mode" controls how much is computed
//
//  Returns the polygon intersecting the cell.
//

int CellIntersection :: IntersectCellWithPlane
   (
      const Plane&      plane,         // IN : Plane cutting the hexahedron
      const Hexahedron& cell,          // IN : The hexahedron
      CVPolygon&          intersection   // OUT:
   )
   {
      /* TDS debugging stuff for gcc3.4.2 segfault problem.
      cerr << "IntersectCellWithPlane(): Plane=" << plane << endl;
      cerr << "IntersectCellWithPlane(): Hexahedron=" << cell << endl;
      */

      unsigned int   i ;
      DistanceVertex maxDistVert ;
      ActiveEdge     cellEdges[12] ;
      HexahedronEdgeIterator     edgeIter ;

      DistanceVertex    dist[8] ;
      AEList            ActiveEdgeList ;
      std::priority_queue
         <
            AEList::iterator,
            std::vector<AEList::iterator>,
            CompareCubeEdgeIters
         >
         WaitingEdges ;

         //  Init the cell edges
      for( i=0; i<12; i++ )
      {
         cellEdges[i].edge = i ;
      }

         //  Calculate "distances" from each point to the plane
      KIJVector normal = -plane.direction() ;
      for( i=0; i<cell.size(); i++ )
      {
         dist[i].vertex = i ;
         dist[i].distance = dot( normal, cell[i]-plane.anchor() ) ;

            //  Keep track of the vertex with the max distance
         if( i == 0 )
         {
            maxDistVert = dist[i] ;
         }
         else
         if( dist[i].distance > maxDistVert.distance )
         {
            maxDistVert = dist[i] ;
         }
      }

         //  Initialize the WaitingEdges priority queue
      edgeIter = HexahedronEdgeIterator( maxDistVert.vertex ) ;
      do
      {
         ActiveEdge * newEdge = &(cellEdges[ edgeIter.Current() ]) ;
         newEdge->endPt = &(dist[ edgeIter.GetOtherVertex() ]) ;
         newEdge->active = 1 ;

         ActiveEdgeList.push_back( newEdge ) ;
         WaitingEdges.push( --(ActiveEdgeList.end()) ) ;

         edgeIter.Next() ;
      }
      while( ! edgeIter.End() ) ;

         //  PHASE I : Compute intersecting polygon

         //  Build polygon through split or merge process

/*
// TDS: This is my fix to the gcc3.4.2 segfault bug: stick this into the
// DeactivateAndRemove() macro function:
//      if( (*x) == (*WaitingEdges.top()) )\
//      {\
//        WaitingEdges.pop();\
//      }\
// The obvious alternative would be to not pop()
// the top and instead just not call "(*(x))->active = 0".  But I tried
// that and, when I tried to turn on a 3D EB, ChomboVis exited and printed
// "*** IntersectCellWithPlane: list problem".  So that's why I chose
// the pop() route.  We still get a segfault if we try the volume-fraction
// method.  So until the issue is resolved, I'm keeping my "fix" out in the
// interest of making the bug easier to detect.
*/
#define DeactivateAndRemove( x ) \
   { \
      (*(x))->active = 0 ; \
      ActiveEdgeList.erase( (x) ) ; \
   }
#define ActivateAndInsert( e ) \
   { \
      AEList::iterator temp ; \
      ActiveEdge * newEdge = &(cellEdges[ (e) ]) ; \
      \
      newEdge->endPt = &(dist[ \
            GetOtherVertexConnectedToEdge( (e), (*topOfPQ)->endPt->vertex ) \
         ]) ; \
      newEdge->active = 1 ; \
      \
      ActiveEdgeList.insert( topOfPQ, newEdge ) ; \
      temp = Previous( topOfPQ, ActiveEdgeList ) ; \
      WaitingEdges.push( temp ) ; \
   }

      AEList::iterator topOfPQ ;
      topOfPQ = WaitingEdges.top() ;
      //cerr << "1. topOfPQ=|" << (*topOfPQ) << "|\n";
      while( ! WaitingEdges.empty() && StillHaveExpiredEdges( topOfPQ ) )
      {
            //  Get rid of the top before we change the queue
         WaitingEdges.pop() ;

            //  Perform merge or split
         if( (*topOfPQ)->IsActive() )
         {
               //  The 3 edges being dealt with
               //  edge[0] is the "active" one being examined.
               //  edge[1] is ccw from edge[0]
               //  edge[2] is ccw from edge[1]
            int   edges[3] ;

               //  Determine if we are to split or merge
            edgeIter = HexahedronEdgeIterator
               ( (*topOfPQ)->endPt->vertex, (*topOfPQ)->edge ) ;
            i = 0 ;
            do
            {
               edges[i] = edgeIter.Current() ;
               i++ ;
               edgeIter.Next() ;
            }
            while( ! edgeIter.End() ) ;

            AEList::iterator temp ;
            if(   cellEdges[ edges[1] ].IsNotActive()
                  && cellEdges[ edges[2] ].IsNotActive()
               )
            {
                  //  Do Split
                  //   ie- insert the two new edges,
                  //       remove the current one
               ActivateAndInsert( edges[1] ) ;
               ActivateAndInsert( edges[2] ) ;
               DeactivateAndRemove( topOfPQ ) ;
            }
            else
            {
                  //  Do Merge
                  //   ie- remove the active and current (active)
                  //       edges, insert the inactive one
               int   Active, InActive ;

               if( cellEdges[ edges[1] ].IsActive() )
               {
                  Active = 1 ;
                  InActive = 2 ;
               }
               else
               {
                  Active = 2 ;
                  InActive = 1 ;
               }

               ActivateAndInsert( edges[InActive] ) ;

               temp = Next( topOfPQ, ActiveEdgeList ) ;
               if( (*temp)->edge == edges[Active] )
               {
                  DeactivateAndRemove( temp ) ;
               }
               else
               {
                     //  Because of the insert, we
                     //    need to move back twice.
                  temp = Previous( topOfPQ, ActiveEdgeList ) ;
                  temp = Previous( temp, ActiveEdgeList ) ;
                  if( (*temp)->edge == edges[Active] )
                  {
                     DeactivateAndRemove( temp ) ;
                  }
                  else
                  {
                     cerr  << "*** IntersectCellWithPlane: list problem"
                           << endl ;
                     exit( 200 ) ;
                  }
               }
                  //  Remove the current edge
               DeactivateAndRemove( topOfPQ ) ;
            }
         }

            //  Update everything
         topOfPQ = WaitingEdges.top() ;
         //cerr << "2. topOfPQ=|" << (*topOfPQ) << "|\n";
      }

         //  Build polygon from the edges
      std::list<Point> polyPts ;
      {
         AEList::iterator iter ;
         for(  iter=ActiveEdgeList.begin();
               iter!=ActiveEdgeList.end();
               iter++ )
         {
            int edge, vertex ;
            Point newPt ;
            edge = (*iter)->edge ;
            vertex = (*iter)->endPt->vertex ;
            IntersectLineWithPlane
               (
                  cell[ vertex ],
                  cell[ GetOtherVertexConnectedToEdge( edge, vertex ) ],
                  plane,
                  newPt
               ) ;
            polyPts.push_back( newPt ) ;
         }
      }

         // Pass the result back
      intersection = CVPolygon( polyPts ) ;

         //  PHASE Ia : Compute which faces of the
         //             cell are intersected
      if( _mode & COMPUTE_INTERSECTED_FACES )
      {
         _FacesIntersected.clear() ;
         AEList::iterator iter1, iter2 ;
         iter2 = ActiveEdgeList.begin() ;
         iter2 = Next( iter2, ActiveEdgeList ) ;
         for(  iter1=ActiveEdgeList.begin();
               iter1!=ActiveEdgeList.end();
               iter1++, iter2 = Next( iter2, ActiveEdgeList ) )
         {
            int edge1, edge2 ;
            edge1 = (*iter1)->edge ;
            edge2 = (*iter2)->edge ;
            _FacesIntersected.push_back
               ( GetFaceConnectedToEdges( edge1, edge2 ) ) ;
         }
      }

         //  PHASE Ib : Compute the centroid of the
         //             volume of intersection
      if( _mode & COMPUTE_INTERSECTING_VOLUME_CENTROID )
      {
         unsigned int i ;
         std::list<Point> voiPts ;
         for( i=0; i<intersection.size(); i++ )
         {
            voiPts.push_back( intersection[i] ) ;
         }
         for( i=0; i<cell.size(); i++ )
         {
            if( plane.IsIn( cell[i] ) )
            {
               voiPts.push_back( cell[i] ) ;
            }
         }
         _centroid = average( voiPts ) ;
      }


// TODO fix this part
      //if( _mode == EB_COMPUTE_INTERSECTING_POLYGON )
      if( !( _mode & COMPUTE_INTERSECTING_VOLUME ) )
         return 1 ;

         //  PHASE II : Compute remaining faces of
         //             intersecting polyhedron

      std::vector<CVPolygon> polyhedron ;
      std::list<std::list<Point>*> faces ;

      polyhedron.push_back( intersection ) ;

         //  Initialize the polygons touching each edge of
         //     the intersecting polygon.
// TODO: Optimization inquiry
         // Reset the list of tet faces
      _TetFaces.clear() ;
      {
         unsigned int i, j ;
         unsigned int size = intersection.size() ;
         AEList::iterator iter ;
         std::list<Point>* prevFace = NULL ;

         for(  i=0, j=1, iter=ActiveEdgeList.begin() ;
               i<size;
               i++, j=((j+1)%size), iter++ )
         {
            std::list<Point>* newFace ;
            newFace = new std::list<Point> ;
            faces.push_back( newFace ) ;

            newFace->push_back( intersection[i] ) ;
            newFace->push_back( intersection[j] ) ;

            (*iter)->leftPoly = newFace ;
            (*iter)->rightPoly = prevFace ;

            prevFace = newFace ;
// TODO: Optimization inquiry
               //  Keep track of which polygons do not touch
               //    the first point of the first poly
            if( i != 0 && i != (size-1) )
            {
               _TetFaces.push_back(i+1) ;
            }
         }

            //  Patch up the first ActiveEdge's pointer
         iter = ActiveEdgeList.begin() ;
         (*iter)->rightPoly = faces.back() ;
      }


      topOfPQ = WaitingEdges.top() ;
      while( ! WaitingEdges.empty() )
      {
            //  Get rid of the top before we change the queue
         //cerr << "About to pop|" << (*WaitingEdges.top()) << "|\n";
         WaitingEdges.pop() ;
         //cerr << "...popped" << endl;

            //  The 3 edges being dealt with
            //    edge[0] is the "active" one being examined.
            //    edge[1] is ccw from edge[0]
            //    edge[2] is ccw from edge[1]
         int   edges[3] ;
            //  In case of MERGE, these are the indices of the
            //    edges that are Active (aside from edges[0])
            //    and InActive
         int   Active = -1, InActive = -1 ;
            //  Flags whether to do split (or merge)
         int   DoSplit = 0 ;

            //  Determine if we are to split or merge or stop
         edgeIter = HexahedronEdgeIterator
            ( (*topOfPQ)->endPt->vertex, (*topOfPQ)->edge ) ;
         i = 0 ;
         do
         {
            edges[i] = edgeIter.Current() ;
            i++ ;
            edgeIter.Next() ;
         }
         while( ! edgeIter.End() ) ;

            //  A few aliases for code readability
         const Point & pt2add = cell[ (*topOfPQ)->endPt->vertex ] ;
         ActiveEdge & aEdge0 = cellEdges[ edges[0] ] ;
         ActiveEdge & aEdge1 = cellEdges[ edges[1] ] ;
         ActiveEdge & aEdge2 = cellEdges[ edges[2] ] ;

         AEList::iterator temp ;

         if( (*topOfPQ)->IsActive() )
         {
            if( aEdge1.IsActive() && aEdge2.IsActive() )
            {
#if DEBUG
cerr << ">>> all merge " ;
cerr << (char)(edges[0]+'a') << " " ;
cerr << (char)(edges[1]+'a') << " " ;
cerr << (char)(edges[2]+'a') << " -> " ;
cerr << (*topOfPQ)->endPt->vertex ;
cerr << endl ;
#endif
               for( i=0; i<3; i++ )
               {
                  cellEdges[ edges[i] ].leftPoly->push_back( pt2add ) ;
               }
               break ;
            }

            DoSplit = aEdge1.IsNotActive() && aEdge2.IsNotActive() ;


            if( DoSplit )
            {
                  //  Create a new polygon
               std::list<Point>* newFace ;
               newFace = new std::list<Point> ;
               faces.push_back( newFace ) ;

                  //  Update the polygons
               aEdge1.leftPoly   = newFace ;
               aEdge1.rightPoly  = aEdge0.rightPoly ;
               aEdge2.leftPoly   = aEdge0.leftPoly ;
               aEdge2.rightPoly  = newFace ;

               aEdge0.leftPoly->push_front( pt2add ) ;
               aEdge0.rightPoly->push_back( pt2add ) ;
               newFace->push_front( pt2add ) ;
// TODO: Optimization inquiry
               _TetFaces.push_back( faces.size() ) ;

#if DEBUG
cerr << ">>> split " ;
cerr << (char)(edges[0]+'a') << " -> " ;
cerr << (char)(edges[1]+'a') << " " << (char)(edges[2]+'a') ;
cerr << endl ;
#endif
            }
            else
            {
               int   A0, A1, I ;

                  //  Update the polygons
               if( aEdge1.IsActive() )
               {
                  Active = 1 ;
                  InActive = 2 ;
                  A0 = 0 ;
                  A1 = 1 ;
                  I  = 2 ;
               }
               else
               {
                  Active = 2 ;
                  InActive = 1 ;
                  A0 = 2 ;
                  A1 = 0 ;
                  I  = 1 ;
               }

               cellEdges[edges[I]].leftPoly  = cellEdges[edges[A0]].leftPoly ;
               cellEdges[edges[I]].rightPoly = cellEdges[edges[A1]].rightPoly ;

               cellEdges[ edges[A0] ].rightPoly->push_back( pt2add ) ;
               cellEdges[ edges[I] ].leftPoly->push_front( pt2add ) ;
               cellEdges[ edges[I] ].rightPoly->push_back( pt2add ) ;

#if DEBUG
cerr << ">>> merge " ;
cerr << (char)(edges[0]+'a') << " " << (char)(edges[Active]+'a') ;
cerr << " -> " << (char)(edges[InActive]+'a') ;
cerr << endl ;
#endif
            }
         }
   
            //  Perform merge or split on edges
         if( (*topOfPQ)->IsActive() )
         {
            if( DoSplit )
            {
                  //  Do Split
                  //   ie- insert the two new edges,
                  //       the current one is removed later
               ActivateAndInsert( edges[1] ) ;
               ActivateAndInsert( edges[2] ) ;

                  //  Remove the current edge
               DeactivateAndRemove( topOfPQ ) ;
            }
            else
            {
                  //  Do Merge
                  //   ie- remove the active edge, insert the inactive one
                  //       the current (active) one is removed later

               ActivateAndInsert( edges[InActive] ) ;

               temp = Next( topOfPQ, ActiveEdgeList ) ;
               if( (*temp)->edge == edges[Active] )
               {
                  DeactivateAndRemove( temp ) ;
               }
               else
               {
                     //  Because of the insert, we
                     //    need to move back twice.
                  temp = Previous( topOfPQ, ActiveEdgeList ) ;
                  temp = Previous( temp, ActiveEdgeList ) ;
                  if( (*temp)->edge == edges[Active] )
                  {
                     DeactivateAndRemove( temp ) ;
                  }
                  else
                  {
                     cerr  << "*** IntersectCellWithPlane: list problem"
                           << endl ;
                     exit( 200 ) ;
                  }
               }
                  //  Remove the current edge
               DeactivateAndRemove( topOfPQ ) ;
            }
         }

            //  Update topOfPQ list iterator
         topOfPQ = WaitingEdges.top() ;
      } // end of split or merge loop

         //  Build other faces and clean up memory
      {
         std::list<std::list<Point>*>::iterator iter ;
         for( iter=faces.begin(); iter!=faces.end(); iter++ )
         {
            polyhedron.push_back( CVPolygon( *(*iter) ) ) ;
            delete *iter ;
         }
      }

      _VolumeFaces = polyhedron ;

      return 1 ;
   }

// ===============================================================--
//    2D Intersection Calculations
// ===============================================================--

//  Assumes that an intersection with the cell exists
//  Assumes that 2D calculations are done in the XY plane
//  Does not handle special case when plane anchor is a cell corner
//    and normal is (+/-) X/Y-axes.
//  Returns the polygon that remains of the cell for a volume (area)
//    calculation.
// TODO insert face connectivity info in here
int CellIntersection :: IntersectCellWithPlane
   (
      const Plane &     plane,   //  IN : cutting "plane" (line segment)
      const CVPolygon &   cell,    //  IN : the cell (quad) being cut
      Point &           left,    //  OUT: point "left" of the normal
      Point &           right    //  OUT: point "right" of the normal
   )
   {
         //  Rewrite the clipping algorithm here.
      unsigned int   i, currPt, prevPt ;
      double   d ;         // Value of dot product ;
      double   pd = 0.0 ;  // Value of previous dot product ;
      double   t ;         // Affine "fraction" for intersection calc
      Point    I ;         // Intersection point
      std::list<Point> in ;     // Collection of intersection points.
      Point    II[2] ;     // Two intersection points
      int      icount ;

      icount = 0 ;

         //  Create alias for plane information
      const KIJVector & n = plane.direction() ;
      const Point & P = plane.anchor() ;

      if( _mode & COMPUTE_INTERSECTED_FACES )
      {
         _FacesIntersected.clear() ;
      }

         //  Perform standard clipping algorithm
      for( i=0; i<=cell.size(); i++ )
      {
         currPt = i % cell.size() ;
         prevPt = (i-1) ;

         d = dot ( n, cell[currPt] - P ) ;

         if( d*pd < 0 )
         {
            t = pd / ( pd - d ) ;
            I = cell[prevPt] + t * ( cell[currPt] - cell[prevPt] ) ;
            in.push_back( I ) ;

            if( _mode & COMPUTE_INTERSECTED_FACES )
            {
                  // Since polygon edges are numbered
                  // by their previous vertex, this is correct.
               _FacesIntersected.push_back(prevPt) ;
            }

            if( icount > 2 )
            {
               cerr  << "IntersectCellWithPlane: 2D, too many intersections"
                     << endl ;
               return 0 ;
               //exit( 1 ) ;
            }
            II[icount] = I ;
            icount++ ;
         }
         if( d >= 0.0 && i != cell.size() )
         {
            if( d == 0.0 )
            {
               II[icount] = cell[i] ;
               icount++ ;
            }
            in.push_back( cell[currPt] ) ;
         }
         pd = d ;
      }

         //  Determine orientation
      int   refPt = 0,
            otherPt = 1 ;
      if( II[0] == P )
      {
         refPt = 1 ;
         otherPt = 0 ;
      }
      if( cross( n, II[refPt]-P ).z() > 0 )
      {
         left = II[refPt] ;
         right = II[otherPt] ;
      }
      else
      {
         left = II[otherPt] ;
         right = II[refPt] ;
      }

      if( _mode & COMPUTE_INTERSECTING_VOLUME )
      {
         if( in.size() >= 3 )
            _Volume2D = CVPolygon( in ) ;
      }
      if( _mode & COMPUTE_INTERSECTING_VOLUME_CENTROID )
      {
         _centroid = _Volume2D.Centroid() ;
      }

      return 1 ;
   } // end IntersectCellWithPlane() (2D)

int CellIntersection :: IntersectLineWithPlane
   (
      const Point& a,      // IN :  1st point defining line
      const Point& b,      // IN :  2nd point defining line
      const Plane& plane,  // IN :  plane to be projected to
      Point & result       // OUT:  point in plane and on <a,b>
   )
   {
      double t ;
      const KIJVector & n = plane.direction() ;
      const Point & p = plane.anchor() ;

      t = dot( n, a-p ) / dot( n, a-b ) ;
      result = affine( a, b, t ) ;

      return 1 ;
   }

// ============================================================
//    Helper Routines
//       (primarily for volume calculations)
// ============================================================

int CellIntersection :: SplitVolumeIntoTets
   (
      TetList&   tets        // OUT: List to place the tets in
   )
   {
         //  Assuming that the volume is a convex polyhedron
         //    consisting of convex polygons, we will simply
         //    split each face into triangles, and connect
         //    the triangles to the centroid of the polyhedron
         //    to make tets.

      unsigned int   i, j ;
      std::vector<int>::iterator iter ;

         //  Calculate the centroid of the polyhedron

         //  _VolumeFaces's tet pivot point
         //    Assumed to be first point of first poly
      const Point & phtp = (_VolumeFaces[0])[0] ;

         //  Split each appropriate polygon into triangles
      for( iter=_TetFaces.begin();
         iter!=_TetFaces.end();
         iter++ )
      {
            //  We assume the polygon's points are already
            //    ordered ( CW or CCW )
         unsigned int nVerts ;   //  Num verts in poly

         const CVPolygon & poly = _VolumeFaces[*iter] ;
         nVerts   = poly.size() ;

            //  Connect each triangle to the polyhedron tet pivot
            //    point to make a tet
         for( i=1, j=2; j<nVerts; i++, j++ )
         {
            Tetrahedron newTet( poly[0], poly[i], poly[j], phtp ) ;
            tets.push_back( newTet ) ;
         }
      }
      return 1 ;
   }

double CellIntersection :: VolumeOfIntersection2D ()
   {
      return _Volume2D.Area() ;
   }

double CellIntersection :: VolumeOfIntersection3D ()
   {
      double volume = 0.0 ;
      TetList tets ;

         //  Split the polyhedron into tetrahedra
      SplitVolumeIntoTets( tets ) ;

         //  Sum the volumes of the the tets
      TetList::iterator tet ;
      for( tet=tets.begin() ; tet!=tets.end(); tet++ )
      {
         volume += tet->Volume() ;
      }

      return volume ;
   }

// ============================================================
//    Set / Access Routines
// ============================================================

void CellIntersection :: mode ( int m )
   {
      _mode = m ;
   }

CellIntersection::IntArray& CellIntersection :: facesIntersected ()
   {
      return _FacesIntersected ;
   }

Point& CellIntersection :: centroid ()
   {
      return _centroid ;
   }
