// ============================================================
//    Class EBFindPlane
//
//       Encapsulates a root finding routine for obtaining
//       the plane that intersects a volume within a given
//       tolerance. The result is a fraction indicating
//       the anchor point along a line, where one point
//       is anchored at 0.0 and the other is anchored at 1.0.
//
//    (c) Christopher S. Co
// ============================================================

#include <iostream>
#include <math.h>

#include "EBFindPlane.h"

using std::cerr;
using std::endl;

// ============================================================
//    Constructors, Destructor
// ============================================================

EBFindPlane :: EBFindPlane ()
   {
      _volComputer = NULL ;
      _cell3D = NULL ;
      _cell2D = NULL ;
      _tol = 1.0e-5 ;
      _maxiter = 100 ;
   }

EBFindPlane :: ~EBFindPlane ()
   {
   }

// ============================================================
//    Yet another min function
// ============================================================

inline double EBFindPlane :: min ( double x, double y )
   {
      return ( x<y ? x : y ) ;
   }

// ============================================================
//    The find routine
// ============================================================

Point EBFindPlane :: find ( const KIJVector& n )
   {
      if( _volComputer == NULL )
      {
         cerr  << "EBFindPlane::find(): "
               << "must have pointer to volume calculator" << endl ;
         return -1.0 ;
      }

      double t ;

      _volComputer->normal( n ) ;
      _volComputer->ObtainBrackets() ;

      t = _find() ;

      return _volComputer->GetPlane( t ) ;
   }

   //
   //  The following is an implementation of "Brent's Method"
   //    for one-dimensional root finding. Pseudo-code for this
   //    algorithm can be found on p. 253 of "Numerical Recipes"
   //    ISBN 0-521-30811-9
   //
double EBFindPlane :: _find ()
   {
         //  Floating point precision
      const double         EPS      = 3.0e-8 ;

      //unsigned int i ;
      int i ;
      double a, b, c, fa, fb, fc ;
      double d, e ;
      double tol1, xm ;
      double p, q, r, s ;

      a = 0.0 ;
      b = 1.0 ;
      fa = _volComputer->volume( a ) ;
      fb = _volComputer->volume( b ) ;

         //  Init these to be safe
      c = d = e = 0.0 ;

      if( fb*fa > 0 )
      {
         cerr  << "EBFindPlane::_find(): "
               << "Root must be bracketed (around zero)" << endl ;
         return 0.0 ;
      }
      fc = fb ;

      for( i=0; i<_maxiter; i++ )
      {
         if( fb*fc > 0 )
         {
               //  Rename a, b, c and adjust bounding interval d
            c  = a ;
            fc = fa ;
            d  = b - a ;
            e  = d ;
         }

         if( fabs(fc) < fabs(fb) )
         {
            a  = b ;
            b  = c ;
            c  = a ;
            fa = fb ;
            fb = fc ;
            fc = fa ;
         }

            //  Convergence check
         tol1  = 2.0 * EPS * fabs(b) + 0.5 * _tol ;
         xm    = 0.5 * ( c - b ) ;

         if( fabs(xm) <= tol1 || fb == 0.0 )
         {
            break ;
         }

         if( fabs(e) >= tol1 && fabs(fa) > fabs(fb) )
         {
               //  Attempt inverse quadratic interpolation
            s  = fb / fa ;
            if( a == c )
            {
               p  = 2.0 * xm * s ;
               q  = 1.0 - s ;
            }
            else
            {
               q  = fa / fc ;
               r  = fb / fc ;
               p  = s * ( 2.0 * xm * q * (q-r) - (b-a) * (r-1.0) ) ;
               q  = (q-1.0) * (r-1.0) * (s-1.0) ;
            }

               //  Check whether in bounds
            if( p > 0 ) q = -q ;

            p = fabs( p ) ;

            if( 2.0 * p < min( 3.0*xm*q-fabs(tol1*q), fabs(e*q) ) )
            {
                  //  Accept interpolation
               e  = d ;
               d  = p / q ;
            }
            else
            {
                  //  Interpolation failed, use bisection
               d  = xm ;
               e  = d ;
            }
         }
         else
         {
               //  Bounds decreasing too slowly, use bisection
            d  = xm ;
            e  = d ;
         }

            //  Move last best guess to a
         a  = b ;
         fa = fb ;

            //  Evaluate new trial root
         if( fabs(d) > tol1 )
         {
            b  = b + d ;
         }
         else
         {
            if( xm < 0 )   b = b - tol1 ;
            else           b = b + tol1 ;
         }
         fb = _volComputer->volume(b) ;
      }

      if( i >= _maxiter )
         cerr  << "EBFindPlane::_find(): exceeding maximum iterations: "
               << _maxiter << endl ;

      return b ;
   }

// ============================================================
//    Set Routines
// ============================================================

void EBFindPlane :: volume ( EBVolumeCalculator * v )
   {
      if( v )
      {
         _volComputer = v ;
      }
   }

void EBFindPlane :: cell2D ( CVPolygon * c )
   {
      if( c )
      {
         _cell2D = c ;
      }
   }

void EBFindPlane :: cell3D ( Hexahedron * c )
   {
      if( c )
      {
         _cell3D = c ;
      }
   }

/*
void EBFindPlane :: lowerBound ( Point& p )
   {
      _lb = p ;
   }

void EBFindPlane :: upperBound ( Point& p )
   {
      _ub = p ;
   }
*/

void EBFindPlane :: tolerance ( double t )
   {
      if( t >= 0.0 )
      {
         _tol = t ;
      }
      else
      {
         cerr  << "EBFindPlane::tolerance(): "
               << "tolerance must be non-negative value" << endl ;
      }
   }

void EBFindPlane :: target ( double t )
   {
      if( _volComputer )
      {
         _volComputer->target( t ) ;
      }
      else
      {
         cerr  << "EBFindPlane::target(): "
               << "cannot set target without volume computer" << endl ;
      }
   }

void EBFindPlane :: maxIter ( int m )
   {
      if( m >= 0 )
      {
         _maxiter = m ;
      }
      else
      {
         cerr  << "EBFindPlane::maxIter(): "
               << "value must be non-negative" << endl ;
      }
   }

// ============================================================
//    Access Routines
// ============================================================

double EBFindPlane :: tolerance () const
   {
      return _tol ;
   }

double EBFindPlane :: target () const
   {
      if( _volComputer )
      {
         return _volComputer->target() ;
      }
      return 0.0 ;
   }
