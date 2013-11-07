
//
// ============================================================
//
//                      VECTOR.C
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

#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <stream.h>

#ifdef VIS
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#include "KIJVector.h"
#include "Point.h"

#define		TRUE		1 
#define		FALSE		0


/*
   ==========================================================

	       CONSTRUCTORS, DESTRUCTOR

   ==========================================================
*/

KIJVector :: KIJVector ()

   {
      _vx = 0.0 ;
      _vy = 0.0 ;
      _vz = 0.0 ;
   }

KIJVector :: KIJVector ( const double x, const double y, const double z )

   {
      _vx = x ;
      _vy = y ;
      _vz = z ;
   }

KIJVector :: KIJVector ( const KIJVector& v ) 

   {
      _vx = v._vx ;
      _vy = v._vy ;
      _vz = v._vz ;
   }

KIJVector :: ~KIJVector () { }
      

/*
   ==========================================================

	       OPERATOR=

   ==========================================================
*/

KIJVector& KIJVector :: operator= ( const KIJVector& v ) 

   {
      if ( this == &v ) return ( *this ) ;

      _vx = v._vx ;
      _vy = v._vy ;
      _vz = v._vz ;

      return ( *this ) ;
   }

/*
   ==========================================================

	       OUTPUT

   ==========================================================
*/

ostream& operator<< ( ostream& co, const KIJVector& v )

   {
      co << "< "
	 << v._vx 
	 << ", "
	 << v._vy 
	 << ", "
	 << v._vz 
	 << " >" ;

      return co ;
   }

/*
   ==========================================================

	       COMPARISON OPERATIONS

   ==========================================================
*/

int operator== ( const KIJVector& v1, const KIJVector& v2 ) 

   {
      if ( ( v1._vx == v2._vx ) &&
           ( v1._vy == v2._vy ) &&
           ( v1._vz == v2._vz ) )
	    return ( TRUE ) ;
	 else
	    return ( FALSE ) ;
   }

int operator!= ( const KIJVector& v1, const KIJVector& v2 ) 

   {
      if ( ( v1._vx != v2._vx ) ||
           ( v1._vy != v2._vy ) ||
           ( v1._vz != v2._vz ) )
	    return ( TRUE ) ;
	 else
	    return ( FALSE ) ;
   }

/*
   ==========================================================

	       ARITHMETIC OPERATIONS

   ==========================================================
*/

KIJVector operator+ ( const KIJVector& v1, const KIJVector& v2 ) 

   {
      KIJVector vv ;

      vv._vx = v1._vx + v2._vx ;
      vv._vy = v1._vy + v2._vy ;
      vv._vz = v1._vz + v2._vz ;

      return ( vv ) ;
   }

KIJVector operator- ( const KIJVector& v1, const KIJVector& v2 ) 

   {
      KIJVector vv ;

      vv._vx = v1._vx - v2._vx ;
      vv._vy = v1._vy - v2._vy ;
      vv._vz = v1._vz - v2._vz ;

      return ( vv ) ;
   }

KIJVector operator- ( const KIJVector& v ) 

   {
      KIJVector vv ;

      vv._vx = - v._vx ;
      vv._vy = - v._vy ;
      vv._vz = - v._vz ;

      return ( vv ) ;
   }

KIJVector operator* ( const double& c, const KIJVector& v ) 

   {
      KIJVector vv ;

      vv._vx = c * v._vx ;
      vv._vy = c * v._vy ;
      vv._vz = c * v._vz ;

      return ( vv ) ;
   }

KIJVector operator* ( const KIJVector& v, const double& c ) 

   {
      KIJVector vv ;

      vv._vx = c * v._vx ;
      vv._vy = c * v._vy ;
      vv._vz = c * v._vz ;

      return ( vv ) ;
   }

KIJVector operator/ ( const KIJVector& v, const double& c ) 

   {
      KIJVector vv ;

      vv._vx = v._vx / c ;
      vv._vy = v._vy / c ;
      vv._vz = v._vz / c ;

      return ( vv ) ;
   }

KIJVector& KIJVector :: operator+= ( const KIJVector& v ) 

   {
      _vx += v._vx ;
      _vy += v._vy ;
      _vz += v._vz ;

      return *this ;
   }

KIJVector& KIJVector :: operator-= ( const KIJVector& v ) 

   {
      _vx -= v._vx ;
      _vy -= v._vy ;
      _vz -= v._vz ;

      return *this ;
   }

KIJVector& KIJVector :: operator*= ( const double& c ) 

   {
      _vx *= c ;
      _vy *= c ;
      _vz *= c ;

      return *this ;
   }

KIJVector& KIJVector :: operator/= ( const double& c ) 

   {
      _vx /= c ;
      _vy /= c ;
      _vz /= c ;

      return *this ;
   }

/*
   ==========================================================

	       NORMALIZE

   ==========================================================
*/

void KIJVector :: normalize ()  

   {
      double l =  length () ;

      _vx = _vx / l ;
      _vy = _vy / l ;
      _vz = _vz / l ;
   }

/*
   ==========================================================

	       LENGTH

   ==========================================================
*/

double KIJVector :: length ()  const

   {
      double		l ;

      l =  sqrt ( _vx * _vx + _vy * _vy + _vz * _vz ) ;
      return ( l ) ;
   }

/*
   ==========================================================

	       DOT

   ==========================================================
*/

