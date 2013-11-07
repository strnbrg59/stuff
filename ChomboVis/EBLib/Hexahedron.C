// ============================================================
//                      Hexahedron.C
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

#include <list>
#include <vector>
#include "Hexahedron.h"
#include "Tetrahedron.h"

#define		TRUE		1 
#define		FALSE		0

typedef enum { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM } ;

// ==========================================================
//      CONSTRUCTORS, DESTRUCTOR
// ==========================================================

Hexahedron :: Hexahedron ()
   {
		int i ;
		for( i=0; i<8; i++ )
		{
      	_verts[i] = PointZero ;
		}
   }

Hexahedron :: Hexahedron ( const Point * pl )
	{
		if( pl )
		{
			int i ;
			for( i=0; i<8; i++ )
			{
				_verts[i] = pl[i] ;
			}
		}
		else
		{
			cerr	<< "Hexahedron :: Hexahedron : NULL point list pointer"
					<< endl ;
			abort( ) ;
		}
	}

Hexahedron :: Hexahedron ( const Hexahedron& p ) 
   {
		_Copy( p ) ;
   }

Hexahedron :: ~Hexahedron ()
   {
   }
      
void Hexahedron :: _Copy( const Hexahedron& p )
	{
		int i ;

		for( i=0; i<8; i++ )
		{
      	_verts[i] = p._verts[i] ;
		}
	}


// ==========================================================
//      OPERATOR=
// ==========================================================

Hexahedron& Hexahedron :: operator= ( const Hexahedron& p ) 
   {
      if ( this == &p ) return ( *this ) ;

		_Copy( p ) ;

      return ( *this ) ;
   }

// ==========================================================
//      OUTPUT
// ==========================================================

ostream& operator<< ( ostream& co, const Hexahedron& p )
   {
		int i ;

      co	<< "{ " ;
		for( i=0; i<8; i++ )
		{
			co << p._verts[i] << ( i!=7 ? ", " : "" ) ;
		}
		co	<< " }" ;

      return co ;
   }

// ==========================================================
//      COMPARISON OPERATIONS
// ==========================================================

int operator!= ( const Hexahedron& p1, const Hexahedron& p2 ) 
   {
		return !(p1 == p2) ;
   }

int operator== ( const Hexahedron& p1, const Hexahedron& p2 ) 
   {
		int i ;

		for( i=0; i<8; i++ )
		{
			if( p1._verts[i] != p2._verts[i] )
			{
				return FALSE ;
			}
		}
		return TRUE ;
   }

// ==========================================================
//      ACCESS FUNCTIONS FOR POINT ITERATION
// ==========================================================

Point Hexahedron :: operator[] ( const int idx ) const
   {
		if( idx < 0 || idx > 7 )
		{
			cerr	<< "Hexahedron :: operator[] (const): Access out of bounds: "
					<< idx << "requested in range [0,7]"
					<< endl ;
			exit( 200 ) ;
		}

		return _verts[idx] ;
   }

Point& Hexahedron :: operator[] ( const int idx )
   {
  		if( idx < 0 || idx > 7 )
		{
			cerr	<< "Hexahedron :: operator[]: Access out of bounds: "
					<< idx << "requested in range [0,7]"
					<< endl ;
			exit( 200 ) ;
		}

		return _verts[idx] ;
 }

// ==========================================================
//      FACE ACCESS
// ==========================================================

CVPolygon Hexahedron :: Front () const
	{
		Point	points[4] ;

		points[0] = _verts[1] ;
		points[1] = _verts[5] ;
		points[2] = _verts[7] ;
		points[3] = _verts[3] ;

		return CVPolygon( points, 4 ) ;
	}

CVPolygon Hexahedron :: Back () const
	{
		Point	points[4] ;

		points[0] = _verts[0] ;
		points[1] = _verts[2] ;
		points[2] = _verts[6] ;
		points[3] = _verts[4] ;

		return CVPolygon( points, 4 ) ;
	}

CVPolygon Hexahedron :: Left () const
	{
		Point	points[4] ;

		points[0] = _verts[0] ;
		points[1] = _verts[1] ;
		points[2] = _verts[3] ;
		points[3] = _verts[2] ;

		return CVPolygon( points, 4 ) ;
	}

CVPolygon Hexahedron :: Right () const
	{
		Point	points[4] ;

		points[0] = _verts[4] ;
		points[1] = _verts[6] ;
		points[2] = _verts[7] ;
		points[3] = _verts[5] ;

		return CVPolygon( points, 4 ) ;
	}

