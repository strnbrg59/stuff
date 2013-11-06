#include <math.h>
#include "rcm.h"

// FIXME: Why can't we pass the library functions exp, log etc to
// apply()?  Why do we need to define our own wrappers like exp_func
// and log_func?

// exp
static double exp_func(double x) { return exp(x); }
Md exp( const Md& mat )
{
    return apply( exp_func, mat );
}

// log
static double log_func(double x) { return log(x); }
Md log( const Md& mat )
{
    return apply( log_func, mat );
}

// fabs
static double fabs_func(double x) { return fabs(x); }
Md fabs( const Md& mat )
{
    return apply( fabs_func, mat );
}

// sin
static double sin_func(double x) { return sin(x); }
Md sin( const Md& mat )
{
    return apply( sin_func, mat );
}

// cos
static double cos_func(double x) { return cos(x); }
Md cos( const Md& mat )
{
    return apply( cos_func, mat );
}

// tan
static double tan_func(double x) { return tan(x); }
Md tan( const Md& mat )
{
    return apply( tan_func, mat );
}

// asin
static double asin_func(double x) { return asin(x); }
Md asin( const Md& mat )
{
    return apply( asin_func, mat );
}

// acos
static double acos_func(double x) { return acos(x); }
Md acos( const Md& mat )
{
    return apply( acos_func, mat );
}

// atan
static double atan_func(double x) { return atan(x); }
Md atan( const Md& mat )
{
    return apply( atan_func, mat );
}

// sinh
static double sinh_func(double x) { return sinh(x); }
Md sinh( const Md& mat )
{
    return apply( sinh_func, mat );
}

// cosh
static double cosh_func(double x) { return cosh(x); }
Md cosh( const Md& mat )
{
    return apply( cosh_func, mat );
}

// tanh
static double tanh_func(double x) { return tanh(x); }
Md tanh( const Md& mat )
{
    return apply( tanh_func, mat );
}

// asinh
static double asinh_func(double x) { return asinh(x); }
Md asinh( const Md& mat )
{
    return apply( asinh_func, mat );
}

// acosh
static double acosh_func(double x) { return acosh(x); }
Md acosh( const Md& mat )
{
    return apply( acosh_func, mat );
}

// atanh
static double atanh_func(double x) { return atanh(x); }
Md atanh( const Md& mat )
{
    return apply( atanh_func, mat );
}

// pow
static double pow_func(double a, double b)
{
  return pow(a,b);
}
Md pow( const Md& a, const Md& b )
{
  return apply( pow_func, a, b );
}

/** N(0,1) cdf,
 *  Adapted from Numerical Recipes' erfcc() function.
*/
double gaussian01( double x )
{
    double t,z,ans; 

    z=fabs(x/pow(2.0,0.5)); 
    t=1.0/(1.0+0.5*z); 
    ans=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+ 
        t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+ 
        t*(-0.82215223+t*0.17087277))))))))); 

    return x>=0.0 ? 1-ans/2 : ans/2;

}
