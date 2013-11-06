//
// With lexicographic compare, the problem is that the easiest way to write
// the metafunction unfortunately does the comparison from the back end.
// In earlier efforts, I tried passing the "original N" as an extra function
// argument to lexLT_impl, and then I even tried saving "original N" in a
// global variable, and still performance was no good, because that original
// N wasn't known at compile time.
//
// But now I've got it: this version of lexLT is as fast as the hand-coded one
// (a little faster even), but only with gcc-4.0.1.  With gcc-3.4.2, the
// hand-coded one is three times faster.
//
// The std::lexicographical_compare is always three or more times slower than
// the hand-coded version.
//
// With icpc (the Intel compiler) and -O3 (the highest level it seems to
// accept) STL takes much less a performance hit; this program clocks 0.57s
// with lexicographical_compare, 0.40s with my lexLT<4>, and 0.30 with the
// handwritten code.
// But in dotprod.cpp, STL's performance is pretty bad -- about ten times
// slower than my metaprogram there.
//
// On Bassi (IBM AIX, xlC compiler), std::inner_product is twice as slow
// as the metaprogram and the handcoded version (which are equally fast).
// That's without optimization.  With -O3, the handcoded version is way faster
// than anything.  With -O4, xlC lets us know that inlining "failed due to the
// presence of a C++ exception handler" -- something related to the iostream
// library.  It goes away, though, if I remove the iostream stuff, and then
// (with -O5) the STL function is only about 25% slower than the metaprogram
// and the handcoded stuff (which are the same speed).  Actually, sometimes
// commenting out the iostream stuff doesn't do the trick and I get that
// can't-inline warning no matter what.
//

#include <cassert>
#include <numeric>
#include <iostream>

//---------------------------------------------------------------------

template<int N, typename T> struct LexLT
{
    bool operator()( const T* v1, const T* v2 )
    {
        return doit( v1+N, v2+N );
    }

    static bool doit( const T* v1, const T* v2 )
    {
        if      ( v1[-N] < v2[-N] ) return true;
        else if ( v1[-N] > v2[-N] ) return false;
        else                        return LexLT<N-1,T>::doit(v1,v2);
    }
};


template<typename T> struct LexLT<1,T>
{
    bool operator()( const T* v1, const T* v2 )
    {
        return doit(v1,v2);
    }
    static bool doit( const T* v1, const T* v2 )
    {
        if( v1[-1] < v2[-1] ) return true;
        else                  return false;
    }
};

//---------------------------------------------------------------------

// Traditional recursion -- still twice as fast as std::lexicographical_compare.
inline bool lexLT_untemplated( const int* v1, const int* v2, int N )
{
    if      ( v1[-N] < v2[-N] ) return true;
    else if ( v1[-N] > v2[-N] ) return false;
    else if ( N>0             ) return lexLT_untemplated(v1,v2,N-1);
    else if ( v1[0] < v2[0]   ) return true;
    else                        return false;
}


#define DIM 3

int main( int argc )
{
    int v1[] = {1,2,3}, v2[] = {1,3,1};
    int const n = 100000000L;
    int result = 0;
    if( argc==1 )
    {
        for( long i=0;i<n;++i ) result +=
            std::lexicographical_compare(v1,v1+DIM,v2,v2+DIM);
    } else
    if( argc==2 )
    {
        for( long i=0;i<n;++i ) result += LexLT<DIM,int>()(v1,v2);
    } else
    if( argc==3 )
    {
        for( long i=0;i<n;++i )
        {
            if      ( v1[0] < v2[0] ) result += 1;
            else if ( v1[0] > v2[0] ) result += 0;
            else if ( v1[1] < v2[1] ) result += 1;
            else if ( v1[1] > v2[1] ) result += 0;
            else if ( v1[2] < v2[2] ) result += 1;
            else                      result += 0;
        }
    } else
    if( argc==4 )
    {
        for( long i=0;i<n;++i ) result += lexLT_untemplated(v1,v2,DIM);
    } else
    {
        assert(0);
    }

    std::cout << result << '\n';
}