CVPolygon Hexahedron :: Top () const
	{
		Point	points[4] ;

		points[0] = _verts[3] ;
		points[1] = _verts[7] ;
		points[2] = _verts[6] ;
		points[3] = _verts[2] ;

		return CVPolygon( points, 4 ) ;
	}

CVPolygon Hexahedron :: Bottom () const
	{
		Point	points[4] ;

		points[0] = _verts[0] ;
		points[1] = _verts[4] ;
		points[2] = _verts[5] ;
		points[3] = _verts[1] ;

		return CVPolygon( points, 4 ) ;
	}

   // Faces are returned according to encoding
   // of faces given in ConnectedHexahedron
int Hexahedron :: Face ( int i, CVPolygon& face ) const
   {
      switch( i )
      {
         case 0 :
            face = Left() ;
            return 1 ;
         case 1 :
            face = Right() ;
            return 1 ;
         case 2 :
            face = Bottom() ;
            return 1 ;
         case 3 :
            face = Top() ;
            return 1 ;
         case 4 :
            face = Back() ;
            return 1 ;
         case 5 :
            face = Front() ;
            return 1 ;
      }
      cerr  << "Hexahedron::Face(): "
            << "face index must be >= 0 and < 6" << endl ;
      return 0 ;
   }

// ==========================================================
//      VOLUME CALCULATIONS
// ==========================================================

double Hexahedron :: Volume () const

	{
		Point		centroid ;
		Point		faceCentroid ;
		CVPolygon	faces[6] ;
		Tetrahedron	currTet ;
		double	volume = 0.0 ;
		unsigned int i,j,k ;

		centroid = Centroid();

			//  Split the hexahedron into tets,
			//    and sum their volumes

		faces[0] = Front() ;
		faces[1] = Back() ;
		faces[2] = Left() ;
		faces[3] = Right() ;
		faces[4] = Top() ;
		faces[5] = Bottom() ;

		for( i=0; i<6; i++ )
		{
			faceCentroid = faces[i].Centroid() ;
				//  We know all the faces have 4 sides
			for( j=0, k=1; j<4; j++, k=((j+1) % 4 ))
			{
				currTet = Tetrahedron
					(
						(faces[i])[j],
						(faces[i])[k],
						faceCentroid,
						centroid
					) ;
				volume += currTet.Volume() ;
			}
		}

		return volume ;
	}

Point Hexahedron :: Centroid () const

	{
		int i ;
		double xx, yy, zz ;

		xx = yy = zz = 0 ;

		for( i=0; i<8; i++ )
		{
			xx += 0.125 * _verts[i].x() ;
			yy += 0.125 * _verts[i].y() ;
			zz += 0.125 * _verts[i].z() ;
		}

		return Point( xx, yy, zz ) ;
	}

double Hexahedron :: LongestDiagonal () const

	{
		double	distance = 0.0 ;
		double	diagLength ;
		int		i, j ;

			//  Calculate all the diagonal lengths and 
			//    record the longest one.
			//  NOTE: The verts are ordered such that
			//    the following loop works
		for( i=0, j=7; i<4 ; i++, j-- )
		{
			diagLength = ( _verts[i] - _verts[j] ).length() ;
			if( distance < diagLength )
			{
				distance = diagLength ;
			}
		}

		return distance ;
	}

// ==========================================================
//      DRAWING ROUTINES
// ==========================================================

#ifdef VIS
void Hexahedron :: Draw
   (
      const double R,
      const double G,
      const double B,
      const double A
   ) const

   {
      unsigned char faces[6][4] =
         {
            { 1, 5, 7, 3 }, // front
            { 0, 2, 6, 4 }, // back
            { 0, 1, 3, 2 }, // left
            { 4, 6, 7, 5 }, // right
            { 3, 7, 6, 2 }, // top
            { 0, 4, 5, 1 }  // bottom
         } ;
      unsigned int i, j ;
		GLfloat	colorv[4] = { R, G, B, A } ;

		if( A < 1.0 )
		{
			glDepthMask( GL_FALSE ) ;
		}

/*
      glEnable( GL_VERTEX_ARRAY ) ;
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorv ) ;
		glColor4fv( colorv ) ;
      glVertexPointer( 3, GL_DOUBLE, sizeof(Point), _verts ) ;
      for( i=0; i<6; i++ )
         glDrawElements( GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, faces[i] );
*/
      glPushAttrib( GL_POLYGON_BIT | GL_POLYGON_MODE ) ;
      glCVPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) ;
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorv ) ;
		glColor4fv( colorv ) ;
      glBegin( GL_QUADS ) ;
      for( i=0; i<6; i++ )
         for( j=0; j<4; j++ )
            glVertex3d( _verts[ faces[i][j] ] ) ;
      glEnd() ;
      glPopAttrib() ;

		if( A < 1.0 )
		{
			glDepthMask( GL_TRUE ) ;
		}
   }

