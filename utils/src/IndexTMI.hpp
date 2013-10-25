#ifndef _INDEXTMI_H_
#define _INDEXTMI_H_

#include <iostream>
using std::ostream;
using std::istream;
using std::ws;

#include "IndexTM.hpp"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>
#include "Metaprograms.hpp"


template<typename T, int N> void
IndexTM<T,N>::print (ostream& os) const
{
    os << "[";
    for(int i=0;i<N-1;++i) os << m_rep[i] << ",";
    os << m_rep[N-1] << "]";
}

template<typename T, int N> ostream&
operator<<(ostream& os, IndexTM<T,N> const& v)
{
    v.print(os);
    return os;
}


//
// Static object initialization.
//
template<typename T, int N> int
IndexTM<T,N>::InitStatics()
{
  IndexTM<T,N>* pz = const_cast<IndexTM<T,N>*>( &IndexTM<T,N>::Zero );
  T zeros[] = {0,0,0,0,0,0,0,0,0,0};
  T ones[]  = {1,1,1,1,1,1,1,1,1,1};
  *pz = IndexTM<T,N>( zeros );

  IndexTM<T,N>* pu = const_cast<IndexTM<T,N>*>( &IndexTM<T,N>::Unit );
  *pu = IndexTM<T,N>( ones );

  // No danger of Index::Zero and Unit not having been allocated, as ARM section
  // 3.4 says "The initialization of nonlocal static objects in a translation
  // unit is done before the first use of any function...defined in that
  // translation unit."
  //
  // Had to go through the const_cast stuff because it's nice to be able to
  // declare Index::Zero and Index::Unit as const.

  return 0; // arbitrary
}


template<typename T, int N>
IndexTM<T,N>::IndexTM(const T *a)
  : GenericArithmeticable< T, IndexTM<T,N> >( this )
{
  memcpy( m_rep, a, N*sizeof(T) );
}

template<typename T, int N>
IndexTM<T,N>::IndexTM(const IndexTM<T,N> &iv)
  : GenericArithmeticable< T, IndexTM<T,N> >( this )
{
  memcpy(m_rep, iv.m_rep, N*sizeof(T) );
}

template<typename T, int N>
IndexTM<T,N>&
IndexTM<T,N>::operator=(const IndexTM<T,N> &iv)
{
  memcpy( m_rep, iv.m_rep, N*sizeof(T) );
  return *this;
}

template<typename T, int N>
T&
IndexTM<T,N>::operator[] (int i)
{
  assert(i>=0 && i < N);
  return m_rep[i];
}

template<typename T, int N>
T
IndexTM<T,N>::operator[] (int i) const
{
  assert(i>=0 && i < N);
  return m_rep[i];
}

template<typename T, int N> T*
IndexTM<T,N>::operator&()
{
  return m_rep;
}


template<typename T, int N>
bool
IndexTM<T,N>::operator==(const IndexTM& p) const
{
  return Metaprograms::pointwiseCompare<N,T,std::equal_to<T> >(m_rep,p.m_rep);
}

template<typename T, int N>
bool
IndexTM<T,N>::operator!=(const IndexTM& p) const
{
  return ! operator==(p);
}

template<typename T, int N>
bool
IndexTM<T,N>::lexLT (const IndexTM &s) const
{
  return Metaprograms::LexLT<N,T>()(m_rep,s.m_rep);
}

template<typename T, int N>
bool
IndexTM<T,N>::lexGT (const IndexTM& s) const
{
  return ! Metaprograms::LexLT<N,T>()(m_rep,s.m_rep);
}


template<typename T, int N>
T
IndexTM<T,N>::innerProduct( const IndexTM<T,N>& a_rhs ) const
{
  return Metaprograms::InnerProduct<N,T,T,
                                    std::plus<T>,
                                    std::multiplies<T> >()( m_rep,
                                                            a_rhs.m_rep );
}


/*static*/ template<typename T, int N> void
IndexTM<T,N>::normal(IndexTM<T,N> const& v1,
                     IndexTM<T,N> const& v2,
                     IndexTM<T,N>& result)
{
  STATIC_ASSERT(N==3);
  result[0] = v1[1]*v2[2] - v1[2]*v2[1];
  result[1] = v1[2]*v2[0] - v1[0]*v2[2];
  result[2] = v1[0]*v2[1] - v1[1]*v2[0];
  double norm = pow(result.innerProduct(result), 0.5);
  result /= norm;
}

template<typename T, int N>
T
IndexTM<T,N>::sum () const
{
  return Metaprograms::Accum<N,T,std::plus<T> >()( m_rep );
}

template<typename T, int N>
T
IndexTM<T,N>::product () const
{
  return Metaprograms::Accum<N,T,std::multiplies<T> >()( m_rep );
}

