// ============================================================
//                      CVPolygon.C
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

#include "CVPolygon.h"

#define		TRUE		1 
#define		FALSE		0

// ==========================================================
//      CONSTRUCTORS, DESTRUCTOR
// ==========================================================

CVPolygon :: CVPolygon ()
   {
      _verts = NULL ;
		_size = 0 ;
   }

CVPolygon :: CVPolygon ( std::list<Point>& pl )
	{
		int i, n ;

		n = pl.size() ;

		if( n >= 3 )
		{
			_verts = new Point [n] ;
			if( _verts )
			{
				std::list<Point>::iterator pi ;
				for( i=0, pi=pl.begin(); i<n; i++, pi++ )
				{
      			_verts[i] = *pi ;
				}
				_size = n ;
			}
			else
			{
				cerr	<< "CVPolygon::CVPolygon(): insufficient memory"
						<< endl ;
				abort() ;
			}
		}
		else
		{
			cerr	<< "CVPolygon::CVPolygon(): insufficient points"
					<< endl ;
			_size = 0 ;
			_verts = NULL ;
		}
	}

CVPolygon :: CVPolygon ( const Point* pl, const int n )
   {
		if( pl && n >= 3 )
		{
			_verts = new Point [n] ;
			if( _verts )
			{
				int i ;

				for( i=0; i<n; i++ )
				{
      			_verts[i] = pl[i] ;
				}
				_size = n ;
			}
			else
			{
				cerr	<< "CVPolygon::CVPolygon(): insufficient memory"
						<< endl ;
				abort() ;
			}
		}
		else
		{
			cerr	<< "CVPolygon::CVPolygon(): NULL point list or insufficient points"
					<< endl ;
			_size = 0 ;
			_verts = NULL ;
		}
   }

CVPolygon :: CVPolygon ( Point* pl, const int n )
   {
		if( pl && n >= 3 )
		{
			_verts = new Point [n] ;
			if( _verts )
			{
				int i ;

				for( i=0; i<n; i++ )
				{
      			_verts[i] = pl[i] ;
				}
				_size = n ;
			}
			else
			{
				cerr	<< "CVPolygon::CVPolygon(): insufficient memory"
						<< endl ;
				abort() ;
			}
		}
		else
		{
			cerr	<< "CVPolygon::CVPolygon(): NULL point list or insufficient points"
					<< endl ;
			_size = 0 ;
			_verts = NULL ;
		}
   }

CVPolygon :: CVPolygon ( const CVPolygon& p ) 
   {
			//  The following is somewhat of a "hack" to prevent
			//    "delete" from seg faulting... 
			//    To see what I mean, uncomment the following two lines
			//    and see what happens when the copy contructor is invoked.
			//
			//  The main problem with this is that if we invoke the copy
			//    constructor explicitly, there is a memory leak.
			//
			//  This should not be a problem for implicit calls to the copy
			//     constructor, on the other hand.
		_verts = NULL ;
		_size = 0 ;

		_Copy( p ) ;
   }

CVPolygon :: ~CVPolygon ()
	{
		delete [] _verts ;
      _verts = NULL ;
	}
      
void CVPolygon :: _Copy( const CVPolygon& p )
	{
		unsigned int i ;
//cerr << "CVPolygon::_Copy()" << endl ;

		if( _size == p._size )
		{
//cerr << "same size" << endl ;
			for( i=0; i<_size; i++ )
			{
				_verts[i] = p._verts[i] ;
			}
		}
		else
		{
//cerr << "must resize" << endl ;
// TODO memory leak... for some reason will not work if i uncomment above line
         if( _verts )   delete [] _verts ;
			_size = p._size ;
			_verts = new Point [_size] ;
			if( _verts )
			{
				for( i=0; i<_size; i++ )
				{
					_verts[i] = p._verts[i] ;
				}
			}
			else
			{
				cerr	<< "CVPolygon::_Copy(): insufficient memory"
						<< endl ;
				abort() ;
			}
		}
	}

// ==========================================================
//      OPERATOR=
// ==========================================================

CVPolygon& CVPolygon :: operator= ( const CVPolygon& p ) 
   {
      if ( this == &p ) return ( *this ) ;

		_Copy( p ) ;

      return ( *this ) ;
   }

// ==========================================================
//      OUTPUT
// ==========================================================

ostream& operator<< ( ostream& co, const CVPolygon& p )
   {
		unsigned int i ;

      co	<< "{ " ;
		for( i=0; i<p._size; i++ )
		{
			co	<< p._verts[i] << ( i!=(p._size-1) ? ", " : "" ) ;
		}
		co	<< " }" ;

      return co ;
   }

// ==========================================================
//      COMPARISON OPERATIONS
// ==========================================================

int operator== ( const CVPolygon& p1, const CVPolygon& p2 ) 
   {
		if( p1._size != p2._size )
		{
			return FALSE ;
		}

		unsigned int i ;
		for( i=0; i<p1._size; i++ )
		{
			if( p1._verts[i] != p2._verts[i] )
			{
				return FALSE ;
			}
		}

		return TRUE ;
   }

