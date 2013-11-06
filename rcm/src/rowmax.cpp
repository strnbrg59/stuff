// rowmax.cpp : returns column vector containing max element of
// each row.

#include "rcm.h"

Vd rowmax( const Md& x ) {

    int xr=x.rows(), xc=x.cols();
    Vd result( xr, vertical );

    for (int i=0;i<xr;i++) {
        double biggest = -3E100;
        for( int j=0;j<xc; j++ ) 
            if( x[i][j] > biggest ) biggest = x[i][j];
        result[i] = biggest;
  }

  return result;
}
