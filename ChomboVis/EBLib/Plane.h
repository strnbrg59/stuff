#ifndef _Plane_h_
#define _Plane_h_

//  ============================================================
//                        Plane.h
//  ============================================================

#include <iostream>
#include "Point.h"
#include "KIJVector.h"
#include "CVPolygon.h"

class CVPolygon ;

class	Plane
{

	protected :
	
		Point		_anchor ;
		KIJVector	_dir ;

		void _Copy( const Plane& ) ;

	public :

			//  Constructors

		Plane () ;

		Plane ( const Point&, const KIJVector& ) ;

		Plane ( const CVPolygon& ) ;

			//  Copy Constructor

		Plane ( const Plane& ) ;

			//  Destructor

		virtual ~Plane () ;

			//  Assignment

		Plane& operator= ( const Plane& ) ;

			//  Output

		friend ostream& operator<< ( ostream&, const Plane& ) ;

			//  Comparison

		friend int operator== ( const Plane&, const Plane& ) ;
		friend int operator!= ( const Plane&, const Plane& ) ;
		friend Plane operator- ( const Plane& ) ;

			//  Query / Test functions

		int IsIn( const Point& ) const ;
		int IsOut( const Point& ) const ;
		int IsOn( const Point&, const double = 0.0 ) const ;

			//  Set / Access functions

		Point	anchor() const { return _anchor ; } ;
		KIJVector direction() const { return _dir ; } ;
		void SetAnchor( const Point& p ) { _anchor = p ; } ;
		void SetDirection( const KIJVector& v ) { _dir = v ; } ;

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
