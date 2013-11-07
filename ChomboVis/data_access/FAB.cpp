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

#include "./NormalNumberComparator.h"
#include "../utils/Trace.h"
#include <cmath>
#include <algorithm>

#if __GNUC__ < 3
#   include <function.h>
#else
#   include <cassert>
#   include <functional>
#   include <numeric>
#endif
#include "FABAccess.h"
#include "FAB.h"

using std::ptr_fun;
using std::bind2nd;

//----------- Template utilities --------------------
template<typename InputIterator1, // for data
         typename InputIterator2, // for mask
         typename T>
T accumulate_if( InputIterator1 dataFirst, InputIterator1 dataLast,
                 InputIterator2 maskFirst,
                 T init )
{
    T result(init);
    for( ;
         dataFirst!=dataLast;
         ++dataFirst, ++maskFirst )
    {
        if( *maskFirst )
        {
            result += *dataFirst;
        }
    }
    return result;
}

template<typename InputIterator1, // for data
         typename InputIterator2, // for mask
         typename T,
         typename BinaryFunction> // accumulator
T accumulate_if( InputIterator1 dataFirst, InputIterator1 dataLast,
                 InputIterator2 maskFirst,
                 T init,
                 BinaryFunction binary_op )
{
    T result(init);
    for( ;
         dataFirst!=dataLast;
         ++dataFirst, ++maskFirst )
    {
        if( *maskFirst )
        {
            result = binary_op( result, *dataFirst );
        }
    }    
    return result;
}

//
// transform_if (Adapted from gcc 2.95.3's stl_algo.h.)
//
template <class _InputIter, class _OutputIter, class _UnaryOperation,
          class MaskIter>
_OutputIter transform_if(_InputIter __first, _InputIter __last,
                         _OutputIter __result,
                         MaskIter maskFirst, _UnaryOperation __oper) {
  for ( ; __first != __last; ++__first, ++__result, ++maskFirst)
  {
    if( *maskFirst )
    {
        *__result = __oper(*__first);
    }
  }
  return __result;
}
 
template <class _InputIter1, class _InputIter2, class _OutputIter,
          class _BinaryOperation, class MaskIter>
_OutputIter transform_if(_InputIter1 __first1, _InputIter1 __last1,
                         _InputIter2 __first2, _OutputIter __result,
                         MaskIter maskFirst,
                         _BinaryOperation __binary_op) {
  for ( ; __first1 != __last1; ++__first1, ++__first2, ++__result, ++maskFirst)
  {
    if( *maskFirst )
    {
        *__result = __binary_op(*__first1, *__first2);
    }
  }
  return __result;
}


//----------- Methods of class FAB ------------------
//
/** Constructs a FAB but doesn't load the cell data.  That happens only when
 *  that data is needed.  (See VisualizableDataset::GetFAB().)
*/
template<class REAL_T>
FAB<REAL_T>::FAB( CtorArgs const & ctorArgs )
  : m_box( ctorArgs.box ),
    m_rep( ctorArgs.fieldData ),
    m_clampingMask( 0 ),
    m_dx( ctorArgs.dx ),
    m_isSlice( false ),
    m_sliceAxis( 0 ),
    m_real( ctorArgs.real ),
    m_padded( ctorArgs.padded ),
    m_ghost( ctorArgs.ghost ),
    m_origin( ctorArgs.origin ),
    m_level( ctorArgs.level ),
    m_component( ctorArgs.component ),
    m_dummy( false ),
    m_refcount( 0 )
{
    Trace t("FAB::FAB(Box)");
    assert( ctorArgs.AllSet() );

    // Initialize m_sizes
    m_box.GetDims( m_sizes );
    // In previous versions, FABs didn't own their m_box's; they just pointed
    // into VisualizableDataset's collection of Boxes, since we could share the
    // same Box over all FABs for the same component.  But now we have FABs own
    // their Boxes because when FABs are loaded they pad themselves out with
    // ghost cells and grow their Box.
}


