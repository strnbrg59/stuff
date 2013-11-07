#ifndef INCLUDED_BOX_H
#define INCLUDED_BOX_H

#include <iosfwd>
#include <vector>
#include "hdf5.h"
#include "Intvect.h"
#include "../utils/StatusCodes.h"

class Box;

/** Six integers that define the extents of a FAB or of the entire domain.
 *
 *  Struct BoxSimple is meant to hold nothing but the data -- a really simple
 *  structure that the offsetof macro should have no trouble with.  We call
 *  H5Dread() on BoxSimple, and then construct the fancier class Box from that.
*/
struct BoxSimple
{
    BoxSimple() { for( int i=0;i<2*THREE;++i ) data[i] = 0; }
    BoxSimple( Box const & );
    mutable int data[ 2*THREE ]; // lo_i, lo_j, lo_k, hi_i, hi_j, hi_k.
};


class Box
{
  public:
    Box() { }
    explicit Box( BoxSimple const & simple, int idNum );
    Box( Box const & );
    Box( Box const &, char axis ); // slices
    explicit Box( int corners[6] );

    void Grow( Intvect const & iv, bool atBothEnds );
    void Shrink( Intvect const & iv, bool atBothEnds );

    void operator*=( double );
    void operator*=( double[THREE] );
    Box  operator*( double ) const;
    Box  operator*( double[THREE] ) const;
    void Scale( int r );

    bool CanCombineWith( Box const & that ) const;
    bool ContainsPlane( char axis, double axisPosition,
                        Triple<double> dx, Triple<double> origin ) const;

    Intvect GetLoCorner() const { return m_loCorner; }
    Intvect GetHiCorner() const { return m_hiCorner; }
    int GetLoCorner( int axis ) const { return m_loCorner.m_data[axis]; }
    int GetHiCorner( int axis ) const { return m_hiCorner.m_data[axis]; }
    int GetNumCells() const;
    void GetDims( int * result ) const;
    int GetIdNum() const { return m_idNum; }
    void SetIdNum( int b ) { m_idNum = b; }
    int GetRealIdNum() const { return m_realIdNum; }

    static void InitializeHDF5Datatype( int length );
    static void ReleaseHDF5Datatype();
    static hid_t s_HDF5Datatype;

    std::vector<Box> operator-( Box const & that ) const;
    Box Intersect( Box const & that, Status * status ) const;
    void operator+=( Box const & that );
    Box & operator=( Box const & that );
    bool operator==( Box const & that ) const;
    bool operator!=( Box const & that ) const;
    bool HasOddSide( int dimensionality ) const;
    bool ContainsCell( int cell[3] ) const;

    static Box WackyBox();

  private:
    Intvect m_loCorner;
    Intvect m_hiCorner;
    int m_idNum;
    int m_realIdNum;
};

template<typename OSTREAM_T> OSTREAM_T &
operator<<( OSTREAM_T & ost, Box const & box )
{
    ost << "[ " << box.GetLoCorner() << ", " << box.GetHiCorner() << " ]";
    return ost;
}



#endif //INCLUDED_BOX_H
