#ifndef _EBCapper_H_
#define _EBCapper_H_

// ============================================================
//    Class EBCapper
//
//       This class encapsulates code to stitch together
//       disjoint surfaces created for regular grid data.
//
//       Cells' boundaries are clipped against the EB
//       produced by EBSingleCell on consecutive cells
//       in a grid to produce the caps.
//
//       NOTE:
//
//       It is assumed that the anchor point of the
//       planes passed in for capping lies within
//       its respective cell.
//
//    (c) Christopher S. Co
// ============================================================

#include <vector>
#include "Plane.h"
#include "Point.h"
#include "LineSegment.h"
#include "CVPolygon.h"
#include "Hexahedron.h"

using namespace std ;

class EBCapper
{
   public :

      typedef  std::vector<Plane>        PlaneArray ;
      typedef  std::vector<CVPolygon>      PolyArray ;
      typedef  std::vector<LineSegment>  LineArray ;

   private :

      void _Copy ( const EBCapper& ) ;

         // Internal geometry helper
      int _exor( PlaneArray&, PolyArray&, PolyArray& ) ;
      int _exor( PlaneArray&, LineArray&, LineArray& ) ;

   public :

         // Constructors, Destructor
      EBCapper () ;
      EBCapper ( const EBCapper& ) ;
      ~EBCapper () ;

         // Operations
      int cap ( CVPolygon&, PlaneArray&, PolyArray& ) ;
      int cap ( LineSegment&, PlaneArray&, LineArray& ) ;
      void adjCell ( int, int&, int&, int& ) ;
      void adjCell ( int, int&, int& ) ;
} ; // end class EBCapper

#endif