template<typename T, int N> template<typename OP> bool
IndexTM<T,N>::operatorCompare(const IndexTM<T,N>& p, const OP& op) const
{
  return Metaprograms::pointwiseCompare<N,T,OP>(m_rep,p.m_rep);
}


template<typename T, int N> template<typename OP>
IndexTM<T,N>&
IndexTM<T,N>::operatorOpEquals(const T& s, const OP& op)
{
  Metaprograms::Transform<N,T,OP>()(m_rep,s);
  return *this;
}

template<typename T, int N> template<typename OP>
IndexTM<T,N>&
IndexTM<T,N>::operatorOpEquals(const IndexTM<T,N>& p, const OP& op)
{
  Metaprograms::Transform<N,T,OP>()( m_rep, p.m_rep );
  return *this;
}

template<typename T, int N>
IndexTM<T,N>&
IndexTM<T,N>::reciprocal()
{
  std::transform( m_rep, m_rep+N, m_rep, bind1st(std::divides<T>(),T(1)) );
  return *this;
}

template<typename T> static T ourmin(T a, T b) { return a<b ? a:b; }
template<typename T> static T ourmax(T a, T b) { return a>b ? a:b; }

template<typename T, int N>
IndexTM<T,N>&
IndexTM<T,N>::min (const IndexTM<T,N>& p)
{
  std::transform( m_rep, m_rep+N, p.m_rep, m_rep,
                  std::ptr_fun(ourmin<T>) );
  return *this;
}

template<typename T, int N> IndexTM<T,N>&
IndexTM<T,N>::max (const IndexTM<T,N>& p)
{
  std::transform( m_rep, m_rep+N, p.m_rep, m_rep,
                  std::ptr_fun(ourmax<T>) );
  return *this;
}


template<typename T, int N> IndexTM<T,N>&
IndexTM<T,N>::reflect (T ref_ix,
                       int idir)
{
  assert(idir >= 0 && idir < N);
  m_rep[idir] = -m_rep[idir] + 2*ref_ix;
  return *this;
}


template<typename T, int N> IndexTM<T,N>
min(const IndexTM<T,N>& p1,
    const IndexTM<T,N>& p2)
{
  IndexTM<T,N> result( p1 );
  return result.min(p2);
}

template<typename T, int N> IndexTM<T,N>
max(const IndexTM<T,N>& p1,
    const IndexTM<T,N>& p2)
{
  IndexTM<T,N> result( p1 );
  return result.max(p2);
}

template<typename T, int N> IndexTM<T,N>
BASISV_TM(int dir)
{
  assert(dir >= 0 && dir < N);
  IndexTM<T,N> tmp = IndexTM<T,N>::Zero ;
  tmp.dataPtr()[dir] = T(1);
  return tmp;
}

template<typename T, int N> IndexTM<T,N>
reflect(const IndexTM<T,N>& a,
        T            ref_ix,
        int            idir)
{
  IndexTM<T,N> result( a );
  return result.reflect( ref_ix, idir );
}


/*
template<typename T, int N>
IndexTM<T,N>::operator IndexTM<double,N>()
{
  IndexTM<double,N> result;
  for( int i=0;i<N;++i ) result.dataPtr()[i] = Real(m_rep[i]);
  return result;
}
*/


template<typename T, int N>
IndexTM<T,N>::IndexTM( T i, T j )
  : GenericArithmeticable< T, IndexTM<T,N> >( this )
{
  STATIC_ASSERT( N==2 );
  m_rep[0] = i;
  m_rep[1] = j;
}
template<typename T, int N>
IndexTM<T,N>::IndexTM( T i, T j, T k )
  : GenericArithmeticable< T, IndexTM<T,N> >( this )
{
  STATIC_ASSERT( N==3 );
  m_rep[0] = i;
  m_rep[1] = j;
  m_rep[2] = k;
}

template<typename T, int N> const IndexTM<T,N> IndexTM<T,N>::Zero;
template<typename T, int N> const IndexTM<T,N> IndexTM<T,N>::Unit;
static int s_dummyForIntVectCpp2( IndexTM<int,2>::InitStatics() );
static int s_dummyForIntVectCpp3( IndexTM<int,3>::InitStatics() );
static int s_dummyForRealVectCpp2( IndexTM<double,2>::InitStatics() );
static int s_dummyForRealVectCpp3( IndexTM<double,3>::InitStatics() );
// If Index::Zero and Index::Unit were pointers, we wouldn't need this extra
// static int.  But they're objects, and the danger is that the initializations
// right above here ("Index Index::Zero;" and "Index Index::Unit;") are hit
// after the last call to Index::InitStatics, and in that case the
// Index::Index() constructor could redefine Zero and Unit.  In fact, the way
// things stand now, nothing bad would happen, because the Index::Index()
// constructor doesn't assign anything to any of the data members.  But we don't
// want to count on that always being the case.

#endif // include guard
