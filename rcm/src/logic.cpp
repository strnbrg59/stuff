#include "rcm.h"

// We work with ints 1&0 instead of booleans so that it'll be easier
// to obtain counts, i.e. by summing.

// operator< 
static int lessThan(double a, double b)
{
    return a<b;
}
M<int> operator<( const Md& mat1, const Md& mat2 )
{
    return apply( lessThan, mat1, mat2 );
}
V<int> operator<( const Vd& vect1, const Vd& vect2 )
{
    return apply( lessThan, Md(vect1), Md(vect2) );
}

// operator<= 
static int lessThanOrEqualTo(double a, double b)
{
    return a<=b;
}
M<int> operator<=( const Md& mat1, const Md& mat2 )
{
    return apply( lessThanOrEqualTo, mat1, mat2 );
}
V<int> operator<=( const Vd& vect1, const Vd& vect2 )
{
    return apply( lessThanOrEqualTo, Md(vect1), Md(vect2) );
}

// operator> 
static int greaterThan(double a, double b)
{
    return a>b;
}
M<int> operator>( const Md& mat1, const Md& mat2 )
{
    return apply( greaterThan, mat1, mat2 );
}
V<int> operator>( const Vd& vect1, const Vd& vect2 )
{
    return apply( greaterThan, Md(vect1), Md(vect2) );
}

// operator>= 
static int greaterThanOrEqualTo(double a, double b)
{
    return a>=b;
}
M<int> operator>=( const Md& mat1, const Md& mat2 )
{
    return apply( greaterThanOrEqualTo, mat1, mat2 );
}
V<int> operator>=( const Vd& vect1, const Vd& vect2 )
{
    return apply( greaterThanOrEqualTo, Md(vect1), Md(vect2) );
}

// operator== 
static int equals(double a, double b)
{
    return a==b;
}
M<int> operator==( const Md& mat1, const Md& mat2 )
{
    return apply( equals, mat1, mat2 );
}
V<int> operator==( const Vd& vect1, const Vd& vect2 )
{
    return apply( equals, Md(vect1), Md(vect2) );
}

// operator!= 
static int notEquals(double a, double b)
{
    return a!=b;
}
M<int> operator!=( const Md& mat1, const Md& mat2 )
{
    return apply( notEquals, mat1, mat2 );
}
V<int> operator!=( const Vd& vect1, const Vd& vect2 )
{
    return apply( notEquals, Md(vect1), Md(vect2) );
}

// operator! 
static int nott(int a)
{
    if( a==0 )
        return 1;
    else
        return 0;
}
M<int> operator!( const M<int>& mat )
{
    return apply( nott, mat );
}
V<int> operator!( const V<int>& vect )
{
    return apply( nott, M<int>(vect) );
}


// operator|| 
static int orr(int a, int b)
{
    return a || b;
}
M<int> operator||( const M<int>& mat1, const M<int>& mat2 )
{
    return apply( orr, mat1, mat2 );
}
V<int> operator||( const V<int>& vect1, const V<int>& vect2 )
{
    return apply( orr, M<int>(vect1), M<int>(vect2) );
}

// operator&& 
static int andd(int a, int b)
{
    return a && b;
}
M<int> operator&&( const M<int>& mat1, const M<int>& mat2 )
{
    return apply( andd, mat1, mat2 );
}
V<int> operator&&( const V<int>& vect1, const V<int>& vect2 )
{
    return apply( andd, M<int>(vect1), M<int>(vect2) );
}
