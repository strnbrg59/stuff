#ifndef INCLUDED_INTVECT_H
#define INCLUDED_INTVECT_H

#include "hdf5.h"
#include <iosfwd>
#include <vector>

struct Consts;

using std::ostream;
using std::istream;

#define THREE 3

/** An array of three ints or three reals of whatever type.  Used to specify
 *  index coordinates of box corners and of cells; the "ghost" attribute of
 *  Chombo HDF5 files; the coordinate origin used in particle rendering.
 *  In 2D, clients should just ignore element 2.
 *
 *  Don't put any virtual methods in this class, as we need to use the
 *  equivalent of offsetof on it.
*/
template<typename T> struct Triple
{
    T m_data[THREE];

    Triple() { for( int i=0;i<THREE;++i ) m_data[i] = 0; }
    Triple( Triple<T> const & );
    Triple( Triple<T> const & iComp,
            Triple<T> const & jComp, Triple<T> const & kComp );
    explicit Triple( T const * data );
    Triple( T i, T j, T k=0 );
    operator std::vector<T>();

    Triple<T> & operator=( Triple<T> const & );
    void operator-=( Triple<T> const & );
    void operator+=( Triple<T> const & );
    void operator*=( T x );
    void MultiplyAndRoundDown( double x );

    T i() const { return m_data[0]; }
    T j() const { return m_data[1]; }
    T k() const { return m_data[2]; }
    T operator[]( int i ) const { return m_data[i]; }
    T & operator[]( int i ) { return m_data[i]; }
    bool operator==( Triple<T> const & that ) const;
    bool operator!=( Triple<T> const & that ) const;

    static void InitializeHDF5Datatype( int length );
    static void ReleaseHDF5Datatype();
    static hid_t s_HDF5Datatype;
    static Triple<T> Imax( Triple<T> const & iv1, Triple<T> const & iv2 );
    static Triple<T> Imin( Triple<T> const & iv1, Triple<T> const & iv2 );
    static Triple<T> Jmax( Triple<T> const & iv1, Triple<T> const & iv2 );
    static Triple<T> Jmin( Triple<T> const & iv1, Triple<T> const & iv2 );
    static Triple<T> WackyTriple();
};

typedef Triple<int> Intvect;

template<typename T> Triple<T>
    operator-( Triple<T> const & iv1, Triple<T> const & iv2 );
template<typename T> Triple<T>
    operator+( Triple<T> const & iv1, Triple<T> const & iv2 );
template<typename T> Triple<T> operator*( Triple<T> const & iv1, T x );
template<typename T> Triple<T> MultiplyAndRoundDown( Triple<T> const & iv1,
                                                     double x );


template<typename T, typename OSTREAM_T> OSTREAM_T &
operator<<( OSTREAM_T & ost, Triple<T> const & intvect )
{
    ost << "( " << intvect.m_data[0] << ", " << intvect.m_data[1]
        << ", " << intvect.m_data[2] << " )";
    return ost;
}

template<typename T> istream &
operator>>( istream & ist, Triple<T> & intvect )
{
    ist >> intvect.m_data[0] >> intvect.m_data[1] >> intvect.m_data[2];
    return ist;
}


#endif // INCLUDED_INTVECT_H
