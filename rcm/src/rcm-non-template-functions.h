#ifndef _INCLUDED_RCM_NON_TEMPLATE_FUNCTIONS_H_
#define _INCLUDED_RCM_NON_TEMPLATE_FUNCTIONS_H_

#include <string>

Md inverse( const Md& );

// Arithmetic operators
Md operator+( const Md&, const Md& );
Vd operator+( const Vd&, const Vd& );
Md operator-( const Md&, const Md& );
Vd operator-( const Vd&, const Vd& );
Md operator-( const Md& );
Vd operator-( const Vd& );
Md operator*( const Md&, const Md& );
Vd operator*( const Vd&, const Vd& );
Md operator/( const Md&, const Md& );
Vd operator/( const Vd&, const Vd& );

// Logic operators
M<int> operator!( const M<int>& );
V<int> operator!( const V<int>& );
M<int> operator!=( const Md&, const Md& );
V<int> operator!=( const Vd&, const Vd& );
M<int> operator&&( const M<int>&, const M<int>& );
V<int> operator&&( const V<int>&, const V<int>& );
M<int> operator||( const M<int>&, const M<int>& );
V<int> operator||( const V<int>&, const V<int>& );
M<int> operator<( const Md&, const Md& );
V<int> operator<( const Vd&, const Vd& );
M<int> operator<=( const Md&, const Md& );
V<int> operator<=( const Vd&, const Vd& );
M<int> operator==( const Md&, const Md& );
V<int> operator==( const Vd&, const Vd& );
M<int> operator>( const Md&, const Md& );
V<int> operator>( const Vd&, const Vd& );
M<int> operator>=( const Md&, const Md& );
V<int> operator>=( const Vd&, const Vd& );

Md fabs( const Md& );
double gaussian01( double x );

Md pow( const Md&, const Md& );
Md exp( const Md& );
Md log( const Md& );

Md sin( const Md& );
Md cos( const Md& );
Md tan( const Md& );
Md asin( const Md& );
Md acos( const Md& );
Md atan( const Md& );
Md sinh( const Md& );
Md cosh( const Md& );
Md tanh( const Md& );
Md asinh( const Md& );
Md acosh( const Md& );
Md atanh( const Md& );

// zero functions, for initializing things.  The argument is a dummy
// and is ignored, but it's necessary since you can't overload on return
// type alone.
int zero(int dummy);
double zero(double dummy);
std::string zero(std::string dummy);

Md matmult( const Md&, const Md& );
Md matmult_xpy( const Md&, const Md& );
Md matmult_xyp( const Md&, const Md& );
Vd iota( int n );

Md urand( const Md& );
Md urand( int r, int c );
Md nrand( int r, int c );

Md lamPlus( const Md& mat );
Md lamTimes( const Md& mat );
Vd sum( const Md& mat );
Vd mean( const Md& mat );
Vd sd( const Md& mat );
double covar( const Vd&, const Vd& );
double correl( const Vd&, const Vd& );

Md varcovar( const Md& );
Md correlmat( const Md& );
Md cgram( const Vd& x, const Vd& y, int maxlag );
Md eigen( const Md& x);

V<int> indexx( const Md& mat );
Md sort( const Md& );
Md shuffle( const Md& );
Md quantiles( const Md&, int n );
Md median( const Md& );

double round( double x, double tol );
Md round( const Md& x, double tol );
Vd round( const Vd& x, double tol );
Vd rowmax( const Md& x );
Md kernelDensity( const Vd& xx, double h );
Md diffs( const Md& x );
Md efficientFrontier( const Md& points, const V<int>& orthant );
//Md smooth( const Md y_, double h, double (*kernel)(double) );
Md smooth( const Md y_, double h, const char* kernelName = "gaussian" );

class Regression;
Regression* stepwiseRegression( const Vd& y_, const Md& candidates_, 
                                const Regression*, double stopoutRsqr );

#endif
