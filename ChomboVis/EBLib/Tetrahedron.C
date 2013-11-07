// ============================================================
//                      Tetrahedron.C
// ============================================================

#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <stream.h>
#include "Tetrahedron.h"
#include "CVPolygon.h"

#define		TRUE		1 
#define		FALSE		0


// ==========================================================
//      CONSTRUCTORS, DESTRUCTOR
// ==========================================================

Tetrahedron :: Tetrahedron ()

   {
      _verts[0] = PointZero ;
      _verts[1] = PointZero ;
      _verts[2] = PointZero ;
      _verts[3] = PointZero ;
   }

Tetrahedron :: Tetrahedron (
	const Point& x, const Point& y,
	const Point& z, const Point& w
)

   {
      _verts[0] = x ;
      _verts[1] = y ;
      _verts[2] = z ;
      _verts[3] = w ;
   }

Tetrahedron :: Tetrahedron ( Point* pl )

   {
		if( pl )
		{
				//  We need to trust there will
				//  not be a segmentation fault here...
      	_verts[0] = pl[0] ;
      	_verts[1] = pl[1] ;
      	_verts[2] = pl[2] ;
      	_verts[3] = pl[3] ;
		}
		else
		{
			cerr	<< "Tetrahedron::Tetrahedron(): NULL point list"
					<< endl ;
			abort() ;
		}
   }

Tetrahedron :: Tetrahedron ( const Tetrahedron& p ) 

   {
		_Copy( p ) ;
   }

Tetrahedron :: ~Tetrahedron () { }
      
void Tetrahedron :: _Copy( const Tetrahedron& p )

	{
      _verts[0] = p._verts[0] ;
      _verts[1] = p._verts[1] ;
      _verts[2] = p._verts[2] ;
      _verts[3] = p._verts[3] ;
	}


// ==========================================================
//      OPERATOR=
// ==========================================================

Tetrahedron& Tetrahedron :: operator= ( const Tetrahedron& p ) 

   {
      if ( this == &p ) return ( *this ) ;

		_Copy( p ) ;

      return ( *this ) ;
   }

// ==========================================================
//      OUTPUT
// ==========================================================

ostream& operator<< ( ostream& co, const Tetrahedron& p )

   {
      co	<< "{ "
			<< p._verts[0] << ", "
			<< p._verts[1] << ", "
			<< p._verts[2] << ", "
			<< p._verts[3]
			<< " }" ;

      return co ;
   }

// ==========================================================
//      COMPARISON OPERATIONS
// ==========================================================

int operator== ( const Tetrahedron& p1, const Tetrahedron& p2 ) 

   {
      if ( ( p1._verts[0] == p2._verts[0] ) &&
           ( p1._verts[1] == p2._verts[1] ) &&
           ( p1._verts[2] == p2._verts[2] ) &&
           ( p1._verts[3] == p2._verts[3] ) )
	    return ( TRUE ) ;
	 else
	    return ( FALSE ) ;
   }

int operator!= ( const Tetrahedron& p1, const Tetrahedron& p2 ) 

   {
      if ( ( p1._verts[0] != p2._verts[0] ) ||
           ( p1._verts[1] != p2._verts[1] ) ||
           ( p1._verts[2] != p2._verts[2] ) ||
           ( p1._verts[3] != p2._verts[3] ) )
	    return ( TRUE ) ;
	 else
	    return ( FALSE ) ;
   }

// ==========================================================
//      ARITHMETIC OPERATIONS
// ==========================================================

Tetrahedron operator+ ( const Tetrahedron& p, const KIJVector& v ) 

   {
		Point plist[4] ;
		unsigned int i ;

		for( i=0; i<4; i++ )
		{
			plist[i] = p._verts[i] + v ;
		}

		return Tetrahedron( plist ) ;
   }

Tetrahedron operator+ ( const KIJVector& v, const Tetrahedron& p )

   {
		Point plist[4] ;
		unsigned int i ;

		for( i=0; i<4; i++ )
		{
			plist[i] = p._verts[i] + v ;
		}

		return Tetrahedron( plist ) ;
   }

Tetrahedron operator- ( const Tetrahedron& p, const KIJVector& v ) 

   {
		Point plist[4] ;
		unsigned int i ;

		for( i=0; i<4; i++ )
		{
			plist[i] = p._verts[i] - v ;
		}

		return Tetrahedron( plist ) ;
   }

Tetrahedron operator- ( const KIJVector& v, const Tetrahedron& p )

   {
		Point plist[4] ;
		unsigned int i ;

		for( i=0; i<4; i++ )
		{
			plist[i] = p._verts[i] - v ;
		}

		return Tetrahedron( plist ) ;
   }

Tetrahedron& Tetrahedron :: operator+= ( const KIJVector& v )

   {
		unsigned int i ;

		for( i=0; i<4; i++ )
		{
			_verts[i] += v ;
		}

		return *this ;
   }

Tetrahedron& Tetrahedron :: operator-= ( const KIJVector& v ) 

   {
		unsigned int i ;

		for( i=0; i<4; i++ )
		{
			_verts[i] -= v ;
		}

		return *this ;
   }


// ==========================================================
//      CALCULATIONS
// ==========================================================

double Tetrahedron :: Volume() const

	{
		KIJVector	a, b, c ;
		double	volume ;
		const double	Sixth = 1.0 / 6.0 ;

		a = _verts[1] - _verts[0] ;
		b = _verts[2] - _verts[0] ;
		c = _verts[3] - _verts[0] ;

			//  This is 1/6-th of a parallelepiped
		volume = Sixth * dot( cross( a, b ), c ) ;
		//volume = Sixth * cross(a,b).length() * c.length() ;

		return ( volume<0 ? -volume : volume ) ;
	}

Point Tetrahedron :: Centroid () const

	{
		return average( _verts, 4 ) ;
	}

// ==========================================================
//      DRAWING ROUTINES
// ==========================================================

#ifdef VIS
void Tetrahedron :: DrawWire
(
	const double R,
	const double G,
	const double B,
	const double A
) const

	{
		Point	face[3] ;
		CVPolygon	tmpPoly ;

		face[0] = _verts[0] ;
		face[1] = _verts[1] ;
		face[2] = _verts[2] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawWire( R, G, B, A ) ;

		face[0] = _verts[0] ;
		face[1] = _verts[2] ;
		face[2] = _verts[3] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawWire( R, G, B, A ) ;

		face[0] = _verts[0] ;
		face[1] = _verts[3] ;
		face[2] = _verts[1] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawWire( R, G, B, A ) ;

		face[0] = _verts[1] ;
		face[1] = _verts[3] ;
		face[2] = _verts[2] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawWire( R, G, B, A ) ;
	}
	
void Tetrahedron :: DrawSolid
(
	const double R,
	const double G,
	const double B,
	const double A
) const

	{
		Point	face[3] ;
		CVPolygon	tmpPoly ;

		face[0] = _verts[0] ;
		face[1] = _verts[1] ;
		face[2] = _verts[2] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawSolid( R, G, B, A ) ;

		face[0] = _verts[0] ;
		face[1] = _verts[2] ;
		face[2] = _verts[3] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawSolid( R, G, B, A ) ;

		face[0] = _verts[0] ;
		face[1] = _verts[3] ;
		face[2] = _verts[1] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawSolid( R, G, B, A ) ;

		face[0] = _verts[1] ;
		face[1] = _verts[3] ;
		face[2] = _verts[2] ;
		tmpPoly = CVPolygon( face, 3 ) ;
		tmpPoly.DrawSolid( R, G, B, A ) ;
	}
#endif