template<typename REAL_T> 
FAB<REAL_T>::FAB( ArgForDummyTemplateInstantiation )
  : m_isSlice( false ), // Initializing this only because it's const
    m_sliceAxis( 'x' ), // Ditto
    m_dummy( true )
{
}



template<class REAL_T> boost::shared_array<REAL_T>
FAB<REAL_T>::GetSlicedFArray( char axis, REAL_T axisPositionXYZ ) const
{
    assert( (axis=='x') || (axis=='y') || (axis=='z') );
    Box slicedBox( m_box, axis );
    int dims[3];
    slicedBox.GetDims( dims );
    int axisNum( axis - 'x' );

    boost::shared_array<REAL_T> slab( new REAL_T[ dims[0] * dims[1] ] );
    int coords[THREE];

    int loCorner = GetBox().GetLoCorner( axisNum );
    int axisPositionIJK = int( (axisPositionXYZ - m_origin[axisNum]) 
                             / m_dx[axisNum] );
    if( (axisPositionIJK - loCorner) == m_sizes[axisNum] )
    {
        axisPositionIJK = int( (axisPositionXYZ - m_origin[axisNum])
                                /m_dx[axisNum] - 1E-10 );
    }
    // That keeps the display looking like the 3D one; if axisPosition is right
    // at the hi extreme of a box, we show a slice from that box.

    for( int g=0; g<dims[0]; ++g )
    {
        for( int h=0; h<dims[1]; ++h )
        {
            int offsetIntoThis = g + dims[0] * h;

            switch( axis )
            {
                case 'x':
                    coords[0] = axisPositionIJK - loCorner;
                    coords[1] = g;
                    coords[2] = h;
                    break;
                case 'y':
                    coords[0] = h;
                    coords[1] = axisPositionIJK - loCorner;
                    coords[2] = g;
                    break;
                case 'z':
                    coords[0] = g;
                    coords[1] = h;
                    coords[2] = axisPositionIJK - loCorner;
                    break;
            }
            slab[ offsetIntoThis ] = GetArrayItem( coords );
        }
    }
    
    return slab;
}


/** Constructs a FAB from a slice of another FAB.  This ctor actually loads up
 *  the cell data too.
*/
template<class REAL_T>
FAB<REAL_T>::FAB(
    boost::intrusive_ptr< FAB<REAL_T> > that,
    char axis, REAL_T axisPositionXYZ ) :
  m_box( that->m_box, axis ),
  m_clampingMask( 0 ),
  m_dx( that->m_dx ),
  m_isSlice( true ),
  m_sliceAxis( axis ),
  m_real( that->m_real ),
  m_padded( that->m_padded ),
  m_ghost( that->m_ghost ),
  m_origin( that->m_origin ),
  m_level( that->m_level ),
  m_component( that->m_component ),
  m_refcount( 0 )
{
    Trace t("FAB::FAB(FAB)");
    assert( (axis=='x') || (axis=='y') || (axis=='z') );

    m_box.GetDims( m_sizes );

    m_rep = that->GetSlicedFArray( axis, axisPositionXYZ );

    assert( !that->m_dummy );
    m_dummy = false;
}


template<class REAL_T>
FAB<REAL_T>::~FAB()
{
    Trace t("FAB::~FAB()");
    if( m_dummy ) return;
    UnClamp();
}


