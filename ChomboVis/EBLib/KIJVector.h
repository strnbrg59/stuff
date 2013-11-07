#ifndef _KIJVector_h_
#define _KIJVector_h_

//
//  ============================================================
//
//                        KIJVector.h
//
//  ============================================================
//
//
// 	Copyright (C) 1992,1993,1994,1995,1996 
//
//			Professor Kenneth I. Joy
//			Computer Science Department
//			University of California
//			Davis, CA  95616
//
// 	Permission is granted to use at your own risk and 
//	distribute this software in source and binary forms 
//	provided the above copyright notice and this paragraph 
//	are preserved on all copies.  This software is provided 
//	"as is" with no express or implied warranty.
//
//
//  ============================================================
//

#include <iostream>
using std::ostream;

class Point ;

class	KIJVector {

   private :
	
   public :

			//  Public Data

      double		_vx, _vy, _vz ;

			//  Constructors

      KIJVector () ;

      KIJVector ( const double, const double = 0.0, const double = 0.0 ) ;

			//  Copy Constructor

      KIJVector ( const KIJVector& ) ;

			//  Destructor

      virtual ~KIJVector () ;

			//  Assignment

      KIJVector& operator= ( const KIJVector& ) ;

			//  Output

      friend ostream& operator<< ( ostream&, const KIJVector& ) ;

			//  Comparison

      friend int operator== ( const KIJVector&, const KIJVector& ) ;
      friend int operator!= ( const KIJVector&, const KIJVector& ) ;

			//  Sum, Difference, Scalar Product

      friend KIJVector operator+ ( const KIJVector&, const KIJVector& ) ;
      friend KIJVector operator- ( const KIJVector&, const KIJVector& ) ;
      friend KIJVector operator- ( const KIJVector& ) ;
      friend KIJVector operator* ( const double&, const KIJVector& ) ;
      friend KIJVector operator* ( const KIJVector&, const double& ) ;
      friend KIJVector operator/ ( const KIJVector&, const double& ) ;

			//  Immediate Sum, Difference, Scalar Product

      KIJVector& operator+= ( const KIJVector& ) ;
      KIJVector& operator-= ( const KIJVector& ) ;
      KIJVector& operator*= ( const double& ) ;
      KIJVector& operator/= ( const double& ) ;

			//  Member Functions
			   //  Dot Product

      friend double dot ( const KIJVector&, const KIJVector& ) ;

			   //  Cross Product

      friend KIJVector cross ( const KIJVector&, const KIJVector& ) ;

			   //  Normalization, Make it a unit KIJVector

      void normalize () ;

			   //  KIJVector Length

      double length () const ;

			//  Access Functions to get 
			//    x-coordinate, y-coordinate or
			//    z-coordinate of the vector

      double x () const { return _vx ; }
      double y () const { return _vy ; }
      double z () const { return _vz ; }

      void x ( double v ) { _vx = v ; }
      void y ( double v ) { _vy = v ; }
      void z ( double v ) { _vz = v ; }

			//  Draw
#ifdef VIS
		friend void glRotated( const double, const KIJVector& ) ;
		friend void glTranslated( const KIJVector& ) ;
		friend void glNormal3d( const KIJVector& ) ;

		void DrawWire (
			const Point&, // = PointZero,
			const double = 1.0,
			const double = 1.0,
			const double = 1.0,
			const double = 1.0
		) const ;

		void DrawSolid (
			const Point&, // = PointZero,
			const double = 1.0,
			const double = 1.0,
			const double = 1.0,
			const double = 1.0
		) const ;
		//void Draw () const ;
#endif

   } ;

			//  The global constant KIJVectors

const KIJVector VZero ( 0.0, 0.0, 0.0 ) ;
const KIJVector VX    ( 1.0, 0.0, 0.0 ) ;
const KIJVector VY    ( 0.0, 1.0, 0.0 ) ;
const KIJVector VZ    ( 0.0, 0.0, 1.0 ) ;

#endif



