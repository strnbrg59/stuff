#ifndef _EB2DVolumeCalculator_H_
#define _EB2DVolumeCalculator_H_

// ============================================================
//    Class EB2DVolumeCalculator
//
//       Computes the difference between a target
//       volume and the volume of a half-plane intersection.
//       The plane is described by a passed in parameter.
//
//       This is to be used with a root finding algorithm
//       (EBFindPlane).
//
//       Used for 3D hexahedral cells.
//
//    (c) Christopher S. Co
// ============================================================

#include "EBVolumeCalculator.h"
#include "CVPolygon.h"

class EB2DVolumeCalculator
   : public EBVolumeCalculator
{
   protected :

      CVPolygon*          _cell2D ;

   public :

         // Constructors, Destructor
      EB2DVolumeCalculator () ;

         // Operations
      virtual double volume ( double ) ;
      virtual int ObtainBrackets () ;
      virtual Point GetPlane ( const double ) ;

         // Set / Access Routines
      void cell ( CVPolygon * ) ;
} ;

#endif