/** Does what you'd expect of operator=().  We just don't want to call it that,
 *  so it'll be easier to track down where we do deep copies.
 *
 *  If one of the FABs is a slice, then the other has to be a slice too, and
 *  they have to be a slice through the same axis.  We could actually allow
 *  those to be different, but until I see a legitimate use for that, we'll
 *  have an assertion to enforce similar-sliceness.
 *
 *  Doesn't touch m_refcount.
*/
template<class REAL_T> void
FAB<REAL_T>::DeepCopy( boost::intrusive_ptr< FAB<REAL_T> > that )
{
    assert( that );
    assert( this != that.get() );

    for( int i=0;i<THREE;++i )
    {
        m_sizes[i] = that->m_sizes[i];
    }

    //
    // m_rep
    //
    long nThat( that->m_box.GetNumCells() );
    assert( nThat > 0 );
    m_rep.reset( new REAL_T[ nThat ] );

    memcpy( m_rep.get(), that->m_rep.get(), nThat*sizeof(REAL_T) );
    m_box = that->m_box;

    UnClamp();
    if( that->m_clampingMask )
    {
        if( ! m_clampingMask )
        {
            m_clampingMask = new bool[ nThat ];
        }
        memcpy( m_clampingMask, that->m_clampingMask, nThat*sizeof(bool) );
    }

    m_dx = that->m_dx;
    assert( m_isSlice == that->m_isSlice ); // const member, better be equal.
    assert( m_sliceAxis == that->m_sliceAxis ); // const member, ditto.
    m_level = that->m_level;
    m_component = that->m_component;
    m_real = that->m_real;
    m_padded = that->m_padded;
    m_ghost = that->m_ghost;
    m_origin = that->m_origin;

    assert( !that->m_dummy );
    m_dummy = false;
}


/** Reduce this FAB to that subset of it that intersects arg croppingBox.
 *
 *  If the FAB has ghost cells, so should the cropped FAB (and some of its ghost
 *  cells may be what were valid cells in the original FAB).  Thus, if the FAB
 *  has ghost cells, it's possible that some of the cropped FAB's ghost cells
 *  will lie outside the cropping box.  This stands in contrast to the
 *  convention for clamping, where the clamping box is, literally and exactly,
 *  the border beyond which we consider no data.
 *
 *  Leaves FAB in unclamped state.
*/
template<class REAL_T> Status
FAB<REAL_T>::Crop( Box const & croppingBox )
{
    UnClamp();

    //
    // Find the cropped Box.
    //
    Status status;
    Box new_mbox;
    if( m_padded )
    {
        Box mboxCopy( m_box );
        mboxCopy.Shrink( m_ghost, true );
        new_mbox = mboxCopy.Intersect( croppingBox, &status );
        new_mbox.Grow( m_ghost, true );
    } else
    {
        new_mbox = m_box.Intersect( croppingBox, &status );
    }
    new_mbox.SetIdNum( m_box.GetIdNum() );

    //
    // Case of (unpadded) FAB completely enclosed within croppingBox: no need to
    // do anything.
    //
    if( new_mbox == m_box )
    {
        return STATUS_OK;                               // Early return
    }


    //
    // Case of cropping box not intersecting this FAB.  If the cropping Box only
    // intersects this FAB's ghost cells, we don't consider that a real
    // intersection; the cropped BoxLayoutData won't retain anything of this FAB
    // in that case.
    //
    // Don't delete [] m_rep (even though we don't need it).  Leave that to 
    // BoxLayoutData::Crop(), which will anyway have to go through the FABs and
    // throw out those left outside the cropping box, and renumber those inside.
    // The way BoxLayoutData will detect FABs to throw away is that their
    // m_sizes will be nonpositive in at least one dimension.
    //
    if( status == STATUS_EMPTY )
    {
        m_box = new_mbox;
        m_box.GetDims( m_sizes );
        return STATUS_OK;                              // Early return
    }


    //
    // Case of new_mbox a proper subset of m_box.
    //
    int new_msizes[3];
    new_mbox.GetDims( new_msizes );
    boost::shared_array<REAL_T> new_mrep( new REAL_T[new_mbox.GetNumCells()] );
    
    Intvect cornerOffset;
    for( int i=0;i<3;++i )
    {
        cornerOffset[i] = new_mbox.GetLoCorner()[i]
                        - m_box.GetLoCorner()[i];
    }
    for( int i=0;
         i<=new_mbox.GetHiCorner().i()-new_mbox.GetLoCorner().i();
         ++i )
    {
        for( int j=0;
             j<=new_mbox.GetHiCorner().j()-new_mbox.GetLoCorner().j();
             ++j )
        {
            for( int k=0;
                 k<=new_mbox.GetHiCorner().k()-new_mbox.GetLoCorner().k();
                 ++k)
            {
                // Set new_mrep.
                FabAccess::GetArrayItemFast<REAL_T>(
                    i,j,k,new_msizes[0], new_msizes[1], new_mrep.get() )
                = FabAccess::GetArrayItemFast<REAL_T>(
                    i+cornerOffset.i(), j+cornerOffset.j(), k+cornerOffset.k(),
                    m_sizes[0], m_sizes[1], m_rep.get() );
            }
        }
    }


    //
    // Replace old data with new.
    //
    m_rep = new_mrep;
    std::copy( new_msizes, new_msizes+3, m_sizes );
    m_box = new_mbox;
    

    return STATUS_OK;
}


