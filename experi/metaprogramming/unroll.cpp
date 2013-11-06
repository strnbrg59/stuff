//
// This shows how (with a fair amount of contortion) you can get a template
// to perform as fast as pure hand-coding.
//
// Unfortunately, C++ doesn't allow a critical kind of template template
// function specialization (see below).
//
// Fortunately, with gcc-4.0.1 there's a much simpler alternative --
// see unroll_func.cpp.
//

#include <iostream>
using std::cout;

template<int N> struct Accum
{
    Accum( int const* v ): _v(v) { }
    int operator()() { return doit<N>(); }

    template<int M> int doit();

    int const* _v;
};

template<int N> template<int M> inline int Accum<N>::doit()
{
    return _v[M-1] + doit<M-1>();
}

// I can't get the compiler to accept 
// template<int N> template<> int Accum<N>::doit<1>, dammit!
template<> template<> inline int Accum<4>::doit<1>()
{
    return _v[0];
}

inline int handaccum( int const v[4] )
{
    return v[0] + v[1] + v[2] + v[3];
}

int main( int argc, char* argv[] )
{
    int vect[4] = {1,2,3,4};
    long result(0);

    Accum<4> accum( vect );

    if( argc == 1 )
    {
        for( long i=0; i<10000000L; ++i )
        {
            result += accum();
        }
    } else
    {
        for( long i=0; i<10000000L; ++i )
        {
            result += handaccum( vect );
        }
    }

    cout << result << '\n';
}
