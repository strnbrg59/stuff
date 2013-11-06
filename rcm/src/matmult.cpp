#include "rcm.h"
#include "rcm-template-functions.h"
#include "rcm-non-template-functions.h"

static double plus_func(double a, double b ) { return a+b; }
static double times_func(double a, double b ) { return a*b; }

Md matmult( const Md& mat1, const Md& mat2 )
{
  return innerProduct( plus_func, times_func, mat1, mat2 );
}

Md matmult_xpy( const Md& mat1, const Md& mat2 )
{
  return innerProduct_xpy( plus_func, times_func, mat1, mat2 );
}

Md matmult_xyp( const Md& mat1, const Md& mat2 )
{
  return innerProduct_xyp( plus_func, times_func, mat1, mat2 );
}

