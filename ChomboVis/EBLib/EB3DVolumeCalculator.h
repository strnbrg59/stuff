#ifndef _EB3DVolumeCalculator_H_
#define _EB3DVolumeCalculator_H_

// ============================================================
//    Class EB3DVolumeCalculator
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
#include "Hexahedron.h"

class EB3DVolumeCalculator
   : public EBVolumeCalculator
{
   protected :

      Hexahedron*    _cell3D ;

   public :

         // Constructors, Destructor
      EB3DVolumeCalculator () ;

         // Operations
      virtual double volume ( double ) ;
      virtual int ObtainBrackets () ;
      virtual Point GetPlane ( const double ) ;

         // Set / Access Routines
      void cell ( Hexahedron * ) ;
} ;

#endif
