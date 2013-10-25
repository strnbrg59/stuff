#include "astro.hpp"
#include "trace.hpp"
#include "ols.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
using std::endl;

StretchParams::StretchParams()
  : _A(boost::extents[2][1]), 
    _B(boost::extents[2][2]),
    _C(boost::extents[2][3])
{
    _B[0][0] = _B[1][1] = 1.0;
    _B[0][1] = _B[1][0] = 0.0;
    _FindIntuitiveParams();
}


StretchParams::StretchParams( StretchParams const & that )
  : _A( that._A ),
    _B( that._B ),
    _C( that._C ),
    _intuitive( that._intuitive )
{
}


StretchParams &
StretchParams::operator=( StretchParams const & that )
{
    if( this != &that )
    {
        _A = that._A;
        _B = that._B;
        _C = that._C;
        _intuitive = that._intuitive;
    }
    return *this;
}


/** Find (and store) the transformation that takes star_positions1 onto
 *  star_positions2.
 *  Arg polynomial_degree is the degree of the fitting function.
*/
void
StretchParams::Estimate(
    vector<Star> const & stars1,
    vector<Star> const & stars2,
    int polynomial_degree )
{
    Trace t("StretchParams::Estimate()");
    assert( stars1.size() == stars2.size() );
    assert( (polynomial_degree == 1)
        ||  (polynomial_degree == 2) );  // For now.

    _polynomial_degree = polynomial_degree;

    cvector lhs(boost::extents[2*stars1.size()]);
    matrix  rhs(boost::extents[2*stars1.size()]
                              [(1+polynomial_degree)*(2+polynomial_degree)]);
           
    for( unsigned i=0;i<stars1.size();++i )
    {
        lhs[2*i  ] = stars1[i].x();
        lhs[2*i+1] = stars1[i].y();

        // 0-th degree terms
        rhs[2*i  ][0] = 1.0;
        rhs[2*i+1][1] = 1.0;

        // 1st degree terms
        rhs[2*i  ][2] = stars2[i].x();
        rhs[2*i  ][3] = stars2[i].y();
        for( int j=4;j<6;++j )
        {
            rhs[2*i+1][j] = rhs[2*i][j-2];
        }

        // 2nd degree terms
        if( polynomial_degree == 2 )
        {
            rhs[2*i][6] = stars2[i].x() * stars2[i].x();
            rhs[2*i][7] = stars2[i].x() * stars2[i].y();
            rhs[2*i][8] = stars2[i].y() * stars2[i].y();
            for( int j=9;j<12;++j )
            {
                rhs[2*i+1][j] = rhs[2*i][j-3];
            }
        }
    }

    cvector beta( Ols( lhs, rhs ) );
    _A[0][0]  = beta[0];
    _A[1][0]  = beta[1];
    for( int j=0;j<2;++j )
    {
        _B[0][j] = beta[2+j];
        _B[1][j] = beta[2+2+j];
    }
    if( polynomial_degree == 2 )
    {
        for( int j=0;j<3;++j )
        {
            _C[0][j] = beta[6+j];
            _C[1][j] = beta[6+3+j];
        }
    }

    _FindIntuitiveParams();

    // Check residuals.  (Gotta find out how to use STL min_element/max_element
    // with boost::multi_array.)
    cvector yhat( MatCvectMult( rhs, beta ) );
    cvector resid( CvectMinus( lhs, yhat ) );
    double min_resid(9e300), max_resid(-9e300);
    for( int i=0;i<resid.size();++i )
    {
        min_resid = min_resid > resid[i] ? resid[i] : min_resid;
        max_resid = max_resid < resid[i] ? resid[i] : max_resid;
    }
    t.Info( "max abs(residual) = %f",
            std::max( fabs(min_resid), fabs(max_resid) ) );
}


