#include <math.h>
#include "rcm.h"

/** Correlation matrix of the columns of x. */
Md correlmat(const Md& x)
{
    int xc( x.cols() );

    Md vcv( varcovar(x) );
    Md result(xc,xc);

    for( int i=0;i<xc;i++ ) for( int j=0;j<xc;j++ )
    {
        if( (vcv[i][i] == 0) || (vcv[j][j] == 0) )
            result[i][j] = 0;
        else
            result[i][j] = vcv[i][j]/pow(vcv[i][i]*vcv[j][j],0.5);
    }

    return result;
}