int operator!= ( const CVPolygon& p1, const CVPolygon& p2 ) 
   {
		return !( p1==p2 ) ;
   }

// ==========================================================
//      ARITHMETIC OPERATIONS
// ==========================================================

CVPolygon operator+ ( const CVPolygon& p, const KIJVector& v ) 
   {
		std::list<Point> plist ;
		unsigned int i ;

		for( i=0; i<p._size; i++ )
		{
			plist.push_back( p._verts[i] + v ) ;
		}

		return CVPolygon( plist ) ;
   }

CVPolygon operator+ ( const KIJVector& v, const CVPolygon& p )
   {
		std::list<Point> plist ;
		unsigned int i ;

		for( i=0; i<p._size; i++ )
		{
			plist.push_back( p._verts[i] + v ) ;
		}

		return CVPolygon( plist ) ;
   }

CVPolygon operator- ( const CVPolygon& p, const KIJVector& v ) 
   {
		std::list<Point> plist ;
		unsigned int i ;

		for( i=0; i<p._size; i++ )
		{
			plist.push_back( p._verts[i] - v ) ;
		}

		return CVPolygon( plist ) ;
   }

CVPolygon operator- ( const KIJVector& v, const CVPolygon& p )
   {
		std::list<Point> plist ;
		unsigned int i ;

		for( i=0; i<p._size; i++ )
		{
			plist.push_back( p._verts[i] - v ) ;
		}

		return CVPolygon( plist ) ;
   }

CVPolygon& CVPolygon :: operator+= ( const KIJVector& v )
   {
		unsigned int i ;

		for( i=0; i<_size; i++ )
		{
			_verts[i] += v ;
		}

		return *this ;
   }

CVPolygon& CVPolygon :: operator-= ( const KIJVector& v ) 
   {
		unsigned int i ;

		for( i=0; i<_size; i++ )
		{
			_verts[i] -= v ;
		}

		return *this ;
   }

// ==========================================================
//      ACCESS FUNCTIONS FOR POINT ITERATION
// ==========================================================

Point CVPolygon :: operator[] ( const int idx ) const
   {
		if( (unsigned int) idx < 0 || (unsigned int) idx >= _size )
		{
			cerr	<< "CVPolygon :: operator[] (const): Access out of bounds: "
					<< idx << "requested in range [0,"
					<< _size << ")"
					<< endl ;
			exit( 200 ) ;
		}

		return _verts[idx] ;
   }

Point& CVPolygon :: operator[] ( const int idx )
   {
  		if( (unsigned int) idx < 0 || (unsigned int) idx >= _size )
		{
			cerr	<< "CVPolygon :: operator[]: Access out of bounds: "
					<< idx << " requested in range [0,"
					<< _size << ")"
					<< endl ;
			exit( 200 ) ;
		}

		return _verts[idx] ;
 }

// ==========================================================
//      ACCESS / QUERY FUNCTIONS
// ==========================================================

// TODO : IsValidPoly() <-- check coplanarity
int CVPolygon :: IsValidPoly() const
	{
		if( _size < 3 || _verts == NULL )
		{
			return FALSE ;
		}

		return TRUE ;
	}

// TODO : IsInside()
int CVPolygon :: IsInside( const Point & p ) const
	{
		return FALSE ;
	}

// ==========================================================
//    Edge access functions
//
//    Edges are numbered by the index of the vertex
//    touching the edge on its right, if the edge is
//    considered to "point" outward.
//
//    For example:
//
//           2
//       3 ----- 2
//       |       |
//     3 |       | 1
//       |       |
//       0 ----- 1
//           0
//
// ==========================================================

int CVPolygon :: Edge ( int i, LineSegment& edge ) const
   {
      if( unsigned(i) > 0 && unsigned(i) < _size )
      {
         edge.left( _verts[(i+1)%_size] ) ;
         edge.right( _verts[i] ) ;
         return 1 ;
      }
      return 0 ;
   }

// ==========================================================
//      CALCULATIONS
// ==========================================================

int CVPolygon :: ClipIn( const Plane & cutPlane, CVPolygon& result ) const
	{
		unsigned int	i, currPt, prevPt ;
		double	d ;			// Value of dot product ;
		double	pd = 0.0 ;	// Value of previous dot product ;
		double	t ;			// Affine "fraction" for intersection calc
		Point		I ;			// Intersection point
		Point		*Q ;			// Alias for _verts
		std::list<Point>	in ;		// Collection of intersection points.

		const KIJVector& n = cutPlane.direction() ;  // Plane's normal
		const Point& P = cutPlane.anchor() ;         // Plane's anchor
		Q = _verts ;

		if( ! IsValidPoly() )
		{
			return 0 ;
		}

		for( i=0; i<=_size; i++ )
		{
			currPt = i % _size ;
			prevPt = (i-1) ;

			d = dot ( n, Q[currPt] - P ) ;

			if( d*pd < 0 )
			{
				t = pd / ( pd - d ) ;
				I = Q[prevPt] + t * ( Q[currPt] - Q[prevPt] ) ;
				in.push_back( I ) ;
			}
			if( d >= 0.0 && i != _size )
			{
				in.push_back( Q[currPt] ) ;
			}
			pd = d ;
		}

		if( in.size() >= 3 )
      {
			result = CVPolygon( in ) ;
         return 1 ;
      }

		return 0 ;
	}

