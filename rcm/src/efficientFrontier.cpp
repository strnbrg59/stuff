#include "rcm.h"

/** Efficient frontier in p dimensions.
 *  Arg points is nxp -- n p-dimensional points.  
 *  Arg orthant is px1, and indicates how we define "efficient".
 *    The p-th coordinate of orthant is +1 or -1, according to
 *    whether the p-th coordinate axis is positive or negative in that
 *    orthant.
 *
 *  We do it by brute force -- the obvious pn^2 algorithm.
 *
 *  Returns a the subset of points that is efficient.
*/
Md efficientFrontier( const Md& points, const V<int>& orthant )
{
    int n = points.rows();
    int p = points.cols();
    if( p != orthant.size() )
    {
        rcmError( "efficientFrontier: points.cols() != orthant.size()!" );
    }

    // isEfficient is all true initially.  Elements become false when we
    // can prove they are dominated.
    V<bool> isEfficient(n,horizontal); 
    for( int i=0;i<n;i++ ) isEfficient[i] = true;
    int numEfficient=n; // will decrement as we find dominated points.
                                
    // For each point...
    for( int i=0;i<n;i++ )
    {
        // ...look at every other point...
        for( int j=0;j<n;j++ )
        {
            // ...and see if the j-th point dominates the i-th point.
            bool j_dominates_i = true;
            for( int k=0;k<p;k++ )
            {
                j_dominates_i =  j_dominates_i
                              && (orthant[k]*(points[i][k]-points[j][k])<0);
            }
            isEfficient[i] = isEfficient[i] && !j_dominates_i;
            if( isEfficient[i] == false )
            {
                numEfficient--;
                break;
            }
        }
    }

    // Put together the result: copy into result all efficient elements 
    // of arg points.
    Md result( numEfficient, p );
    int m=0; // indexes elements of result
    for( int i=0;i<n;i++ )
    {
        if( isEfficient[i] )
        {
            for( int k=0;k<p;k++ )
            {
                result[m][k] = points[i][k];
            }
            m++;
        }
    }
    return result;
}
