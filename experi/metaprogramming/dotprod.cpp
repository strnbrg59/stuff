//
// Compare this to unroll_func.cpp.  Here again, the template metaprogramming
// approach is a big win with gcc-4.0.1, but not with gcc-3.4.2.
// The surprise here is that even -funroll-loops doesn't make the looping
// approach (argc==3) competitive.
//
// The bigger surprise really is that it's not a performance hit to
// pass those arguments to dotprod().
//
// Flash: the dotprod template is as fast as handcoding even with gcc-3.4.2;
// all it takes is the "inline" keyword!
//

#include <iostream>
#include <numeric>

template<int N> inline long dotprod( const long* v1, const long* v2 )
{
    return v1[N-1]*v2[N-1] + dotprod<N-1>(v1,v2);
}

template<> inline long dotprod<1>( const long* v1, const long* v2 )
{
    return v1[0]*v2[0];
}

int main( int argc )
{
    long v1[] = {1,2,3,4}, v2[] = {10,20,30,40};
    long const n = 100000000L;
    long result = 0;
    if( argc==1 )
    {
        for( long i=0;i<n;++i ) result += std::inner_product(v1,v1+4,v2,0L);
    } else
    if( argc==2 )
    {
        for( long i=0;i<n;++i ) result += dotprod<4>(v1,v2);
    } else
    if( argc==3 )
    {
        for( long i=0;i<n;++i ) result += v1[0]*v2[0]
                                       +  v1[1]*v2[1]
                                       +  v1[2]*v2[2]
                                       +  v1[3]*v2[3];
    } else
    {
        for( long i=0;i<n;++i ) for( int j=0;j<4;++j ) result += v1[j]*v2[j];
    }

    std::cout << result << '\n';
}
