#include "rcm.h"

/** first differences across columns */
Md diffs( const Md& x) { 

    int xr=x.rows(), xc=x.cols();
    int max1xc;

    if( xc>1 ) max1xc=xc-1;
    else max1xc = 1;

    Md result( xr, max1xc );

    for (int i=0;i<xr;i++) 
    {
        if( xc==1 ) result[i][0] = 0;
        else for (int j=1;j<xc;j++) result[i][j-1] = x[i][j] - x[i][j-1];
    }

  return result;
}
