#include <math.h>
#include "rcm.h"

/** If we try to define round(Md,double) and round(Vd,double) as
 *  "apply( round(double,double),...)", the compiler gets confused.
*/
static double scalar_round( double x, double tol )
{
    return tol*floor(x/tol + 0.5);
}

double round(double x, double tol )
{
    return scalar_round(x,tol);
}

Md round( const Md& x, double tol )
{
    return apply( scalar_round, x, Md(tol) );
}

Vd round( const Vd& x, double tol )
{
    return apply( scalar_round, Md(x), Md(tol) );
}


