#ifndef _Hexahedron_h_
#define _Hexahedron_h_

//  ============================================================
//                        Hexahedron.h
//  ============================================================

#include <iostream>
#include "Point.h"
#include "CVPolygon.h"

class	Hexahedron
{

	protected :
	
			//  The 8 vertices of the hexadedron.
			//  The order of these vertices is very important
			//
			//          2 ---- 6
			//         /|     /|       y
			//        3 ---- 7 |       |
			//        | |    | |       *-- x
			//        | 0 ---| 4      /
			//        |/     |/      z
			//        1 ---- 5
			//
		Point		_verts[8] ;

		void _Copy( const Hexahedron& ) ;
		void _Draw( const int ) const ;

	public :

			//  Constructors, Destructor
		Hexahedron () ;
		Hexahedron ( const Point * ) ;
		Hexahedron ( const Hexahedron& ) ;
		virtual ~Hexahedron () ;

			//  Assignment
		Hexahedron& operator= ( const Hexahedron& ) ;

			//  Output
		friend ostream& operator<< ( ostream&, const Hexahedron& ) ;

			//  Comparison
		friend int operator== ( const Hexahedron&, const Hexahedron& ) ;
		friend int operator!= ( const Hexahedron&, const Hexahedron& ) ;

			//  Access functions for point iteration
		unsigned int size() const { return 8 ; } ;
		Point operator[] ( const int ) const ;
		Point& operator[] ( const int ) ;

			//  Face access functions
		CVPolygon	Front() const ;
		CVPolygon	Back() const ;
		CVPolygon	Left() const ;
		CVPolygon	Right() const ;
		CVPolygon	Top() const ;
		CVPolygon	Bottom() const ;
      int  Face ( int, CVPolygon& ) const ;

			//  Calculations
		double Volume() const ;
		Point Centroid() const ;
		double LongestDiagonal() const ;

			//  Draw
      #ifdef VIS
		void Draw
         ( const double, const double, const double, const double =1.0) const ;
		void DrawWire() const ;
		void DrawSolid() const ;
      #endif

} ;

static const Point	_UnitCubeVertices[] =
	{
		Point( 0.0, 0.0, 0.0 ),
		Point( 0.0, 0.0, 1.0 ),
		Point( 0.0, 1.0, 0.0 ),
		Point( 0.0, 1.0, 1.0 ),
		Point( 1.0, 0.0, 0.0 ), 
		Point( 1.0, 0.0, 1.0 ),
		Point( 1.0, 1.0, 0.0 ),
		Point( 1.0, 1.0, 1.0 )
	} ;

const Hexahedron UnitCubeHexahedron( _UnitCubeVertices ) ;

#endif
