#ifndef _INDEXTM_H_
#define _INDEXTM_H_

//
// This code supports the chombotomization-of-Tempest project.
//
// What we have here (and in IndexTMI.H) specifically is a templatized
// unification of IntVect and RealVect.
//
// Author: Ted (modified Chombo version)
//


#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "GenericArithmetic.hpp"

template<typename T, int N> class IndexTM;
template<typename T, int N> IndexTM<T,N> min(const IndexTM<T,N>& p1,
                                             const IndexTM<T,N>& p2);
template<typename T, int N> IndexTM<T,N> max(const IndexTM<T,N>& p1,
                                             const IndexTM<T,N>& p2);
template<typename T, int N> IndexTM<T,N> reflect(const IndexTM<T,N>& a,
                                                 T                   ref_ix,
                                                 int                 idir);
template<typename T, int N> std::ostream& operator<<(std::ostream&       os,
                                                     const IndexTM<T,N>& iv);

///
/**
   Returns a basis vector in the given coordinate direction.<br>
   In 3-D:
   BASISV(0) == (1,0,0); BASISV(1) == (0,1,0); BASISV(2) == (0,0,1).<br>
   In 2-D:
   BASISV(0) == (1,0); BASISV(1) == (0,1).<br>
   Note that the coordinate directions are based at zero.
*/
template<typename T, int N> IndexTM<T,N> BASISV_TM(int dir);

template<typename T, int N> class IndexTM
  : public GenericArithmeticable< T,IndexTM<T,N> >
{
public:
  IndexTM() : GenericArithmeticable< T, IndexTM<T,N> >( this )
  {}

  ~IndexTM()
  {}

  IndexTM( T i, T j );
  IndexTM( T i, T j, T k );
  explicit IndexTM(const T* a);
  IndexTM(const IndexTM& rhs);

//operator IndexTM<Real,N>();

  IndexTM& operator=(const IndexTM& rhs);

  T& operator[](int i);
  T operator[](int i) const;
  T* operator&();

  bool operator==(const IndexTM& p) const;
  bool operator!= (const IndexTM& p) const;
  bool lexLT (const IndexTM& s) const;
  bool lexGT (const IndexTM& s) const;

  T innerProduct( const IndexTM& a_rhs ) const;
  static void normal( IndexTM const& v1, IndexTM const& v2,
                      IndexTM& result );

  // Scan functions:
  T sum() const;
  T product() const;


  /** Used by GenericArithmeticable to implement all pointwise arithmetic
   *  and comparison functions.
  */
  template<typename OP> bool operatorCompare(const IndexTM<T,N>&, const OP&)
    const;
  template<typename OP> IndexTM<T,N>& operatorOpEquals(
    const IndexTM<T,N>&, const OP&);
  template<typename OP> IndexTM<T,N>& operatorOpEquals(const T&, const OP&);

  IndexTM& reciprocal();

  /**
     Modifies this IndexTM by taking component-wise min with IndexTM
     argument.
  */
  IndexTM& min(const IndexTM& p);

  ///
  /**
     Modifies this IndexTM by taking component-wise max with IndexTM
     argument.
  */
  IndexTM& max(const IndexTM& p);

  ///
  /**
     Modifies IndexTM by reflecting it in the plane defined by the
     index <i>ref_ix</i> and with normal in the direction of <i>idir</i>.
     Directions are based at zero.
  */
  IndexTM& reflect(T ref_ix, int idir);

  void print(std::ostream&) const; // Used by global operator<<

  /**
     This is an IndexTM all of whose components are equal to zero.
  */
  static const IndexTM Zero;

  /**
     This is an IndexTM all of whose components are equal to one.
  */
  static const IndexTM Unit;

  /**
     Initializes Zero and Unit.
  */
  static int InitStatics();


private:
  T m_rep[N];
};


/**
    Useful for preventing division by IntVects, e.g.
        STATIC_ASSERT( ! IndexTraits<T>::isInt );
*/
template<typename T> struct IndexTraits
{
    static bool const isInt=false;
    static bool const isReal=false;
};
template<> struct IndexTraits<int>
{
    static bool const isInt=true;
    static bool const isReal=false;
};    
template<> struct IndexTraits<double>
{
    static bool const isInt=false;
    static bool const isReal=true;
};    


typedef IndexTM<double,3> RealVect3;
typedef IndexTM<int,3>    IntVect3;

//
// Static initialization.  Gotta make sure there are no static object
// definitions above here (except possibly stuff in headers).  Otherwise,
// the danger is that some static object's constructor calls IndexTM::Zero or
// IndexTM::Unit -- the very things the following definition is supposed to
// initialize.
//
static int s_dummyForIntVectH2( IndexTM<int,2>::InitStatics() );
static int s_dummyForIntVectH3( IndexTM<int,3>::InitStatics() );
static int s_dummyForRealVectH2( IndexTM<double,2>::InitStatics() );
static int s_dummyForRealVectH3( IndexTM<double,3>::InitStatics() );

#include "IndexTMI.hpp"

#endif // include guard