/** Until the next call to UnClamp(), apply all pointwise operators and summary
 *  statistics only to cells that are also inside arg clampingBox.
 *  Sets m_clampingMask (which is otherwise NULL), to be an array, parallel to
 *  m_rep, which indicates, with true or false, whether an element of m_rep
 *  falls within clampingBox.
 *
 *  If clampingBox doesn't intersect this FAB, sets all elements of 
 *  m_clampingMask to false;
 *
 *  FAB doesn't have methods for the other kinds of clamping that you find in
 *  BoxLayoutData -- clamping on a single Box of the BoxLayout, or on a
 *  collection of those Boxes -- because those can be handled entirely from
 *  BoxLayoutData; you just don't call into the FABs that you're not clamped on.
*/
template<class REAL_T> Status
FAB<REAL_T>::Clamp( Box const & clampingBox )
{
    UnClamp();
    m_clampingMask = new bool[ m_box.GetNumCells() ];

    Status status;
    Box intersectionBox = m_box.Intersect( clampingBox, &status );

    if( status == STATUS_EMPTY )
    {
        for( int i=0;i<m_box.GetNumCells();++i )
        {
            m_clampingMask[i] = false;
            // Can't do memset; on MacOSX sizeof(bool)!=1.
        }
        return STATUS_OK;                              // Early return
    }

    int coords[3];
    for( int i=0;i<=m_box.GetHiCorner().i()-m_box.GetLoCorner().i();++i )
    {
        coords[0] = i + m_box.GetLoCorner().i();
        for( int j=0;j<=m_box.GetHiCorner().j()-m_box.GetLoCorner().j();++j )
        {
            coords[1] = j + m_box.GetLoCorner().j();
            for( int k=0;k<=m_box.GetHiCorner().k()-m_box.GetLoCorner().k();++k)
            {
                coords[2] = k + m_box.GetLoCorner().k();
                if( intersectionBox.ContainsCell( coords ) )
                {
                    FabAccess::GetArrayItemFast<bool>(
                        i,j,k,m_sizes[0], m_sizes[1], m_clampingMask ) = true;
                } else
                {
                    FabAccess::GetArrayItemFast<bool>(
                        i,j,k,m_sizes[0], m_sizes[1], m_clampingMask ) = false;
                }
            }
        }
    }
    return STATUS_OK;
}


template<class REAL_T> void
FAB<REAL_T>::UnClamp()
{
    delete [] m_clampingMask;
    m_clampingMask = 0;
}


/** Like SetData() except makes a copy of the data (so you can then delete
 *  the argument).
*/
template<class REAL_T> void
FAB<REAL_T>::CopyData( boost::shared_array<REAL_T> data )
{
    int n = m_box.GetNumCells();
    m_rep.reset( new REAL_T[n] );
    memcpy( m_rep.get(), data.get(), n*sizeof(REAL_T) );
}


