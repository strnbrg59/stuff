#include "rcm.h"

Vd iota( int n )
{
    Vd result( n, horizontal );
    for( int i=0;i<n;i++ )
      result[i] = i;

    return result;
}
