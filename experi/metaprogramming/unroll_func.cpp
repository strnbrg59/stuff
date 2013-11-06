//
// This accum() template is as fast as v[0]+v[1]+v[2]+v[3].  Yay!
// But that's only with gcc-4.0.1; with gcc-3.4.2 it's even a little
// slower than std::accumulate() (which is itself nine times slower than
// v[1]+v[1]+v[2]+v[3]).
//
// That's with the -O3 switch.  With -O2, my accum() template is twice
// as slow as v[1]+v[1]+v[2]+v[3].  With -O6 I get the same performance
// numbers as with -O3.
//
// Looping over 0...3 is as fast as v[1]+v[1]+v[2]+v[3] only if you compile
// with -funroll-loops, even -O6 apparently doesn't turn on -funroll-loops
// if you don't tell it to.
//

#include <iostream>
#include <numeric>

template<int N> long accum( const long* v )
{
    return v[N-1] + accum<N-1>(v);
}

template<> long accum<1>( const long* v )
{
    return v[0];
}

int main( int argc )
{
    long v[] = {1,2,3,4};
    long const n = 10000000L;
    long result = 0;
    if( argc==1 )
    {
        for( long i=0;i<n;++i ) result += std::accumulate(v,v+4,0L);
    } else
    if( argc==2 )
    {
        for( long i=0;i<n;++i ) result += accum<4>(v);
    } else
    if( argc==3 )
    {
        for( long i=0;i<n;++i ) result += v[0]+v[1]+v[2]+v[3];
    } else
    {
        for( long i=0;i<n;++i ) for( int j=0;j<4;++j ) result += v[j];
    }

    std::cout << result << '\n';
}
