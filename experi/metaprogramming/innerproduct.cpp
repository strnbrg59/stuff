#include <iostream>
#include <numeric>

template<int N, typename T, typename PlusT, typename TimesT>
struct InnerProduct
{
    int operator()( T const* v1, T const* v2 )
    {
        return PlusT()( TimesT()(v1[N-1],v2[N-1]),
                        InnerProduct<N-1,T,PlusT,TimesT>()(v1,v2) );
    }
};

template<typename PlusT, typename T, typename TimesT>
struct InnerProduct<1,T,PlusT,TimesT>
{
    T operator()( T const* v1, T const* v2 )
    {
        return TimesT()( v1[0], v2[0] );
    }
};

int main( int argc )
{
    long v1[] = {1,2,3,4};
    long v2[] = {1,2,1,2};
    long const n = 10000000L;
    long result = 0;
    if( argc==1 )
    {
        for( long i=0;i<n;++i )
        {
            result += std::inner_product(v1,v1+4,v2,0L);
        }
    } else
    if( argc==2 )
    {
        for( long i=0;i<n;++i )
        {
            result += InnerProduct<4,
                                   long,
                                   std::plus<long>,
                                   std::multiplies<long> >()(v1,v2);
        }
    } else
    if( argc==3 )
    {
        for( long i=0;i<n;++i )
        {
            result += v1[0]*v2[0] + v1[1]*v2[1]
                   +  v1[2]*v2[2] + v1[3]*v2[3];
        }
    } else
    {
        for( long i=0;i<n;++i ) for( int j=0;j<4;++j )
        {
            result += v1[j]*v2[j];
        }
    }

    std::cout << result << '\n';
}
