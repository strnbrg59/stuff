// Shuffle the order of the columns in x.
#include <stdlib.h>
#include "rcm.h"

Md shuffle( const Md& x ) 
{

    Vd randnums( x.cols(), horizontal );
    for( int j=0; j<x.cols(); j++ )
        randnums[j] = random();

    V<int> sort_indices( x.cols(), horizontal );
    sort_indices = indexx(randnums);

    Md result( x.rows(), x.cols() );
    for( int i=0; i<x.rows(); i++ )
    {
        for( int j=0; j<x.cols(); j++ )
        {
            result[i][j] = x[i][sort_indices[j]];
        }
    }

    return result;
}
