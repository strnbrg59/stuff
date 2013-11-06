#include "rcm.h"

/** Sort on the 0th row. */
Md sort( const Md& x )
{
    int r = x.rows();
    int c = x.cols();
    Md result(r,c);

    V<int> indices( indexx(x[0]) );

    for( int i=0;i<r;i++ )
    {
        for( int j=0;j<c;j++ )
        {
            result[i][j] = x[i][indices[j]];
        }
    }

    return result;
}