/** Get a single scalar value out of the FAB.  Arg coords should be THREE long
 *  and its elements should be cell indices relative to the loCorner of the box.
*/
template<class REAL_T> REAL_T
FAB<REAL_T>::GetArrayItem( int i, int j, int k ) const
{
# ifndef NDEBUG
    Trace t("FAB::GetArrayItem()");
# endif

# ifndef NDEBUG
    if( (i<0) || (i>=m_sizes[0]) || (j<0) || (j>=m_sizes[1])
    ||  (k<0) || (k>=m_sizes[2]) )
    {
        t.FatalError( "Out of bounds: (i,j,k)=(%d,%d,%d), msizes=[%d,%d,%d]",
                      i,j,k, m_sizes[0], m_sizes[1], m_sizes[2] );
    }
# endif

    return FabAccess::GetArrayItemFast<REAL_T>(
        i, j, k, m_sizes[0], m_sizes[1], GetFArray().get() );
}


/* Useful for setting a value. */
template<class REAL_T> REAL_T &
FAB<REAL_T>::GetArrayItemRef( int i, int j, int k )
{
# ifndef NDEBUG
    Trace t("FAB::GetArrayItem()");
# endif

# ifndef NDEBUG
    if( (i<0) || (i>=m_sizes[0]) || (j<0) || (j>=m_sizes[1])
    ||  (k<0) || (k>=m_sizes[2]) )
    {
        t.FatalError( "Out of bounds: (i,j,k)=(%d,%d,%d), msizes=[%d,%d,%d]",
                      i,j,k, m_sizes[0], m_sizes[1], m_sizes[2] );
    }
# endif

    return FabAccess::GetArrayItemFast<REAL_T>(
        i, j, k, m_sizes[0], m_sizes[1], GetFArray().get() );
}


template<class REAL_T> REAL_T
FAB<REAL_T>::GetArrayItem( int const coords[] ) const
{
    int i = coords[0];
    int j = coords[1];
    int k = coords[2];
    return GetArrayItem( i, j, k );
}

template<class REAL_T> REAL_T &
FAB<REAL_T>::GetArrayItemRef( int const coords[] )
{
    int i = coords[0];
    int j = coords[1];
    int k = coords[2];
    return GetArrayItemRef( i, j, k );
}


/** Return lo and hi of data range, constrained within [loBound,hiBound]. */
template<class REAL_T> pair<REAL_T,REAL_T>
FAB<REAL_T>::GetRange( double loBound, double hiBound ) const
{
    pair<REAL_T,REAL_T> result(
        ConstrainedMin(loBound,hiBound),
        ConstrainedMax(loBound,hiBound) );
    return result;
}

/** Return number of cells, or if m_clampingMask!=NULL, the number of true's
 *  in m_clampingMask.
*/
template<class REAL_T> int
FAB<REAL_T>::GetNumCells() const
{
    if( m_clampingMask )
    {
        int result(0);
        int n( m_box.GetNumCells() );
        for( int i=0;i<n;++i )
        {
            if( m_clampingMask[i] ) ++result;
        }
        return result;
    } else
    {
        return m_box.GetNumCells();
    }
}

template<class REAL_T> bool
FAB<REAL_T>::ContainsPlane( char axis, double axisPosition ) const
{
    Triple<double> dDx( m_dx[0], m_dx[1], m_dx[2] );
    Triple<double> dOrigin( m_origin[0], m_origin[1], m_origin[2] );
    return m_box.ContainsPlane( axis, axisPosition, dDx, dOrigin );
}


//
// In-place transformations.
//
template<class REAL_T> void
FAB<REAL_T>::Apply_unaryOperatorEquals( double (*f)(double) )
{
    int n( GetBox().GetNumCells() );
    if( m_clampingMask )
    {
        transform_if( m_rep.get(), m_rep.get()+n, m_rep.get(),
                      m_clampingMask, ptr_fun(f) );
    } else
    {
        std::transform( m_rep.get(), m_rep.get()+n, m_rep.get(), ptr_fun(f) );
    }
}


template<class REAL_T> void
FAB<REAL_T>::Apply_binaryOperatorEquals( double (*f)(double,double), double x )
{
    int n( GetBox().GetNumCells() );
    if( m_clampingMask )
    {
        transform_if( m_rep.get(), m_rep.get()+n, m_rep.get(), m_clampingMask,
                           bind2nd(ptr_fun(f),x));
    } else
    {
        std::transform( m_rep.get(), m_rep.get()+n, m_rep.get(), bind2nd(ptr_fun(f),x));
    }
}

