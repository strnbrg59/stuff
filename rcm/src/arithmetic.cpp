#include "rcm.h"

// operator+ 
static double rcmplus(double a, double b)
{
    return a+b;
}
Md operator+( const Md& mat1, const Md& mat2 )
{
    return apply( rcmplus, mat1, mat2 );
}
Vd operator+( const Vd& vect1, const Vd& vect2 )
{
    return apply( rcmplus, Md(vect1), Md(vect2) );
}

// operator* 
static double rcmtimes(double a, double b)
{
    return a*b;
}
Md operator*( const Md& mat1, const Md& mat2 )
{
    return apply( rcmtimes, mat1, mat2 );
}
Vd operator*( const Vd& vect1, const Vd& vect2 )
{
    return apply( rcmtimes, Md(vect1), Md(vect2) );
}

// diadic operator- 
static double diadicMinus(double a, double b)
{
    return a-b;
}
Md operator-( const Md& mat1, const Md& mat2 )
{
    return apply( diadicMinus, mat1, mat2 );
}
Vd operator-( const Vd& vect1, const Vd& vect2 )
{
    return apply( diadicMinus, Md(vect1), Md(vect2) );
}

// monadic operator- 
static double monadicMinus(double a)
{
    return -a;
}
Md operator-( const Md& mat )
{
    return apply( monadicMinus, mat );
}
Vd operator-( const Vd& vect )
{
    return apply( monadicMinus, Md(vect) );
}

// operator/ 
static double divide(double a, double b)
{
    return a/b;
}
Md operator/( const Md& mat1, const Md& mat2 )
{
    return apply( divide, mat1, mat2 );
}
Vd operator/( const Vd& vect1, const Vd& vect2 )
{
    return apply( divide, Md(vect1), Md(vect2) );
}
