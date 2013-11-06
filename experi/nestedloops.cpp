//
// Speed comparison between two alternative ways to implement a
// nested loop -- template metaprogramming vs conventional runtime.
//
// At -O0 (gcc 3.4.2), the metaprogram is only a little faster.
// At -O2, the metaprogram is about 50% faster.
//

#include <iostream>

//-------------------------------------------------------------------
template<int N, class OP> struct NestedLoop
{
  void operator()( int * index, int lo, int hi, OP& op ) const
  {
      for( index[N-1]=lo; index[N-1]<hi; ++index[N-1] )
      {
          NestedLoop<N-1,OP>()( index, lo, hi, op );
      }
  }
  void operator()( int * index, const int * lo, const int * hi, OP& op ) const
  {
      for( index[N-1]=lo[N-1]; index[N-1]<hi[N-1]; ++index[N-1] )
      {
          NestedLoop<N-1,OP>()( index, lo, hi, op );
      }
  }
};


template<class OP> struct NestedLoop<0,OP>
{
    void operator()( int * index, int lo, int hi, OP& op ) const
    {
        op( index );
    }
    void operator()( int * index, const int * lo, const int * hi, OP& op ) const
    {
        op( index );
    }
};
//-------------------------------------------------------------------


using std::cout;

#define N 3

//
// This is templatized on OP but isn't a metaprogram; the essential feature
// is that the depth of the recursion -- n -- is only known at runtime.
//
template<class OP> inline void nestedLoop( int ndx[N], int n, int lo, int hi, OP& op )
{
    if( n==0 )
    {
        op( ndx );
    } else
    {
        for( ndx[n-1]=lo; ndx[n-1]<hi; ++ndx[n-1] )
        {
            nestedLoop( ndx, n-1, lo, hi, op );
        }
    }
}

int g_dummy;

struct Functor
{
    void operator()( int ndx[N] )
    {
        g_dummy = ndx[0];
    }
};


int main( int argc )
{
    const int HI = 1000;
    Functor functor;
    int ndx[N];
    if( argc == 1 )
    {
        cout << "Runtime loop:\n";
        nestedLoop( ndx, 3, 0, HI, functor );
    } else
    {
        cout << "Metatemplate loop:\n";
        NestedLoop<3,Functor>()( ndx, 0, HI, functor );
    }
}