template<class REAL_T> void
FAB<REAL_T>::Apply_binaryCorrespondingOperatorEquals(
    double (*f)(double,double), FAB<REAL_T> const & that )
{
    int n( GetBox().GetNumCells() );
    if( m_clampingMask )
    {
        transform_if( m_rep.get(), m_rep.get()+n,
                      that.m_rep.get(), m_rep.get(), m_clampingMask,
                      ptr_fun(f) );
    } else
    {
        std::transform( m_rep.get(), m_rep.get()+n,
                        that.m_rep.get(), m_rep.get(), ptr_fun(f));
    }
}


//
// Summary statistics
//

/** See ConstrainedMin() */
template<class REAL_T> double
FAB<REAL_T>::Min() const
{
    return ConstrainedMin( -HUGE_VAL, HUGE_VAL );
}

/** See ConstrainedMax() */
template<class REAL_T> double
FAB<REAL_T>::Max() const
{
    return ConstrainedMax( -HUGE_VAL, HUGE_VAL );
}

/** Minimum value.  Returns -HUGE_VAL if every element of m_clampingMask is
 *  false.
 *  Ignores NaNs, infs and, doing what FAB::Min() doesn't do, numbers outside
 *  [loBound,hiBound].
*/
template<class REAL_T> double
FAB<REAL_T>::ConstrainedMin( double loBound, double hiBound ) const
{
    int n( GetBox().GetNumCells() );
    REAL_T result( HUGE_VAL );
    if( m_clampingMask )
    {
        for( int i=0;i<n;++i )
        {
            if( m_clampingMask[i] && (m_rep[i] < result)
                && (IsNormalNumber(m_rep[i],loBound,hiBound)) )
            {
                result = m_rep[i];
            }
        }
    } else
    {
        result = *(std::min_element( m_rep.get(), m_rep.get()+n ));
        if( (! IsNormalNumber(result,loBound,hiBound))
            || (result < loBound)  ||  (result > hiBound) )
        {
            result = *(std::min_element( m_rep.get(), m_rep.get()+n,
                           NormalNumberComparatorForMin(loBound,hiBound) ));
        }
    }
    return result;
}

/** Maximum value.  Returns -HUGE_VAL if every element of m_clampingMask is
 *  false.
 *  Ignores NaNs, infs and, doing what FAB::Max() doesn't do, numbers outside
 *  [loBound,hiBound].
*/
template<class REAL_T> double
FAB<REAL_T>::ConstrainedMax( double loBound, double hiBound ) const
{
    int n( GetBox().GetNumCells() );
    REAL_T result( -HUGE_VAL );
    if( m_clampingMask )
    {
        for( int i=0;i<n;++i )
        {
            if( m_clampingMask[i] && (m_rep[i] > result)
                && (IsNormalNumber(m_rep[i],loBound,hiBound)) )
            {
                result = m_rep[i];
            }
        }
    } else
    {
        result = *(std::max_element( m_rep.get(), m_rep.get()+n ));
        if( (! IsNormalNumber(result,loBound,hiBound)) || (result < loBound)
                                       || (result > hiBound) )
        {
            result = *(std::max_element( m_rep.get(), m_rep.get()+n,
                         NormalNumberComparatorForMax(loBound,hiBound) ));
        }
    }
    return result;
}

/** Sum of values */
template<class REAL_T> double
FAB<REAL_T>::Sum() const
{
    int n( GetBox().GetNumCells() );
    if( m_clampingMask )
    {
        return accumulate_if( m_rep.get(), m_rep.get()+n, m_clampingMask, 0.0 );
    } else
    {
        return std::accumulate( m_rep.get(), m_rep.get()+n, 0.0 );
    }
}

