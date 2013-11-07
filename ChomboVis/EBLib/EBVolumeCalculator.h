#ifndef _EBVolumeCalculator_H_
#define _EBVolumeCalculator_H_

// ============================================================
//    Class EBVolumeCalculator
//
//       Computes the difference between a target
//       volume and the volume of a half-plane intersection.
//       The plane is described by a passed in parameter.
//
//       This is to be used with a root finding algorithm
//       (EBFindPlane).
//
//       This is the base class for a set of volume
//       calculation classes. Different instances
//       will differ from another in the algorithms
//       they use to compute volume and the type of
//       cell (2D or 3D) for which they compute volume for.
//
//    (c) Christopher S. Co
// ============================================================

#include "CellIntersection.h"
#include "Plane.h"
#include "Point.h"

class EBVolumeCalculator
{
   protected :

         // Slicer object for computing
         //    geometry of intersection
      CellIntersection* _slicer ;

         // Indices pointing to corners
         //    of the cell marking brackets
         //    for volume calculations
      int      _lb, _ub ;

         // The normal which defines the 
         //    set of planes we are searching from
      Plane    _currentCut ;

         // The target volume
      double   _target ;
      double   _cellVolume ;

   public :

         // Constructors, Destructor
      EBVolumeCalculator () ;
      virtual ~EBVolumeCalculator () ;

         // Operations
      virtual double volume ( double ) ;

         // Helper routine to find bracketing anchor
         //    points for cutting plane
      virtual int ObtainBrackets () ;
      virtual Point GetPlane ( const double ) ;

         // Set Routines
      virtual void normal ( const KIJVector& ) ;
      virtual void target ( double ) ;
      virtual void cellVolume ( double ) ;
      virtual void slicer ( CellIntersection * ) ;

         // Access Routines
      //virtual KIJVector& normal () ;
      virtual double target () ;
      virtual double cellVolume () ;
      virtual CellIntersection * slicer () ;
      virtual Plane& lastPlane () ;
} ;

#endif
