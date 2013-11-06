#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "rcm.h"

static double urand_func( double x )
{
#if RCM_DEBUG==1
    if( int(x) != int(x+0.00000001) )
        rcmError( "urand: argument should be an integer, or a double just a hair above." );
    if( fabs(x) < 0.01 )
        rcmError( "urand(0) : argument should be at least 1." );
#endif

    int n = int( x+0.00000001 );  
    return rand()%n;
}     

/** Uniform random integers on [0,floor(mat)] */
Md urand( const Md& mat )
{
    return apply( urand_func, mat );
}

/** Uniform random floats on [0,1], returned as the elements of a
  * matrix of designated size.
*/
Md urand( int r, int c )
{
    Md result(r,c);

    for( int i=0;i<r;i++ )
    {
        for( int j=0;j<c;j++ )
        {
            result[i][j] = rand()/double(RAND_MAX);
        }
    }

    return result;
}
