#include <math.h>
#include "rcm.h"

static double gaussian_kernel( double x );
static double box_kernel( double x );

/** Smooth with the prescribed kernel.
 *  y's first row is the "y" coordinates.  If y has a second row, then
 *    that's the corresponding "x" coordinates.  If y doesn't have a
 *    second row, we assume the "y" coordinates are equally spaces.
 *  For Gaussian_kernel, argument h determines the scale factor a
 *  for the kernel (a being set so that at a distance h we have 
 *  only 1% weight).
 * For box_kernel, h is the width of the kernel.
*/
Md smooth( const Md y_, double h, const char* kernelName ) 
{

    int n=y_.cols();
    double totweight, weight, a;
    Vd result( n, horizontal );

    // Check for a second row on y.  If there is one, that's the
    //    x-axis values to which h applies.  If there isn't assume
    //    equally-spaced x values.
    Md y(y_);
    if( y.rows() == 1 )
        y = catrows(y,iota(y.cols()));

    // Calculate a, the scale factor for the Gaussian kernel.
    typedef double (*KernelType)(double);
    KernelType kernel;
    if( string(kernelName) == "gaussian" )
    {
        a = h/pow(-log(.01),0.5);
        kernel = gaussian_kernel;
    }
    else if( string(kernelName) == "box" )
    {
        kernel = box_kernel;
        a = 1.0;
    }
    else
        rcmError( string("smooth(): valid kernelName options are ") +
                  string("\"gaussian\" and \"box\".") );

    for( int i=0;i<n;i++ ) {
        totweight=0;

        // Look backward from i.
        int j=0;
        while(  (i-j > 0)
        &&        (fabs(y[1][i] - y[1][i-j]) < h) ) {
                double dist = fabs(y[1][i] - y[1][i-j]);
                weight = kernel(double(fabs(dist))/a);
                result[i] += y[0][i-j]*weight;
                totweight += weight;
                j++;
        }
        
        // Look forward from i.
        j=-1;
        while(  (i-j < n)
        &&        (fabs(y[1][i] - y[1][i-j]) < h) ) {
                double dist = fabs(y[1][i] - y[1][i-j]);
                weight = kernel(double(fabs(dist))/a);
                result[i] += y[0][i-j]*weight;
                totweight += weight;
                j--;
        }
        
        result[i] /= totweight;
    }

    return result;
} // smooth()
//------------------

double gaussian_kernel( double x ) {
    return exp(-x*x);
}
double box_kernel( double x ) {
    return x;
}