double dot ( const KIJVector& v1, const KIJVector& v2 ) 

   {
      double		d ;

      d =  v1._vx * v2._vx + v1._vy * v2._vy + v1._vz * v2._vz ;

      return ( d ) ;
   }


/*
   ==========================================================

	       CROSS

   ==========================================================
*/

KIJVector cross ( const KIJVector& v1, const KIJVector& v2 ) 

   {
      KIJVector vv ;

      vv._vx = v1._vy * v2._vz - v1._vz * v2._vy ;
      vv._vy = - v1._vx * v2._vz + v1._vz * v2._vx ;
      vv._vz = v1._vx * v2._vy - v1._vy * v2._vx ;

      return ( vv ) ;
   }

// ==========================================================
//		Draw
// ==========================================================
#ifdef VIS
void glRotated( const double theta, const KIJVector& v )

	{
		glRotated( theta, v._vx, v._vy, v._vz ) ;
	}

void glTranslated( const KIJVector& v )

	{
		glTranslated( v._vx, v._vy, v._vz ) ;
	}

void glNormal3d( const KIJVector& n )

	{
		glNormal3d( n._vx, n._vy, n._vz ) ;
	}

void KIJVector :: DrawWire (
	const Point& p,
	const double R,
	const double G,
	const double B,
	const double A
) const

	{
		GLfloat colorv[4] = { R, G, B, A } ;
 		const double thick = 0.02 ;
		double len, theta, phi ;

		len	= length() ;
		theta	= atan2( _vz, _vx ) * 180.0 / M_PI ;
		phi	= atan2( _vy, sqrt( _vx*_vx + _vz*_vz ) ) * 180.0 / M_PI ;

		if( A < 1.0 )
			glDepthMask( GL_FALSE ) ;
 
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorv ) ;
		glColor4fv( colorv ) ;

		glPushAttrib( GL_ENABLE_BIT | GL_LIGHTING | GL_LIGHT0 ) ;
		glDisable( GL_LIGHTING ) ;

		glPushMatrix() ;
 
		glTranslated( p - PointZero ) ;
 
		glPushMatrix() ;
 
		glRotated( -theta, VY ) ;
		glRotated( phi, VZ ) ;
		glScaled( len, thick, thick ) ;
 
		glBegin( GL_LINE_STRIP ) ;
		glVertex3d( 0.0, 0.0, 0.0 ) ;
		glVertex3d( 1.0, 0.0, 0.0 ) ;
		glEnd() ;
	 
		glBegin( GL_POLYGON ) ;
		glVertex3d( 0.85, 1.0, 0.0 ) ;
		glVertex3d( 0.85, -1.0, 0.0 ) ;
		glVertex3d( 1.0, 0.0, 0.0 ) ;
		glEnd() ;
 
		glPopMatrix() ;
 
		glPopMatrix() ;

		glPopAttrib() ;

		if( A < 1.0 )
			glDepthMask( GL_TRUE ) ;
	}

void KIJVector :: DrawSolid (
	const Point& p,
	const double R,
	const double G,
	const double B,
	const double A
) const

	{
		GLfloat colorv[4] = { R, G, B, A } ;
		int i ;
		const double thick = 0.02 ;
		double len, theta, phi ;

		len	= length() ;
		theta	= atan2( _vz, _vx ) * 180.0 / M_PI ;
		phi	= atan2( _vy, sqrt( _vx*_vx + _vz*_vz ) ) * 180.0 / M_PI ;

		if( A < 1.0 )
			glDepthMask( GL_FALSE ) ;

		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorv ) ;
		glColor4fv( colorv ) ;

		glPushMatrix() ;
 
		glTranslated( p - PointZero ) ;
 
		glPushMatrix() ;
 
		glRotated( -theta, VY ) ;
		glRotated( phi, VZ ) ;
		glScaled( len, thick, thick ) ;
 
		for( i = 0 ; i < 4 ; i++ )
		{
			glBegin( GL_POLYGON ) ;
			glNormal3d( 0.0, 0.0, 1.0 ) ;
			glVertex3d( 0.0, 0.5, 0.5 ) ;
			glVertex3d( 0.85, 0.5, 0.5 ) ;
			glVertex3d( 0.85, -0.5, 0.5 ) ;
			glVertex3d( 0.0, -0.5, 0.5 ) ;
			glEnd() ;
	 
			glBegin( GL_POLYGON ) ;
			glNormal3d( -1.0, 0.0, 0.0 ) ;
			glVertex3d( 0.85, 1.0, 1.0 ) ;
			glVertex3d( 0.85, 0.5, 0.5 ) ;
			glVertex3d( 0.85, -0.5, 0.5 ) ;
			glVertex3d( 0.85, -1.0, 1.0 ) ;
			glEnd() ;
	 
			glBegin( GL_POLYGON ) ;
			Point p1 = Point( 0.85, 1.0, 1.0 ) ;
			Point p2 = Point( 0.85, -1.0, 1.0 ) ;
			Point p3 = Point( 1.0, 0.0, 0.0 ) ;
			KIJVector n = cross( p2-p1, p3-p1 ) ;
			n.normalize() ;

			glNormal3d( n ) ;
			glVertex3d( p1 ) ;
			glVertex3d( p2 ) ;
			glVertex3d( p3 ) ;
			glEnd() ;
	 
			glRotated( 90.0, VX ) ;
		}
 
		glPopMatrix() ;
 
		glPopMatrix() ;

		if( A < 1.0 )
			glDepthMask( GL_TRUE ) ;
	}
#endif
