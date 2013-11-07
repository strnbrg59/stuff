/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/
// Author: Ted Sternberg

#ifndef INCLUDED_FAB_H
#define INCLUDED_FAB_H

#include "Intvect.h"
#include "Box.h"
#include "ArgForDummyTemplateInstantiation.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_array.hpp>
#include <map>
#include <cmath> // for HUGE_NUM
using std::pair;


/** Refcount management on behalf of boost::intrusive_ptr.
 *  Explicit instantiation of these doesn't work on MacOSX; linker complains
 *  about undefined symbols (nm shows symbols are in the library, but they're
 *  somehow "local"; they're of type 's'.
*/
template<typename T> void
intrusive_ptr_add_ref( T * fab )
{
    fab->IncrRefcount( +1 );
}

template<typename T> void
intrusive_ptr_release( T * fab )
{
    assert( fab );
    assert( fab->m_refcount > 0 );

    fab->IncrRefcount( -1 );

    if( fab->m_refcount == 0 )
    {
        delete fab;
    }
}


/** Holds, in Fortran-order (hence the name), the Chombo data for one
 *  particular level, box and component.  Also holds a pointer to the associated
 *  Box.
 *
 *  Memory management:
 *  In ChomboVis, multiple ChomboReaders hold pointers to a common
 *  instance of VisualizableDataset, on which they call GetFAB().
*/
template<class REAL_T> class FAB
{
  public:
    struct CtorArgs
    {
        Box box;
        boost::shared_array<REAL_T> fieldData;
        Triple<REAL_T> dx;
        Triple<REAL_T> origin;
        int level;
        int component;
        bool real;
        bool padded;
        Intvect ghost;

        CtorArgs();
        bool AllSet() const;
    };

    FAB( CtorArgs const & );
    FAB( ArgForDummyTemplateInstantiation );
    FAB( boost::intrusive_ptr< FAB<REAL_T> >, char axis,REAL_T axisPositionXYZ);
    void DeepCopy( boost::intrusive_ptr< FAB<REAL_T> > );
         
    ~FAB();

    int  GetRefcount() const { return m_refcount; }

    void CopyData( boost::shared_array<REAL_T> data );
    bool IsReal() const { return m_real; }
    bool IsPadded() const { return m_padded; }
    Intvect GetGhost() const { return m_ghost; }

    REAL_T   GetArrayItem( int const coords[] ) const; // arg is THREE long.
    REAL_T & GetArrayItemRef( int const coords[] );
    REAL_T   GetArrayItem( int i, int j, int k ) const;
    REAL_T & GetArrayItemRef( int i, int j, int k );

    pair<REAL_T,REAL_T> GetRange(double loBound=-HUGE_VAL,
                                 double hiBound= HUGE_VAL) const;
    Box const & GetBox() const { return m_box; }
    int GetBoxNum() const { return m_box.GetIdNum(); }
    Triple<REAL_T> const & GetDx() const { return m_dx; }
    Triple<REAL_T> const & GetOrigin() const { return m_origin; }
    boost::shared_array<REAL_T> GetFArray() const { return m_rep; }
    boost::shared_array<REAL_T> GetSlicedFArray(
        char axis, REAL_T axisPosition ) const;

    bool IsSlice() const { return m_isSlice; }
    char GetSliceAxis() const { return m_sliceAxis; }
    bool ContainsPlane( char axis, double axisPosition ) const;
    void SetLevel( int lev ) { m_level = lev; }

    //
    // In-place transformations
    //
    Status Crop( Box const & croppingBox );
    Status Clamp( Box const & clampingBox );
    void   UnClamp();
    void Apply_unaryOperatorEquals(  double (*f)(double) );
    void Apply_binaryOperatorEquals( double (*f)(double,double), double );
    void Apply_binaryCorrespondingOperatorEquals( 
            double (*f)(double,double), FAB<REAL_T> const & that );

    //
    // Summary statistics
    //
    double Min() const;
    double Max() const;
    double ConstrainedMin( double loBound, double hiBound ) const;
    double ConstrainedMax( double loBound, double hiBound ) const;
    double Sum() const;
    double SumOfSquares() const;
    int    GetNumCells() const;

  protected:
    Box m_box;

  private:
    int m_sizes[ THREE ]; // number of cells along each dimension
    boost::shared_array<REAL_T> m_rep;      // in Fortran order
    bool * m_clampingMask;
    Triple<REAL_T> m_dx; // {dx,dy,dz} at this FAB's level.
    bool const m_isSlice;
    char const m_sliceAxis;
    bool m_real;    // As opposed to subdivided.
    bool m_padded;
    Intvect m_ghost;
    Triple<REAL_T> m_origin;
    int m_level;     // Not strictly necessary, but useful in debugging.
    int m_component; // ditto
    bool m_dummy;

    void Slice( char axis, double axis_position );
    FAB();                          // deliberately unimplemented
    FAB( FAB const & );             // deliberately unimplemented
    FAB & operator=( FAB const & ); // deliberately unimplemented, but see
                                    // DeepCopy().

    // Refcount stuff
    void IncrRefcount(int delta);
    int m_refcount;
    template<typename T> friend void intrusive_ptr_add_ref( T * p );
    template<typename T> friend void intrusive_ptr_release( T * p );
};


/** Should be used only by class BoxLayoutData. */
template<class REAL_T> class FAB_InterfaceForBoxLayoutData : public FAB<REAL_T>
{
  public:
    void SetBoxNum( int b ) { FAB<REAL_T>::m_box.SetIdNum( b ); }
};


template<class REAL_T> std::ostream& operator<<( std::ostream&,
                                                 FAB<REAL_T> const & );


//
// Structure to warehouse smart pointers -- a way to bump their refcount and thus
// prevent them from getting cleaned up.  Very good for smart pointers to FABs;
// FABs can be expensive to load from disk, or in the case of padded FABs compute
// ghost values.
//
// The implementation is a map whose keys are smart pointers and whose values are
// the number of times each smart pointer has been "pushed" on.  Thus the values
// act as a kind of refcount.  It would have been easier to just use a multimap
// and push on multiple copies of each smart pointer, but I'm leary of getting
// into long search times.
//
template<typename T> class FAB_Pinner
{
  public:
    int Push( T const & );
    int Pop(  T const & );
  private:
    std::map< T,int > m_rep;
};

#endif // INCLUDED_FAB_H
