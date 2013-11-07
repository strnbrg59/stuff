
//
// ============================================================
//
//                      POINT.C
//
// ============================================================
//
//
//      Copyright (C) 1992,1993,1994,1995,1996 
//
//                      Professor Kenneth I. Joy
//                      Computer Science Department
//                      University of California
//                      Davis, CA  95616
//
//      Permission is granted to use at your own risk and 
//      distribute this software in source and binary forms 
//      provided the above copyright notice and this paragraph 
//      are preserved on all copies.  This software is provided 
//      "as is" with no express or implied warranty.
//
//
//  ============================================================
//

#include <cstdlib>
#include <cmath>
#include <strings.h>
#include <stream.h>

#ifdef VIS
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#include "Point.h"

#define		TRUE		1 
#define		FALSE		0


/*
   ==========================================================

	       CONSTRUCTORS, DESTRUCTOR

   ==========================================================
*/

Point :: Point ()

   {
      _x = 0.0 ;
      _y = 0.0 ;
      _z = 0.0 ;
   }

Point :: Point ( const double x, const double y, const double z )

   {
      _x = x ;
      _y = y ;
      _z = z ;
   }

Point :: Point ( const Point& p ) 

   {
      _x = p._x ;
      _y = p._y ;
      _z = p._z ;
   }

Point :: ~Point () { }
      

/*
   ==========================================================

	       OPERATOR=

   ==========================================================
*/

Point& Point :: operator= ( const Point& p ) 

   {
      if ( this == &p ) return ( *this ) ;

      _x = p._x ;
      _y = p._y ;
      _z = p._z ;

      return ( *this ) ;
   }

/*
   ==========================================================

	       OUTPUT

   ==========================================================
*/

ostream& operator<< ( ostream& co, const Point& p )

   {
      co << "( " << p._x << ", " << p._y << ", " << p._z << " )" ;

      return co ;
   }

/*
   ==========================================================

	       COMPARISON OPERATIONS

   ==========================================================
*/

int operator== ( const Point& p1, const Point& p2 ) 

   {
      if ( ( p1._x == p2._x ) &&
           ( p1._y == p2._y ) &&
           ( p1._z == p2._z ) )
	    return ( TRUE ) ;
	 else
	    return ( FALSE ) ;
   }

int operator!= ( const Point& p1, const Point& p2 ) 

   {
      if ( ( p1._x != p2._x ) ||
           ( p1._y != p2._y ) ||
           ( p1._z != p2._z ) )
	    return ( TRUE ) ;
	 else
	    return ( FALSE ) ;
   }

/*
   ==========================================================

	       ARITHMETIC OPERATIONS

   ==========================================================
*/

Point operator+ ( const Point& p1, const KIJVector& p2 ) 

   {
      Point pp ;

      pp._x = p1._x + p2.x() ;
      pp._y = p1._y + p2.y() ;
      pp._z = p1._z + p2.z() ;

      return ( pp ) ;
   }

Point operator+ ( const KIJVector& p2, const Point& p1 ) 

   {
      Point pp ;

      pp._x = p1._x + p2.x() ;
      pp._y = p1._y + p2.y() ;
      pp._z = p1._z + p2.z() ;

      return ( pp ) ;
   }

KIJVector operator- ( const Point& p1, const Point& p2 ) 

   {
      return ( KIJVector ( p1._x - p2._x, p1._y - p2._y, p1._z - p2._z ) ) ;
   }

Point operator- ( const Point& p, const KIJVector& v ) 

   {
      return ( Point ( p._x - v.x(), p._y - v.y(), p._z - v.z() ) ) ;
   }

Point operator- ( const Point& p ) 

   {
      Point pp ;

      pp._x = -p._x ;
      pp._y = -p._y ;
      pp._z = -p._z ;

      return ( pp ) ;
   }

Point& Point :: operator+= ( const KIJVector& v )

   {
      _x += v.x() ;
      _y += v.y() ;
      _z += v.z() ;

      return ( *this ) ;
   }

Point& Point :: operator-= ( const KIJVector& v )

   {
      _x -= v.x() ;
      _y -= v.y() ;
      _z -= v.z() ;

      return ( *this ) ;
   }



/*
   ==========================================================

	       AFFINE

   ==========================================================
*/

Point affine ( const Point& p1, const Point& p2, const double t ) 

   {
      Point p ;

      p._x = p1._x + t * ( p2._x - p1._x ) ;
      p._y = p1._y + t * ( p2._y - p1._y ) ;
      p._z = p1._z + t * ( p2._z - p1._z ) ;

      return ( p ) ;
   }

Point affine ( const Point& p1, 
	       const Point& p2, 
	       const Point& p3, 
	       const double a1,
	       const double a2,
	       const double a3 )

   {
      Point p ;

      p._x = a1 * p1._x + a2 * p2._x + a3 * p3._x ;
      p._y = a1 * p1._y + a2 * p2._y + a3 * p3._y ;
      p._z = a1 * p1._z + a2 * p2._z + a3 * p3._z ;

      return ( p ) ;
   }

Point average ( const Point *p, const int n ) 

   {
      double xsum = 0.0 ;
      double ysum = 0.0 ;
      double zsum = 0.0 ;

      double weight = 1.0 / ( (double) n ) ;

      for ( int i = 0 ; i < n ; i++ ) {
	 xsum += weight * p[i].x() ;
	 ysum += weight * p[i].y() ;
	 zsum += weight * p[i].z() ;
	 }

      return ( Point ( xsum, ysum, zsum ) ) ;
   }

Point average ( const std::list<Point>& p ) 

   {
      double xsum = 0.0 ;
      double ysum = 0.0 ;
      double zsum = 0.0 ;

		int n = p.size() ;
      double weight = 1.0 / ( (double) n ) ;
		std::list<Point>::const_iterator i ;

      for ( i = p.begin() ; i != p.end() ; i++ ) {
	 xsum += weight * i->x() ;
	 ysum += weight * i->y() ;
	 zsum += weight * i->z() ;
	 }

      return ( Point ( xsum, ysum, zsum ) ) ;
   }

Point midpoint ( const Point& p1, const Point& p2 )

   {
      return ( affine ( p1, p2, 0.5 ) ) ;
   }

// ==========================================================
//		Draw
// ==========================================================

#ifdef VIS
void glVertex3d( const Point& p )

	{
		glVertex3d( p._x, p._y, p._z ) ;
	}

void Point :: Draw (
	const double R,
	const double G,
	const double B,
	const double A,
	const double size
) const

	{
		GLfloat	colorv[4] = { R, G, B, A } ;

		if( A < 1.0 )
		{
			glDepthMask( GL_FALSE ) ;
		}

		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorv ) ;
		glColor4fv( colorv ) ;

		glPushMatrix() ;
		glTranslated( _x, _y, _z ) ;
		gluSphere( gluNewQuadric(), size, 10, 10 ) ;
		glPopMatrix() ;

		if( A < 1.0 )
		{
			glDepthMask( GL_TRUE ) ;
		}
	}
#endif
