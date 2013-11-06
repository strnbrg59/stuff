//
// Here the metaprogram is as fast as the hand-coded version if we compile
// with gcc-4.0.1 and -O1 or better.  With gcc-3.4.2, the hand-coded version
// is a little faster.
//

#include <iostream>
#include <numeric>

template<int N> struct IntVect
{
    IntVect( int const * v )
    {
        for( int i=0;i<N;++i ) rep[i] = v[i];
    }
    int rep[N];
};

template<int N> inline bool opEquals( const int* v1,
                                      const int* v2 )
{
    return (v1[N-1] == v2[N-1]) && opEquals<N-1>(v1,v2);
}

template<> inline bool opEquals<1>( const int* v1,
                                    const int* v2 )
{
    return v1[0] == v2[0];
}

// We need this indirection because if we try something like
// template<int N, int M> bool operator==(IntVect<M> const&, IntVect<M> const&)
// because the operator==<1> specialization has to be a partial specialization
// (M still has to be M) and you can't do partial specialization of template
// functions.
//
template<int M> inline bool operator==( IntVect<M> const & iv1,
                                        IntVect<M> const & iv2 )
{
    return opEquals<M>(iv1.rep, iv2.rep);
}

#define DIM 3

int main( int argc )
{
    int buf1[] = {1,2,3,4}, buf2[] = {1,2,3,5};
    IntVect<DIM> v1(buf1), v2(buf2);
    long const n = 100000000L;
    long result = 0;
    if( argc==1 )
    {
        for( long i=0;i<n;++i ) result += std::equal(v1.rep,v1.rep+DIM,v2.rep);
    } else
    if( argc==2 )
    {
        for( long i=0;i<n;++i ) result += operator==<DIM>(v1,v2);
    } else
    if( argc==3 )
    {
        for( long i=0;i<n;++i ) result +=
           (v1.rep[0]==v2.rep[0])
        && (v1.rep[1]==v2.rep[1])
        && (v1.rep[2]==v2.rep[2]);
    } else
    {
        for( long i=0;i<n;++i ) for( int j=0;j<DIM;++j )
        {
            result += (v1.rep[j]==v2.rep[j]);
        }
    }

    std::cout << result << '\n';
}