int CVPolygon :: ClipInEx( const Plane & cutPlane, CVPolygon& result ) const
	{
		unsigned int	i, currPt, prevPt ;
		double	d ;			// Value of dot product ;
		double	pd = 0.0 ;	// Value of previous dot product ;
		double	t ;			// Affine "fraction" for intersection calc
		Point		I ;			// Intersection point
		Point		*Q ;			// Alias for _verts
		std::list<Point>	in ;		// Collection of intersection points.

		const KIJVector& n = cutPlane.direction() ;  // Plane's normal
		const Point& P = cutPlane.anchor() ;         // Plane's anchor
		Q = _verts ;

		if( ! IsValidPoly() )
		{
			return 0 ;
		}

		for( i=0; i<=_size; i++ )
		{
			currPt = i % _size ;
			prevPt = (i-1) ;

			d = dot ( n, Q[currPt] - P ) ;

			if( d*pd < 0 )
			{
				t = pd / ( pd - d ) ;
				I = Q[prevPt] + t * ( Q[currPt] - Q[prevPt] ) ;
				in.push_back( I ) ;
			}
			if( d > 0.0 && i != _size )
			{
				in.push_back( Q[currPt] ) ;
			}
			pd = d ;
		}

		if( in.size() >= 3 )
      {
			result = CVPolygon( in ) ;
         return 1 ;
      }

		return 0 ;
	}

int CVPolygon :: ClipOut( const Plane & cutPlane, CVPolygon& result ) const
	{
		Plane	reverseCutPlane( cutPlane.anchor(), -cutPlane.direction() ) ;
		return ClipIn( reverseCutPlane, result ) ;
	}

int CVPolygon :: ClipOutEx( const Plane & cutPlane, CVPolygon& result ) const
	{
		Plane	reverseCutPlane( cutPlane.anchor(), -cutPlane.direction() ) ;
		return ClipInEx( reverseCutPlane, result ) ;
	}

Point CVPolygon :: Centroid () const
	{
		return average( _verts, _size ) ;
	}

KIJVector CVPolygon :: Normal () const
   {
      KIJVector a,b,normal ;

		a = _verts[1] - _verts[0] ;
		b = _verts[2] - _verts[0] ;

		normal = cross( a, b ) ;
		normal.normalize() ;

      return normal ;
   }

double CVPolygon :: Area () const
	{
		double	area ;
		unsigned int i, j ;

		area = 0.0 ;

		for( i=1, j=2; j<_size; i++, j++ )
		{
			KIJVector a, b ;

			a = _verts[i] - _verts[0] ;
			b = _verts[j] - _verts[0] ;

			area += 0.5 * cross( a, b ).length() ;
		}

		return area ;
	}

void CVPolygon :: Reverse ()
   {
      unsigned int i ;
      Point tmp ;
      for( i=0; i<_size/2; i++ )
      {
         tmp = _verts[i] ;
         _verts[i] = _verts[_size-i-1] ;
         _verts[_size-i-1] = tmp ;
      }
   }

// ==========================================================
//      DRAWING ROUTINES
// ==========================================================

#ifdef VIS
void CVPolygon :: DrawWire
   (
	   const double R,
	   const double G,
	   const double B,
	   const double A
   ) const
	{
		if( ! IsValidPoly() )
			return ;

		unsigned int i ;

		if( A < 1.0 )
		{
			glDepthMask( GL_FALSE ) ;
		}

		glPushAttrib( GL_ENABLE_BIT | GL_LIGHTING | GL_LIGHT0 ) ;
		glDisable( GL_LIGHTING ) ;

		glBegin( GL_LINE_LOOP ) ;

		glColor4d( R, G, B, A ) ;
		for( i=0; i<_size; i++ )
		{
			glVertex3d( _verts[i].x(), _verts[i].y(), _verts[i].z() ) ;
		}

		glEnd() ;

		glPopAttrib() ;

		if( A < 1.0 )
		{
			glDepthMask( GL_TRUE ) ;
		}
	}

void CVPolygon :: DrawSolid
   (
	   const double R,
	   const double G,
	   const double B,
	   const double A
   ) const
	{
		if( ! IsValidPoly() )
			return ;

		unsigned int i ;
		KIJVector normal ;
		GLfloat	colorv[4] = { R, G, B, A } ;

      normal = Normal() ;

		if( A < 1.0 )
		{
			glDepthMask( GL_FALSE ) ;
		}

		glBegin( GL_POLYGON ) ;

		//glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE ) ;
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorv ) ;
		glColor4fv( colorv ) ;
		//glColor4d( R, G, B, A ) ;

		glNormal3d( normal.x(), normal.y(), normal.z() ) ;
		for( i=0; i<_size; i++ )
		{
			glVertex3d( _verts[i].x(), _verts[i].y(), _verts[i].z() ) ;
		}

		glEnd() ;

		if( A < 1.0 )
		{
			glDepthMask( GL_TRUE ) ;
		}
	}
#endif
