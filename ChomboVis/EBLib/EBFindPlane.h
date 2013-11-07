#ifndef _EBFindPlane_H_
#define _EBFindPlane_H_

// ============================================================
//    Class EBFindPlane
//
//       Encapsulates a root finding routine for obtaining
//       the plane that intersects a volume within a given
//       tolerance. The result is a fraction indicating
//       the anchor point along a line, where one point
//       is anchored at 0.0 and the other is anchored at 1.0.
//
//    (c) Christopher S. Co
// ============================================================

#include "EBVolumeCalculator.h"
#include "KIJVector.h"
#include "CVPolygon.h"
#include "Hexahedron.h"
#include "Point.h"

class EBFindPlane
{
   private :

      EBVolumeCalculator *    _volComputer ;
      Hexahedron *            _cell3D ;
      CVPolygon *               _cell2D ;
      double                  _tol ;
      int                     _maxiter ;

      double min ( double, double ) ;
      double _find () ;

   public :

         // Constructors, Destructor
      EBFindPlane () ;
      ~EBFindPlane () ;

         // Operations
      Point find ( const KIJVector& ) ;

         // Set Routines
      void volume ( EBVolumeCalculator * ) ;
      void cell2D ( CVPolygon * ) ;
      void cell3D ( Hexahedron * ) ;
      void tolerance ( double ) ;
      void target ( double ) ;
      void maxIter ( int ) ;

         // Access Routines
      double tolerance () const ;
      double target () const ;
} ;

#endif
