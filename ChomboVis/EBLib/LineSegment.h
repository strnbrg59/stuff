#ifndef _LineSegment_H_
#define _LineSegment_H_

// ============================================================
//    Class LineSegment
//
//    (c) Christopher S. Co
// ============================================================

#include "Point.h"
#include "KIJVector.h"
#include "Plane.h"

class Plane ;

class LineSegment
{
   private :

      Point _left, _right ;

   protected :

         // Internal Copy routine
      void _Copy ( const LineSegment& ) ;

   public :

         // Constructors, Destructor
      LineSegment () ;
      LineSegment ( const LineSegment& ) ;
      ~LineSegment () ;

         // Operations
      LineSegment& operator= ( const LineSegment& ) ;
      Point& operator[] ( int ) ;
      int size () const ;

			//  Arithmetic Operations
		friend LineSegment operator+ ( const LineSegment&, const KIJVector& ) ;
		LineSegment& operator+= ( const KIJVector& ) ;

         // Calculations
      int clipIn ( const Plane&, LineSegment& ) const ;
      int clipInEx ( const Plane&, LineSegment& ) const ;
      int clipOut ( const Plane&, LineSegment& ) const ;
      int clipOutEx ( const Plane&, LineSegment& ) const ;
      void reverse () ;
      Point centroid () ;

      #ifdef VIS
         // Draw
      void Draw () ;
      #endif

         // Access Routines
      Point& left () ;
      Point& right () ;

         // Set Routines
      void left ( const Point& ) ;
      void right ( const Point& ) ;
} ;

#endif
