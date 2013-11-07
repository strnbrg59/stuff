#ifndef _Point_h_
#define _Point_h_

//
//  ============================================================
//
//                        Point.h
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
//		distribute this software in source and binary forms 
//		provided the above copyright notice and this paragraph 
//		are preserved on all copies.  This software is provided 
//		"as is" with no express or implied warranty.
//
//		Slightly modified by CSC, 7/2001
//
//  ============================================================
//

#include <iostream>
#include <list>
#include "KIJVector.h"

class	Point {

   private :
	
   public :

			//  Public data

      double		_x, _y, _z ;

			//  Constructors

      Point () ;

      Point ( const double, const double = 0.0, const double = 0.0 ) ;

			//  Copy Constructor

      Point ( const Point& ) ;

			//  Destructor

      virtual ~Point () ;

			//  Assignment

      Point& operator= ( const Point& ) ;

			//  Output

      friend ostream& operator<< ( ostream&, const Point& ) ;

			//  Comparison

      friend int operator== ( const Point&, const Point& ) ;
      friend int operator!= ( const Point&, const Point& ) ;

			//  Arithmetic Operations

      friend Point operator+ ( const Point&, const KIJVector& ) ;
      friend Point operator+ ( const KIJVector&, const Point& ) ;
      friend KIJVector operator- ( const Point&, const Point& ) ;
      friend Point operator- ( const Point&, const KIJVector& ) ;
      friend Point operator- ( const Point& ) ;

      Point& operator+= ( const KIJVector& ) ;
      Point& operator-= ( const KIJVector& ) ;

			//  Affine Combinations

      friend Point affine
			(
				const Point&, 
				const Point&, 
				const double
			) ;

      friend Point affine
			(
				const Point&, 
				const Point&, 
				const Point&, 
				const double,
				const double,
				const double
			) ;

      friend Point average ( const Point *, const int ) ;
      friend Point average ( const std::list<Point> & ) ;
      friend Point midpoint ( const Point&, const Point& ) ;

			//  Access Functions

      double x () const { return _x ; } ;
      double y () const { return _y ; } ;
      double z () const { return _z ; } ;

#ifdef VIS
			//  Draw

		friend void glVertex3d( const Point& ) ;

		void Draw (
			const double = 1.0,
			const double = 1.0,
			const double = 1.0,
			const double = 1.0,
			const double = 0.1
		) const ;
#endif

   } ;

const Point PointZero ( 0.0, 0.0, 0.0 ) ;

#endif
