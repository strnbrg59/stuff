//
// This is the solution to making a metaprogram that can be handed an
// arbitrary accumulation function.  You can't do it with template functions
// because you can't partially specialize them.  But you can partially
// specialize template classes.
//
// At -O6, with gcc-4.0.1, the metafunction here runs just as fast as the
// handcoded version (and both are nine times faster than std::accumulate).
//

#include <iostream>
#include <numeric>

template<int N, typename FunctorT> struct Accum
{
    int operator()( long const *v )
    {
        return FunctorT()( v[N-1], Accum<N-1,FunctorT>()(v) );
    }
};

template<typename FunctorT> struct Accum<1,FunctorT>
{
    int operator()( long const *v )
    {
        return v[0];
    }
};

int main( int argc )
{
    long v[] = {1,2,3,5};
    long const n = 100000000L;
    long result = 0;
    if( argc==1 )
    {
        for( long i=0;i<n;++i ) result += std::accumulate(v,v+4,0L);
    } else
    if( argc==2 )
    {
        for( long i=0;i<n;++i )
        {
            result += Accum<4, std::plus<int> >()(v);
        }
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
