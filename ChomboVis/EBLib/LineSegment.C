
#ifdef VIS
#include <GL/gl.h>
#endif

#include "LineSegment.h"
using std::cerr;
using std::endl;

// ============================================================
//    Internal Copy Routine
// ============================================================

void LineSegment :: _Copy ( const LineSegment& l )
   {
      _left = l._left ;
      _right = l._right ;
   }

// ============================================================
//    Constructors, Destructor
// ============================================================

LineSegment :: LineSegment ()
   {
   }

LineSegment :: LineSegment ( const LineSegment& l )
   {
      _Copy( l ) ;
   }

LineSegment :: ~LineSegment ()
   {
   }

// ============================================================
//    Operations
// ============================================================

LineSegment& LineSegment:: operator= ( const LineSegment& l )
   {
      if( &l == this ) return *this ;
      _Copy( l ) ;
      return *this ;
   }

Point& LineSegment :: operator[] ( int i )
   {
      switch( i )
      {
         case 0 :
            return _left ;
         case 1 :
            return _right ;
      }
      cerr  << "LineSegment::operator[](): "
            << "invalid index" << endl ;
      return _right ;
   }

int LineSegment :: size () const
   {
      return 2 ;
   }

// ============================================================
//    Arithmetic Operations
// ============================================================

LineSegment operator+ ( const LineSegment& l, const KIJVector& v ) 
   {
      LineSegment result ;

      result.left( l._left + v ) ;
      result.right( l._right + v ) ;

		return result ;
   }

LineSegment& LineSegment :: operator+= ( const KIJVector& v )
   {
      _left += v ;
      _right += v ;
		return *this ;
   }


// ============================================================
//    Calculations
// ============================================================

int LineSegment :: clipIn ( const Plane& cutPlane, LineSegment& result ) const
   {
      const KIJVector& n = cutPlane.direction() ;  // Plane's normal
      const Point& P = cutPlane.anchor() ;         // Plane's anchor

      double pd, d, t ;
      Point I ;

      pd = dot( n, _left - P ) ;
      d = dot( n, _right - P ) ;

         // Case when the segment is completely "IN"
      if( d >= 0.0 && pd >= 0.0 )
      {
         result._left = _left ;
         result._right = _right ;
         return 1 ;
      }

         // Case(s) when the segment is partially "IN"
      if( d*pd < 0 )
      {
            // Compute the intersection
         t = pd / ( pd - d ) ;
         I = _left + t * ( _right - _left ) ;

            // Right point is in
         if( d >= 0.0 )
         {
            result._left = I  ;
            result._right = _right ;
            return 1 ;
         }

            // Left point is in
         if( pd >= 0.0 )
         {
            result._left = _left ;
            result._right = I ;
            return 1 ;
         }
      }

      return 0 ;
   }

int LineSegment :: clipInEx ( const Plane& cutPlane, LineSegment& result ) const
   {
      const KIJVector& n = cutPlane.direction() ;  // Plane's normal
      const Point& P = cutPlane.anchor() ;         // Plane's anchor

      double pd, d, t ;
      Point I ;

      pd = dot( n, _left - P ) ;
      d = dot( n, _right - P ) ;

         // Case when the segment is completely "IN"
      if( d > 0.0 && pd > 0.0 )
      {
         result._left = _left ;
         result._right = _right ;
         return 1 ;
      }

         // Case(s) when the segment is partially "IN"
      if( d*pd < 0 )
      {
            // Compute the intersection
         t = pd / ( pd - d ) ;
         I = _left + t * ( _right - _left ) ;

            // Right point is in
         if( d >= 0.0 )
         {
            result._left = I  ;
            result._right = _right ;
            return 1 ;
         }

            // Left point is in
         if( pd >= 0.0 )
         {
            result._left = _left ;
            result._right = I ;
            return 1 ;
         }
      }

      return 0 ;
   }

int LineSegment :: clipOut ( const Plane& cutPlane, LineSegment& result ) const
   {
      Plane reverseCutPlane( cutPlane.anchor(), -cutPlane.direction() ) ;
      return clipIn( reverseCutPlane, result ) ;
   }

int LineSegment :: clipOutEx ( const Plane& cutPlane, LineSegment& result ) const
   {
      Plane reverseCutPlane( cutPlane.anchor(), -cutPlane.direction() ) ;
      return clipInEx( reverseCutPlane, result ) ;
   }

void LineSegment :: reverse ()
   {
      Point tmp ;
      tmp = _left ;
      _left = _right ;
      _right = tmp ;
   }

Point LineSegment :: centroid ()
   {
      return midpoint( _left, _right ) ;
   }

// ============================================================
//    Draw
// ============================================================

#ifdef VIS
void LineSegment :: Draw ()
   {
      glBegin( GL_LINE_LOOP ) ;
      glVertex3d( _left ) ;
      glVertex3d( _right ) ;
      glEnd() ;
   }
#endif

// ============================================================
//    Access Routines
// ============================================================

Point& LineSegment :: left ()
   {
      return _left ;
   }

Point& LineSegment :: right ()
   {
      return _right ;
   }

// ============================================================
//    Set Routines
// ============================================================

void LineSegment :: left ( const Point& p )
   {
      _left = p  ;
   }

void LineSegment :: right ( const Point& p )
   {
      _right = p ;
   }

