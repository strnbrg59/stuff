#include <math.h>
#include "rcm.h"

//static double box_kernel( double u, double h );
static double gaussian_kernel( double u, double h );

Md kernelDensity( const Vd& xx, double h ) {
// Kernel estimator of a density.  Kernels are Gaussian with s.e. h.
//
// Argument xx should be a row vector.

    if( xx.getOrientation() != horizontal )
    {
        rcmError( "kernelDensity: arg1 should be a row vector." );
    }
    Vd x( sort(xx) );

    const int n_pts=400; // resolution desired.  Good choice is to
        // set it to the number of pixels across one of your graphs.

    Md result( n_pts,2 );
    
    // Points along the estimated density:
    Md z( 2, n_pts );   // First row: abscissa
                        // Second row: ordinate

    // Set up abscissa: it should go from minimum x to maximum x, with
    // 3h leeway off the ends.
    double min_z = -rowmax(-x)[0] - 3*h;
    double max_z = rowmax(x)[0] + 3*h;
    double z_intval = (max_z - min_z)/(n_pts-1);
    for( int i=0;i<z.cols();i++ )
        z[0][i] = min_z + i*z_intval;

    // Set up ordinate by scanning the x's within +- 3*h of each abscissa.
    int left_x = 0; // index of left-most x that matters

    for( int i=0; i<z.cols(); i++ ) {
        int j=left_x;

        while( ( j < x.size() ) 
        &&       ( x[j] - z[0][i] < 3*h ) )
        {
            if( z[0][i] - x[j] > 3*h )
                left_x ++;
            z[1][i] += gaussian_kernel( x[j] - z[0][i], h );
//          z[1][i] += box_kernel     ( x[j] - z[0][i], h );
            j++;
        }

        z[1][i] /= x.size();
    }

    // plot
//  scatterplotX( z[M(0)], z[M(1)] );

    // Copy z to return value
    result = transp(z);
    return result;

} // kernelDensity()
//--------------------

static double gaussian_kernel( double u, double h ) {
// Gaussian.  h is s.e.

    const double sqrt2pi = pow( 2*3.141592654, 0.5 );

    double denom = (1/(h*sqrt2pi));
    double mantissa = -pow(u/h,2.0)/2.0;
    double result = denom * exp( mantissa );
    return result;

} // gaussian_kernel

/*
static double box_kernel( double u, double h ) {
    double result;
    if( fabs(u) < h ) result = 1;
    else              result = 0;
    return result;
} // box_kernel()
*/

