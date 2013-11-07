#ifndef _CVPolygon_h_
#define _CVPolygon_h_

//  ============================================================
//                        CVPolygon.h
//  ============================================================

#include <iostream>
#include "Point.h"
#include "Plane.h"
#include "LineSegment.h"
#include "KIJVector.h"
#include <list>

class LineSegment ;
class Plane ;

class	CVPolygon
{
//  A CVPolygon can also be thought of as a plane, so...
	friend class Plane ;

	protected :
	
		Point				* _verts ;
		unsigned int	_size ;

		void _Copy( const CVPolygon& ) ;

	public :

			//  Constructors, Destructor
		CVPolygon () ;
		CVPolygon ( std::list<Point>& ) ;
		CVPolygon ( Point *, const int ) ;
		CVPolygon ( const Point *, const int ) ;
		CVPolygon ( const CVPolygon& ) ;
		virtual ~CVPolygon () ;

			//  Assignment
		CVPolygon& operator= ( const CVPolygon& ) ;

			//  Output
		friend ostream& operator<< ( ostream&, const CVPolygon& ) ;

			//  Comparison
		friend int operator== ( const CVPolygon&, const CVPolygon& ) ;
		friend int operator!= ( const CVPolygon&, const CVPolygon& ) ;

			//  Arithmetic Operations
		friend CVPolygon operator+ ( const CVPolygon&, const KIJVector& ) ;
		friend CVPolygon operator+ ( const KIJVector&, const CVPolygon& ) ;
		friend CVPolygon operator- ( const CVPolygon&, const KIJVector& ) ;
		friend CVPolygon operator- ( const KIJVector&, const CVPolygon& ) ;

		CVPolygon& operator+= ( const KIJVector& ) ;
		CVPolygon& operator-= ( const KIJVector& ) ;

			//  Access Functions for Point Iteration
		unsigned int size () const { return _size ; } ;
		Point operator[] ( const int ) const ;
		Point& operator[] ( const int ) ;

			//  Access/Query Functions
		unsigned int NumSides () const { return _size ; } ;
		int IsValidPoly() const ;
		int IsInside( const Point & ) const ;

         // Edge access functions
      int Edge ( int, LineSegment& ) const ;

			//  Calculations
		int ClipIn( const Plane&, CVPolygon& ) const ;
		int ClipInEx( const Plane&, CVPolygon& ) const ;
		int ClipOut( const Plane&, CVPolygon& ) const ;
		int ClipOutEx( const Plane&, CVPolygon& ) const ;
		Point Centroid() const ;
      KIJVector Normal() const ;
		double Area() const ;
      void Reverse () ;

			//  Draw
#ifdef VIS
		void DrawWire( 
			const double = 1.0,
			const double = 1.0,
			const double = 1.0,
			const double = 1.0
			) const ;
		void DrawSolid (
			const double = 1.0,
			const double = 1.0,
			const double = 1.0,
			const double = 1.0
			) const ;
#endif

} ;

static const Point _UnitSquareVertices[] =
	{
		Point( 0.0, 0.0, 0.0 ),
		Point( 1.0, 0.0, 0.0 ),
		Point( 1.0, 1.0, 0.0 ),
		Point( 0.0, 1.0, 0.0 )
	} ;

const CVPolygon UnitSquare( _UnitSquareVertices, 4 ) ;

#endif