void Hexahedron :: DrawWire () const

	{
		_Draw( GL_LINE_LOOP ) ;
	}
	
void Hexahedron :: DrawSolid () const

	{
		_Draw( GL_POLYGON ) ;
	}

// TODO : Hexahedron::_Draw() <-- Clean this up
void Hexahedron :: _Draw( int renderMode ) const

	{
float	red[4]		= { 1.0, 0.0, 0.0, 0.8 } ;
float	green[4]		= { 0.0, 1.0, 0.0, 0.8 } ;
float	blue[4]		= { 0.0, 0.0, 1.0, 0.8 } ;
float	yellow[4]	= { 1.0, 1.0, 0.0, 0.8 } ;
float	cyan[4]		= { 0.0, 1.0, 1.0, 0.8 } ;
float	magenta[4]	= { 1.0, 0.0, 1.0, 0.8 } ;

		if( renderMode == GL_LINE_LOOP )
		{
			glPushAttrib( GL_ENABLE_BIT | GL_LIGHTING | GL_LIGHT0 ) ;
			glDisable( GL_LIGHTING ) ;
		}

#define	_HexahedronDrawVertex( p ) glVertex3d( (p).x(), (p).y(), (p).z() ) ;
		// Front
		glBegin( renderMode ) ;
		glNormal3d( 0.0, 0.0, 1.0 ) ;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
		glColor4fv( red ) ;
		_HexahedronDrawVertex( _verts[1] ) ;
		_HexahedronDrawVertex( _verts[5] ) ;
		_HexahedronDrawVertex( _verts[7] ) ;
		_HexahedronDrawVertex( _verts[3] ) ;
		glEnd() ;

		// Back
		glBegin( renderMode ) ;
		glNormal3d( 0.0, 0.0, -1.0 ) ;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
		glColor4fv( yellow ) ;
		_HexahedronDrawVertex( _verts[0] ) ;
		_HexahedronDrawVertex( _verts[2] ) ;
		_HexahedronDrawVertex( _verts[6] ) ;
		_HexahedronDrawVertex( _verts[4] ) ;
		glEnd() ;

		// Left
		glBegin( renderMode ) ;
		glNormal3d( -1.0, 0.0, 0.0 ) ;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
		glColor4fv( green ) ;
		_HexahedronDrawVertex( _verts[0] ) ;
		_HexahedronDrawVertex( _verts[1] ) ;
		_HexahedronDrawVertex( _verts[3] ) ;
		_HexahedronDrawVertex( _verts[2] ) ;
		glEnd() ;

		// Right
		glBegin( renderMode ) ;
		glNormal3d( 1.0, 0.0, 0.0 ) ;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cyan);
		glColor4fv( cyan ) ;
		_HexahedronDrawVertex( _verts[4] ) ;
		_HexahedronDrawVertex( _verts[6] ) ;
		_HexahedronDrawVertex( _verts[7] ) ;
		_HexahedronDrawVertex( _verts[5] ) ;
		glEnd() ;

		// Top
		glBegin( renderMode ) ;
		glNormal3d( 0.0, 1.0, 0.0 ) ;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
		glColor4fv( blue ) ;
		_HexahedronDrawVertex( _verts[3] ) ;
		_HexahedronDrawVertex( _verts[7] ) ;
		_HexahedronDrawVertex( _verts[6] ) ;
		_HexahedronDrawVertex( _verts[2] ) ;
		glEnd() ;

		// Bottom
		glBegin( renderMode ) ;
		glNormal3d( 0.0, -1.0, 0.0 ) ;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, magenta);
		glColor4fv( magenta ) ;
		_HexahedronDrawVertex( _verts[0] ) ;
		_HexahedronDrawVertex( _verts[4] ) ;
		_HexahedronDrawVertex( _verts[5] ) ;
		_HexahedronDrawVertex( _verts[1] ) ;
		glEnd() ;

		if( renderMode == GL_LINE_LOOP )
		{
			glPopAttrib() ;
		}
	}
#endif
