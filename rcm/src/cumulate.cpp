#include "rcm.h"

static double plus_func(double a, double b ) { return a+b; }
static double times_func(double a, double b ) { return a*b; }

/** Cumulative sum (uses laminate template function). */
Md lamPlus( const Md& mat )
{
  return laminate( plus_func, mat );
}

/** Cumulative product (uses laminate template function). */
Md lamTimes( const Md& mat )
{
  return laminate( times_func, mat );
}

Vd sum( const Md& mat )
{
    return scan( plus_func, mat );
}

Vd mean( const Md& mat )
{
    return sum(mat)/mat.cols();
}