/** Use *this and that to transform two random points, and then compare
 *  how close the transformed points wind up.
*/
double
StretchParams::DistanceTo( StretchParams const & that )
const
{
    double xthis[2] = {100,200},
           ythis[2] = {250,75},
           xthat[2] = {100,200},
           ythat[2] = {250,75};
    double result(0.0);
    for( int i=0;i<2;++i )
    {
        this->Transform( xthis+i, ythis+i );    
        that.Transform(  xthat+i, ythat+i );

        double dx( xthis[i]-xthat[i] );
        double dy( ythis[i]-ythat[i] );
        result += pow( dx*dx + dy*dy, 0.5 );
    }

    return result/2; // Normalize by the number of points we tested.
}


/** Applies the transformation to the point (x,y). */
void
StretchParams::Transform( double * x, double * y ) const
{
/*  This is the elegant way to do it.  Unfortunately it's 27 times slower than
    writing the expression out by hand.
    Md linear(2,1), quadratic(3,1); // terms
    linear[0][0] = *x;
    linear[1][0] = *y;
    quadratic[0][0] = *x  *  *x;
    quadratic[1][0] = *x  *  *y;
    quadratic[2][0] = *y  *  *y;

    Md result( _A + matmult(_B, linear) + matmult(_C, quadratic) );
    Md result( _A + matmult(_B, linear) );
    double xd( *x ), yd( *y );
    *x = result[0][0];
    *y = result[1][0];
*/

    double xd( *x ), yd( *y ); // Cuts time by another factor of 2, not to have
                               // to do lots of dereferencings.
    double xt = _A[0][0] + _B[0][0] * xd + _B[0][1] * yd
        + _C[0][0] * (xd)*(xd) + _C[0][1] * (xd)*(yd) + _C[0][2] * (yd)*(yd);
    *y = _A[1][0] + _B[1][0] * xd + _B[1][1] * yd
        + _C[1][0] * (xd)*(xd) + _C[1][1] * (xd)*(yd) + _C[1][2] * (yd)*(yd);
    *x = xt;
}


/** Returns norm of stretch in x and y */
double
StretchParams::StretchNorm() const
{
    return pow(  (  _intuitive.stretch_x*_intuitive.stretch_x
                  + _intuitive.stretch_y*_intuitive.stretch_y )/2.0, 0.5 );
}


/** Find what _ax, _ax, _bxx, etc correspond to in terms of shift, stretch,
 *  sheer and rotate.  Ignore 2nd-degree parameters (this->_C).
*/
void
StretchParams::_FindIntuitiveParams()
{
    double rotate = atan2( _B[1][0], _B[1][1] );
    double stretch_y = pow( _B[1][0]*_B[1][0] + _B[1][1]*_B[1][1], 0.5 );
    double sheer = (_B[0][0]*sin(rotate) + _B[0][1]*cos(rotate))
                 / (_B[0][0]*cos(rotate) - _B[0][1]*sin(rotate));
    double stretch_x = _B[0][0] / (cos(rotate) + sheer*sin(rotate));

    _intuitive.rotate = rotate;
    _intuitive.stretch_x = stretch_x;
    _intuitive.stretch_y = stretch_y;
    _intuitive.sheer = sheer;
    _intuitive.shift_x = _A[0][0];
    _intuitive.shift_y = _A[1][0];
}


std::ostream&
operator<<( std::ostream & out, StretchParams const & s )
{
    out << "(" << s._A[0][0] << "," << s._A[1][0] << "), (("
        << s._B[0][0] << "," << s._B[0][1] << "),("
        << s._B[1][0] << "," << s._B[1][1] << "))" << ", (("
        << s._C[0][0] << "," << s._C[0][1] << "," << s._C[0][2] << "),("
        << s._C[1][0] << "," << s._C[1][1] << "," << s._C[1][2] << "))" << endl
        << "  [Intuitive: shift=[" << s._intuitive.shift_x << ", "
                                  << s._intuitive.shift_y << "], "
                    << " stretch=[" << s._intuitive.stretch_x << ", "
                                    << s._intuitive.stretch_y << "], "
                    << " sheer=" << s._intuitive.sheer << ", "
                    << " rotate=" << s._intuitive.rotate << "]";

    return out;
}
