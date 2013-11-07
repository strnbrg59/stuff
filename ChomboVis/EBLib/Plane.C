// ============================================================
//                      Plane.C
// ============================================================

#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <stream.h>

#ifdef VIS
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#include "Plane.h"

#define		TRUE		1 
#define		FALSE		0


// ==========================================================
//      CONSTRUCTORS, DESTRUCTOR
// ==========================================================

Plane :: Plane ()

   {
      _anchor = PointZero ;
      _dir = VZero ;
   }

Plane :: Plane ( const CVPolygon& p )

   {
		if( p.IsValidPoly() )
		{
			KIJVector a, b ;

			a = p._verts[1] - p._verts[0] ;
			b = p._verts[2] - p._verts[0] ;

      	_anchor = p._verts[0] ;
      	_dir = cross( a, b ) ;
		}
		else
		{
			cerr	<< "Plane::Plane: Invalid polygon; cannot obtain plane"
					<< endl ;
			abort() ;
		}
   }

Plane :: Plane ( const Point& x, const KIJVector& y )

   {
      _anchor = x ;
      _dir = y ;
   }

Plane :: Plane ( const Plane& p ) 

   {
		_Copy( p ) ;
   }

Plane :: ~Plane () { }
      
void Plane :: _Copy( const Plane& p )

	{
      _anchor = p._anchor ;
      _dir = p._dir ;
	}


// ==========================================================
//      OPERATOR=
// ==========================================================

Plane& Plane :: operator= ( const Plane& p ) 

   {
      if ( this == &p ) return ( *this ) ;

		_Copy( p ) ;

      return ( *this ) ;
   }

// ==========================================================
//      OUTPUT
// ==========================================================

ostream& operator<< ( ostream& co, const Plane& p )

   {
      co	<< "{ "
			<< p._anchor << ", "
			<< p._dir
			<< " }" ;

      return co ;
   }

// ==========================================================
//      COMPARISON OPERATIONS
// ==========================================================

int operator== ( const Plane& p1, const Plane& p2 ) 

   {
		if ( ( p1._anchor == p2._anchor ) &&
           ( p1._dir == p2._dir ) )
			return ( TRUE ) ;
		else
			return ( FALSE ) ;
   }

int operator!= ( const Plane& p1, const Plane& p2 ) 

   {
		if ( ( p1._anchor != p2._anchor ) ||
           ( p1._dir != p2._dir ) )
			return ( TRUE ) ;
		else
			return ( FALSE ) ;
   }

// ==========================================================
//      FLIPPING THE PLANE
// ==========================================================

Plane operator- ( const Plane& p ) 

   {
      return Plane( p._anchor, -p._dir ) ;
   }

// ==========================================================
//      QUERY / TEST FUNCTIONS
// ==========================================================

// TODO : IsIn() IsOut() IsOn() <-- require fudge factor?

int Plane :: IsIn ( const Point & p ) const

	{
		double	d ;

		d = dot( _dir, p - _anchor ) ;

		if( d > 0.0 )
			return TRUE ;

		return FALSE ;
	}

int Plane :: IsOut ( const Point & p ) const

	{
		double	d ;

		d = dot( _dir, p - _anchor ) ;

		if( d < 0.0 )
			return TRUE ;

		return FALSE ;
	}

int Plane :: IsOn ( const Point & p, const double tolerance ) const

	{
		double	d ;

#define	_QUICK_ABS( x ) ( (x)<0 ? -(x) : (x) )
		d = dot( _dir, p - _anchor ) ;

		if( tolerance == 0.0 )
		{
			if( d == 0.0 )
				return TRUE ;
		}
		else
		{
				//  Allow for a fudge factor, or "tolerance"
			if( _QUICK_ABS( d ) < tolerance )
				return TRUE ;
		}

		return FALSE ;
	}

// ==========================================================
//      DRAW ROUTINES
// ==========================================================

#ifdef VIS
void Plane :: DrawWire (
	double R,
	double G,
	double B,
	double A
) const

	{
		_dir.DrawWire( _anchor, R, G, B, A ) ;
	}
	
void Plane :: DrawSolid (
	double R,
	double G,
	double B,
	double A
) const

	{
		_dir.DrawSolid( _anchor, R, G, B, A ) ;
	}
#endif
