#include <stdlib.h>
#include "rcm.h"

/** Matrix of standard normal deviates.
    Algorithm: sum 12 uniform[-0.5,+0.5] deviates.
    Arguments r and c are dimensions of the result matrix.
*/
Md nrand( int r, int c )
{
    Md result( r, c );
    for( int i=0;i<12;i++ )
    {
        result = result + urand(r,c);
    }

    return result - 6.0;
}
