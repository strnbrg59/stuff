#ifndef _Tetrahedron_h_
#define _Tetrahedron_h_

//  ============================================================
//                        Tetrahedron.h
//  ============================================================

#include <iostream>
#include "Point.h"

class	Tetrahedron
{

	private :
	
			//  The vertices of the tet.
			//  The first 3 are expected to be in counterclockwise order
		Point		_verts[4] ;

		void _Copy( const Tetrahedron& ) ;

	public :

			//  Constructors

		Tetrahedron () ;

		Tetrahedron ( const Point&, const Point&, const Point&, const Point& ) ;

		Tetrahedron ( Point * ) ;

			//  Copy Constructor

		Tetrahedron ( const Tetrahedron& ) ;

			//  Destructor

		virtual ~Tetrahedron () ;

			//  Assignment

		Tetrahedron& operator= ( const Tetrahedron& ) ;

			//  Output

		friend ostream& operator<< ( ostream&, const Tetrahedron& ) ;

			//  Comparison

		friend int operator== ( const Tetrahedron&, const Tetrahedron& ) ;
		friend int operator!= ( const Tetrahedron&, const Tetrahedron& ) ;

			//  Arithmetic Operations

		friend Tetrahedron operator+ ( const Tetrahedron&, const KIJVector& ) ;
		friend Tetrahedron operator+ ( const KIJVector&, const Tetrahedron& ) ;
		friend Tetrahedron operator- ( const Tetrahedron&, const KIJVector& ) ;
		friend Tetrahedron operator- ( const KIJVector&, const Tetrahedron& ) ;

		Tetrahedron& operator+= ( const KIJVector& ) ;
		Tetrahedron& operator-= ( const KIJVector& ) ;

			//  Calculations

		double Volume () const ;
		Point Centroid () const ;

			//  Draw
#ifdef VIS
		void DrawWire(
				const double = 1.0,
				const double = 1.0,
				const double = 1.0,
				const double = 1.0
			) const ;
		void DrawSolid(
				const double = 1.0,
				const double = 1.0,
				const double = 1.0,
				const double = 1.0
			) const ;
#endif

} ;

#endif
