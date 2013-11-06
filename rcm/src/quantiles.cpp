#include <math.h>
#include "rcm.h"

/** Quantiles of 0-th row, with corresponding elements of all rows beneath.
 *  Convention: n=1 ==> return median.  n=2 ==> return 33% & 66%.  etc. 
 *  If mat has just one row, then we return interpolated quantiles, e.g.
 *  if mat has an even number of rows, quantiles(mat,1) will return the
 *  average of the two "middle" values.
*/
Md quantiles( const Md& mat, int n )
{
    int rows = mat.rows();
    int cols = mat.cols();
    Md result( rows, n );

#if RCM_DEBUG==1
    if( (n < 1) || (n>cols) ) 
        rcmError("quantiles: second arg must be > 1 and < number of columns.");
#endif

    Md smat( sort(mat) );
    for( int j=0;j<n;j++ )
    {
        double elem = (j+1)*(cols-1)/(n+1.0);

        if( rows > 1 )
        {
            for( int i=0;i<rows;i++ )
            {
                result[i][j] = smat[i][int(elem+0.5)];
            }
        }
        else
        {
            double ceil_ = ceil(elem-0.000001);
            double floor_ = floor(elem+0.000001);
            if( fabs(elem-floor_) < 0.000001)
            {
                result[0][j] = smat[0][int(floor_)];
            }
            else
            {    
                result[0][j] = smat[0][int(floor_)]*(ceil_-elem)
                             + smat[0][int(ceil_)]*(elem-floor_);
            }
        }
    }

    return result;
}

/** Median.  Same interpolation policy as quantiles(). */
Md median( const Md& mat )
{
    return quantiles(mat,1);
}