/** Sum of squares of values */
template<class REAL_T> double mysumsqr( REAL_T x, REAL_T y )
{
    return x + y*y;
}
template<class REAL_T> double
FAB<REAL_T>::SumOfSquares() const
{
    int n( GetBox().GetNumCells() );
    if( m_clampingMask )
    {
        return accumulate_if( m_rep.get(), m_rep.get()+n, m_clampingMask, 0.0,
                                   mysumsqr<REAL_T> );
    } else
    {
        return std::accumulate( m_rep.get(), m_rep.get()+n, 0.0,
                           mysumsqr<REAL_T> );
    }
}


template<class REAL_T> std::ostream& operator<<( std::ostream & out,
                                                 FAB<REAL_T> const & fab )
{
    int dims[3];
    fab.GetBox().GetDims( dims );

    if( dims[2] == 1 )
    {
        for( int j=0;j<dims[1];++j )
        {
            for( int i=0;i<dims[0];++i )
            {
                out << fab.GetArrayItem(i,j,0) << " ";
            }
            out << '\n';
        }
    } else
    {
        for( int k=0;k<dims[2];++k )
        {
            for( int j=0;j<dims[1];++j )
            {
                for( int i=0;i<dims[0];++i )
                {
                    out << fab.GetArrayItem(i,j,0) << " ";
                }
                out << '\n';
            }
            out << '\n';
        }
    }

    return out;
}

// Return the number of times arg pf has been "pushed" onto the map (not
// including this time) net of the number of times it's been popped.
template<typename T> int
FAB_Pinner<T>::Push( T const & t )
{
    typename std::map<T,int>::iterator iter = m_rep.find( t );
    int refcount;
    if( iter == m_rep.end() )
    {
        refcount = 0;
    } else
    {
        refcount = iter->second;
    }
    m_rep.insert( typename std::map<T,int>::value_type( t, refcount+1 ) );
    return refcount;
}


// Return the number of times arg pf has been "pushed" onto the map (not
// including this time) net of the number of times it's been popped.
// If that "refcount" is 1, then erase the element altogether.
template<typename T> int
FAB_Pinner<T>::Pop( T const & t )
{
    Trace tracer("FAB_Pinner::Pop()");
    typename std::map<T,int>::iterator iter = m_rep.find( t );
    int refcount;
    if( iter == m_rep.end() )
    {
        refcount = 0;
        tracer.Error( "Tried to pop FAB that was never pushed." );
    } else
    {
        refcount = iter->second;
        if( (-- (iter->second)) == 0 )
        {
            m_rep.erase( iter );
        }
    }
    return refcount;
}


/** Initialize to wacky values that we can check for in AllSet(), and thus
 *  detect if the user forgot to set one of the fields.
*/
template<typename REAL_T>
FAB<REAL_T>::CtorArgs::CtorArgs()
 : box( Box::WackyBox() ),
   fieldData( boost::shared_array<REAL_T>() ),
   dx( Triple<REAL_T>::WackyTriple() ),
   origin( Triple<REAL_T>::WackyTriple() ),
   level(-1),
   component(-1),
   ghost( Triple<int>::WackyTriple() )
{
}


/** Return true if all the fields appear to have been set, i.e. if they're
 *  something other than their defaults.
 *  FIXME: We don't have a good way to check the two bool fields, real and
 *  padded.
*/
template<typename REAL_T> bool
FAB<REAL_T>::CtorArgs::AllSet() const
{
    return (box != Box::WackyBox())
        && fieldData
        && (dx != Triple<REAL_T>::WackyTriple() )
        && (origin != Triple<REAL_T>::WackyTriple() )
        && (level != -1)
        && (component != -1)
        && (ghost != Triple<int>::WackyTriple() );
}


template<typename REAL_T> void
FAB<REAL_T>::IncrRefcount( int delta )
{
    assert( (delta == -1) || (delta == +1) );
    m_refcount += delta;
}


//
// Explicit instantiations, to go into the library.
// See Stroustrup 3rd edition, p.866.
//
template class FAB<float>;
template class FAB<double>;
template class FAB_Pinner< boost::intrusive_ptr< FAB<float> > >;
template class FAB_Pinner< boost::intrusive_ptr< FAB<double> > >;
