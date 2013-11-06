#include <iostream>
using std::cout;

template<int N, class OP> struct NestedLoop
{
  void operator()( int * index, int lo, int hi, const OP& op ) const
  {
      for( index[N-1]=lo; index[N-1]<hi; ++index[N-1] )
      {
          NestedLoop<N-1,OP>()( index, lo, hi, op );
      }
  }
};


template<class OP> struct NestedLoop<0,OP>
{
    void operator()( int * index, int lo, int hi, const OP& op ) const
    {
        op( index );
    }
};


template<int N> struct PrintTuple
{
    PrintTuple( char const * msg ) : m_msg(msg) { }

    void operator()( int const * vi ) const
    {
        cout << m_msg << " ( ";
        for( int i=0;i<N;++i ) cout << vi[i] << " ";
        cout << " )\n";
    }
    char const* m_msg;
};


int main()
{
    int vi2[2];
    int vi3[3];
    NestedLoop<2,PrintTuple<2> >()(vi2,0,2,PrintTuple<2>("two"));
    cout << '\n';
    NestedLoop<3,PrintTuple<3> >()(vi3,0,2,PrintTuple<3>("three"));
}
