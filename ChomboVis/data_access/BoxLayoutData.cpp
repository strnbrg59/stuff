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

#include "PointerHandleMap.h"
#include "BoxLayoutData.h"
#include "../utils/Trace.h"
#include <numeric>
#include <cassert>
#include <iostream>
using std::cerr;
using std::endl;

template<class T> T myplus(T x, T y ) { return x+y; }
template<class T> T myminus(T x, T y ) { return x-y; }
template<class T> T mytimes(T x, T y ) { return x*y; }
template<class T> T mydivide(T x, T y ) { return x/y; }


//
// The BoxLayoutData constructors are private.  Use NewVector() to allocate
// and initialize a vector of BoxLayoutData's, one for each component.
//
template<class REAL_T>
BoxLayoutData<REAL_T>::BoxLayoutData()
  : m_initialized(false)
{
}

template<class REAL_T> void
BoxLayoutData<REAL_T>::Init( BoxLayout<REAL_T> const & boxLayout,
                             int level, int component )
{
    Trace t("BoxLayoutData::Init()");
    m_boxLayout = boxLayout;
    int numBoxes = boxLayout.size();
    assert( m_fabs.capacity() == 0 );
    assert( m_fabs.size() == 0 );
    m_fabs.reserve( numBoxes );
    m_level = level;
    m_component = component;

    for( int b=0;b<numBoxes;++b )
    {
        m_fabs.push_back( boost::intrusive_ptr<FAB<REAL_T> >() );
    }
    m_initialized = true;
}


/*static*/ template<class REAL_T> boost::shared_ptr< BoxLayoutData<REAL_T> > *
BoxLayoutData<REAL_T>::NewVector(
    int numComponents, BoxLayout<REAL_T> const & boxLayout, int level )
{
    boost::shared_ptr< BoxLayoutData<REAL_T> > * result =
        new boost::shared_ptr< BoxLayoutData<REAL_T> >[ numComponents ];
    for( int c=0;c<numComponents;++c )
    {
        result[c].reset( new BoxLayoutData<REAL_T> );
        result[c]->Init( boxLayout, level, c );
    }
    return result;
}


/** Expand arg currentBlds by one component.
 *  Arg numComponents is the number of components currently (i.e. the length
 *  of currentBlds.
*/
/*static*/ template<class REAL_T> void
BoxLayoutData<REAL_T>::GrowVector(
    boost::shared_ptr< BoxLayoutData<REAL_T> > * * currentBLDs,
    int numComponents )
{
    BoxLayout<REAL_T> const & boxLayout( (*currentBLDs)[0]->m_boxLayout );
    int level( (*currentBLDs)[0]->m_level );

    boost::shared_ptr< BoxLayoutData<REAL_T> > * expandedBLDs =
        new boost::shared_ptr< BoxLayoutData<REAL_T> >[numComponents + 1];
    for( int c=0;c<numComponents;++c )
    {   // expandedBLDs isn't init'ed yet, so don't try to delete[] its m_fabs!
        expandedBLDs[c] = (*currentBLDs)[c];
    }

    boost::shared_ptr< BoxLayoutData<REAL_T> > newBLD(new BoxLayoutData<REAL_T>);
    newBLD->Init(boxLayout, level, numComponents );
    expandedBLDs[numComponents] = newBLD;

    delete [] (*currentBLDs);
    *currentBLDs = expandedBLDs;
}


/** Remove the last component. 
 *  Arg numComponents should be the number of components *before* the shrinking.
 */
/*static*/ template<class REAL_T> void
BoxLayoutData<REAL_T>::ShrinkVectorByOne(
    boost::shared_ptr< BoxLayoutData<REAL_T> > * * currentBLDs,
    int numComponents )
{
    Trace t("BoxLayoutData::ShrinkVectorByOne()");
    boost::shared_ptr< BoxLayoutData<REAL_T> > * shrunkenBLDs =
        new boost::shared_ptr< BoxLayoutData<REAL_T> >[numComponents - 1];
    for( int c=0;c<numComponents-1;++c )
    {   // shrunkenBLDs isn't init'ed yet, so don't try to delete[] its m_fabs!
        shrunkenBLDs[c] = (*currentBLDs)[c];
    }

    delete [] (*currentBLDs);
    *currentBLDs = shrunkenBLDs;
}
 

template<class REAL_T> boost::shared_ptr< BoxLayoutDataInterfaceForPython >
BoxLayoutData<REAL_T>::Clone() const
{
    BoxLayoutData<REAL_T> * result = new BoxLayoutData;
    result->Init( m_boxLayout, m_level, m_component );
    assert( result->IsInitialized() );
    for( unsigned b=0; b<m_boxLayout.size(); ++b )
    {
        boost::shared_array<REAL_T> dummyArray( new REAL_T[1] );
        typename FAB<REAL_T>::CtorArgs fabCtorArgs;
        fabCtorArgs.box = m_fabs[b]->GetBox();
        fabCtorArgs.fieldData = dummyArray; // Replaced in DeepCopy()
        fabCtorArgs.dx = m_fabs[b]->GetDx();
        fabCtorArgs.origin = m_fabs[b]->GetOrigin();
        fabCtorArgs.level = m_level;
        fabCtorArgs.component = m_component;
        fabCtorArgs.real = false;
        fabCtorArgs.padded = m_fabs[b]->IsPadded();
        fabCtorArgs.ghost = m_fabs[b]->GetGhost();
        result->m_fabs[b] = boost::intrusive_ptr<FAB<REAL_T> >(
            new FAB<REAL_T>( fabCtorArgs ) );
        result->m_fabs[b]->DeepCopy( m_fabs[b] );
    }
    result->m_clampingBoxNums = m_clampingBoxNums;
    result->m_pinnedFabs = m_pinnedFabs;
    return boost::shared_ptr< BoxLayoutDataInterfaceForPython >(result);
}


template<class REAL_T> boost::intrusive_ptr< FAB<REAL_T> >
BoxLayoutData<REAL_T>::GetFAB( int boxNum ) const
{
    return m_fabs[boxNum];
}

template<class REAL_T> void
BoxLayoutData<REAL_T>::SetFAB( int boxNum, boost::intrusive_ptr<FAB<REAL_T> > fab )
{
    m_fabs[boxNum] = fab;
}


/** Returns vector<double>, instead of vector<REAL_T> (or even Triple<REAL_T>),
 *  because we want to call this from Python.  Returns 2 elements if dataset
 *  is 2D, 3 otherwise.
*/
template<class REAL_T> vector<double>
BoxLayoutData<REAL_T>::GetDx() const
{
    Triple<REAL_T> dx( m_boxLayout.GetDx() );
    vector<double> result( dx.m_data, dx.m_data+3 );
    if( m_boxLayout.GetDimensionality() == 2 )
    {
        result.pop_back();
    }
    return result;
}

/** Returns vector<double>, instead of vector<REAL_T> (or even Triple<REAL_T>),
 *  because we want to call this from Python.  Returns 2 elements if dataset
 *  is 2D, 3 otherwise.
*/
template<class REAL_T> vector<double>
BoxLayoutData<REAL_T>::GetOrigin() const
{
    Triple<REAL_T> origin( m_boxLayout.GetOrigin() );
    vector<double> result( origin.m_data, origin.m_data+3 );
    if( m_boxLayout.GetDimensionality() == 2 )
    {
        result.pop_back();
    }
    return result;
}


template<class REAL_T>
BoxLayoutData<REAL_T>::~BoxLayoutData()
{
    assert( m_initialized );
    m_initialized = false;
}



/** Returns true if all the FABs are loaded with data.  If they're not, then
 *  any operations on this BoxLayoutData are hazardous.
*/
template<class REAL_T> bool
BoxLayoutData<REAL_T>::FABsAreFilled() const
{
    for( unsigned b=0;b<m_boxLayout.size();++b )
    {
        if( ! m_fabs[b] )
        {
            return false;
        }
    }
    return true;
}


/** Returns data element (if cell-centered, this corresponds to a cell) of
 *  indicated box.
*/
template<class REAL_T> double
BoxLayoutData<REAL_T>::GetDatum( int boxNum, int i, int j, int k/*=0*/ ) const
{
    //Trace t("BoxLayoutData::GetDatum()");
    assert( unsigned(boxNum) <= m_boxLayout.size() );
    assert( m_fabs[boxNum] );
    double result(m_fabs[boxNum]->GetArrayItem( i,j,k ));
    //t.Info( "datum(%d,%d,%d,%d)=%f", boxNum,i,j,k,result );
    //cerr << "BoxLayoutData::GetDatum(" << boxNum << "," << i << "," << j
    //     << "," << k << ")=" << result << endl;
    return result;
}

/** Sets one data element.
 *  See also BoxLayoutData::Clone().
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::SetDatum( double x, int boxNum, int i, int j,int k/*=0*/)
{
    assert( unsigned(boxNum) <= m_boxLayout.size() );
    assert( m_fabs[boxNum] );
    (m_fabs[boxNum])->GetArrayItemRef( i, j, k ) = x;
    return STATUS_OK;
}


template<class REAL_T> BoxLayout<REAL_T> const &
BoxLayoutData<REAL_T>::GetBoxLayout() const
{
    return m_boxLayout;
}


/** This is of interest to the Python wrapping, where we convert these nested
 *  vectors into correspondingly nested tuples.
 *  Dimensionality assumed to be 2, if locorner[k]==hicorner[k].
 *  The nested vectors represent...
 *  ...                layout     lo&hi corners    corner (i,j,[k])      */
template<class REAL_T> vector<    vector<          vector<int> > >
BoxLayoutData<REAL_T>::GetBoxLayoutAsNestedVectors() const
{
    return m_boxLayout.GetAsNestedVectors();
}


/** Restrict all pointwise operators and summary statistics to operate on the
 *  indicated FABs.  Undo with UnClamp().
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Clamp( vector<int> boxNums )
{
    Trace t("BoxLayoutData::Clamp()");
    UnClamp();
    m_clampingBoxNums = boxNums;
    return STATUS_OK;
}

template<class REAL_T> Status
BoxLayoutData<REAL_T>::Clamp( int boxNum )
{
    Trace t("BoxLayoutData::Clamp()");
    UnClamp();
    m_clampingBoxNums.push_back(boxNum);
    return STATUS_OK;
}


/** Arg clampingBox is any rectangular prism.  Clamped region may therefore
 *  include fractions of our BoxLayout.
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::ClampToGeneralBox( Box const & clampingBox )
{
    UnClamp();
    for( unsigned b=0;b<m_boxLayout.size();++b )
    {
        assert( m_fabs[b] );
        m_fabs[b]->Clamp( clampingBox );
    }
    return STATUS_OK;
}


/** Disable effect of any previous call to Clamp() or ClampToGeneralBox(). */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::UnClamp()
{
    m_clampingBoxNums.clear();
    for( unsigned b=0;b<m_boxLayout.size();++b )
    {
        assert( m_fabs[b] );
        m_fabs[b]->UnClamp();
    }
    return STATUS_OK;
}


//
// In-place math transformations.
//

/*
 *  If optional arg boxNum != -1, apply operation only to that FAB.
 *  If m_clampingBoxNums is not empty, apply operation only to the FABs it
 *    enumerates.
 *  If boxNum!=NULL and !m_clampingBoxNum.empty(), boxNum overrides.
 *  Otherwise apply operation to all the FABs.
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_niladicFabOperator(
    double (*pf)(double), int boxNum )
{
    assert( FABsAreFilled() );
    if( boxNum != -1 )
    {
        m_fabs[boxNum]->Apply_unaryOperatorEquals(pf);
    } else
    if( !m_clampingBoxNums.empty() )
    {
        for( unsigned i=0;i<m_clampingBoxNums.size();++i )
        {
            m_fabs[m_clampingBoxNums[i]]->Apply_unaryOperatorEquals(pf);
        }
    } else
    {
        for( unsigned b=0;b<m_boxLayout.size();++b )
        {
            m_fabs[b]->Apply_unaryOperatorEquals(pf);
        }
    }
    return STATUS_OK;
}

/** See comments for Apply_niladicFabOperator() for explanation of the role of
 *  the boxNum argument, and of m_clampingBoxNums.
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_monadicFabOperator(
    double (*pf)(double,double), REAL_T x, int boxNum )
{
    assert( FABsAreFilled() );
    if( boxNum != -1 )
    {
        m_fabs[boxNum]->Apply_binaryOperatorEquals(pf,x);
    } else
    if( !m_clampingBoxNums.empty() )
    {
        for( unsigned i=0;i<m_clampingBoxNums.size();++i )
        {
            m_fabs[m_clampingBoxNums[i]]->Apply_binaryOperatorEquals(pf,x);
        }
    } else
    {
        for( unsigned b=0;b<m_boxLayout.size();++b )
        {
            m_fabs[b]->Apply_binaryOperatorEquals(pf,x);;
        }
    }
    return STATUS_OK;
}

/** Argument is element of corresponding FAB (rather than a REAL_T scalar). */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_monadicCorrespondingFabOperator(
    double (*pf)(double,double),
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
    int boxNum )
{
    assert( FABsAreFilled() );
    BoxLayoutData<REAL_T> const * concreteThatPtr =
        static_cast<BoxLayoutData<REAL_T> const *>( that.get() );

    if( boxNum != -1 )
    {
        m_fabs[boxNum]->Apply_binaryCorrespondingOperatorEquals(
            pf, *( concreteThatPtr->m_fabs[boxNum] ) );
    } else
    if( !m_clampingBoxNums.empty() )
    {
        for( unsigned i=0;i<m_clampingBoxNums.size();++i )
        {
            m_fabs[m_clampingBoxNums[i]
                  ]->Apply_binaryCorrespondingOperatorEquals(
                      pf, *(concreteThatPtr->m_fabs[m_clampingBoxNums[i]]) );
        }
    } else
    {
        for( unsigned b=0;b<m_boxLayout.size();++b )
        {
            m_fabs[b]->Apply_binaryCorrespondingOperatorEquals(
                pf, *(concreteThatPtr->m_fabs[b]) );
        }
    }
    return STATUS_OK;
}


/** Transform data to log() of itself.
 *  If optional arg boxNum!=-1, apply operation only to that FAB.  Otherwise
 *     apply it to all the FAB.
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_log( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( log, boxNum );
}

/** Antilogarithm */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_exp( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( exp, boxNum );
}

/** Raise all elements to the x power. */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_pow( double x, int boxNum /*=-1*/ )
{
    return Apply_monadicFabOperator( pow, x, boxNum );
}

/** Raise all elements to the corresponding BLD element-th power. */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_powBLD(
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
    int boxNum /*=-1*/ )
{
    return Apply_monadicCorrespondingFabOperator(
        pow, that, boxNum );
}

//
// Trigonometric functions
//
/** sine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_sin( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( sin, boxNum );
}
/** cosine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_cos( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( cos, boxNum );
}
/** tangent */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_tan( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( tan, boxNum );
}
/** arcsine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_asin( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( asin, boxNum );
}
/** arccosine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_acos( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( acos, boxNum );
}
/** arctangent */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_atan( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( atan, boxNum );
}

/** hyperbolic sine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_sinh( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( sinh, boxNum );
}
/** hyperbolic cosine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_cosh( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( cosh, boxNum );
}
/** hyperbolic tangent */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_tanh( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( tanh, boxNum );
}
/** inverse hyperbolic sine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_asinh( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( asinh, boxNum );
}
/** inverse hyperbolic cosine */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_acosh( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( acosh, boxNum );
}
/** inverse hyperbolic tangent */
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_atanh( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( atanh, boxNum );
}


/** Transform data to fabs() of itself.
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_fabs( int boxNum /*=-1*/ )
{
    return Apply_niladicFabOperator( fabs, boxNum );
}


template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_plusEquals( double x, int boxNum /*=-1*/ )
{
    return Apply_monadicFabOperator( myplus, x, boxNum );
}
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_plusEqualsBLD(
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
    int boxNum /*=-1*/ )
{
    return Apply_monadicCorrespondingFabOperator(
        myplus, that, boxNum );
}


template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_minusEquals( double x, int boxNum /*=-1*/ )
{
    return Apply_monadicFabOperator(
        myminus, x, boxNum );
}
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_minusEqualsBLD(
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
    int boxNum /*=-1*/ )
{
    return Apply_monadicCorrespondingFabOperator(
        myminus, that, boxNum );
}

template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_timesEquals( double x, int boxNum /*=-1*/ )
{
    return Apply_monadicFabOperator(
        mytimes, x, boxNum );
}
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_timesEqualsBLD(
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
    int boxNum /*=-1*/ )
{
    return Apply_monadicCorrespondingFabOperator(
        mytimes, that, boxNum );
}


template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_divideEquals( double x, int boxNum /*=-1*/ )
{
    return Apply_monadicFabOperator(
        mydivide, x, boxNum );
}
template<class REAL_T> Status
BoxLayoutData<REAL_T>::Apply_divideEqualsBLD(
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
    int boxNum /*=-1*/ )
{
    return Apply_monadicCorrespondingFabOperator(
        mydivide, that, boxNum );
}

//
// Summary statistics
//
template<class REAL_T> template<class U> U
BoxLayoutData<REAL_T>::AccumOverBoxes( 
    U (FAB<REAL_T>::*pbf)() const, // Function applied to each FAB
    U (*accumFunc)(U,U),           // binary_op for std::accumulate()
    U init,                        // Starting point (e.g. zero for sums)
    int boxNum /*=-1*/ ) const     // Optional restriction to a box
{
    if( boxNum != -1 )
    {
        return (m_fabs[boxNum].get()->*pbf)();
    } else
    {
        vector<U> accumulatedResults;
        if( !m_clampingBoxNums.empty() )
        {
            accumulatedResults.reserve( m_clampingBoxNums.size() );
            for( unsigned i=0;i<m_clampingBoxNums.size();++i )
            {
                accumulatedResults.push_back(
                    (m_fabs[m_clampingBoxNums[i]].get()->*pbf)() );
            }
        } else
        {
            accumulatedResults.reserve( m_boxLayout.size() );
            for( unsigned b=0;b<m_boxLayout.size();++b )
            {
                assert( m_fabs[b] );
                if( m_fabs[b]->GetNumCells() > 0 ) // In case clamping by
                {   // general box leaves this fab out alltogether.
                    accumulatedResults.push_back( (m_fabs[b].get()->*pbf)() );
                }
            }
        }
        return accumulate( accumulatedResults.begin(),
                           accumulatedResults.end(),
                           init, accumFunc );
    }
}

template<class U> U mymin(U x, U y) { return std::min(x,y); }
template<class U> U mymax(U x, U y) { return std::max(x,y); }
template<class REAL_T> double
BoxLayoutData<REAL_T>::Min( int boxNum /*=-1*/ ) const
{
    return AccumOverBoxes( &FAB<REAL_T>::Min, mymin, double(+9E100),boxNum);
}
template<class REAL_T> double
BoxLayoutData<REAL_T>::Max( int boxNum /*=-1*/ ) const
{
    return AccumOverBoxes( &FAB<REAL_T>::Max, mymax, double(-9E100),boxNum);
}


/** Return the number of cells in the indicated box (if != -1), or, if
 *  boxNum==-1 in the box indicated by the last Clamp() call, or if nothing is
 *  clamped, return the number of cells in all the boxes.
*/
template<class REAL_T> int
BoxLayoutData<REAL_T>::GetNumCells( int boxNum /*=-1*/ ) const
{
    return AccumOverBoxes( &FAB<REAL_T>::GetNumCells, &myplus, 0, boxNum );
}

template<class REAL_T> double
BoxLayoutData<REAL_T>::Sum( int boxNum /*=-1*/ ) const
{
    return AccumOverBoxes( &FAB<REAL_T>::Sum, &myplus<double>, 0.0, boxNum );
}

template<class REAL_T> double
BoxLayoutData<REAL_T>::SumOfSquares( int boxNum /*=-1*/ ) const
{
    return AccumOverBoxes( &FAB<REAL_T>::SumOfSquares,
                           &myplus<double>, 0.0, boxNum );
}
    

//
// Unit test and supporting functions.
//

template<class REAL_T> Status
BoxLayoutData<REAL_T>::UnitTest( int lev, int component )
{
    srand(1);
    Trace t("BoxLayoutData::UnitTest()");
    cerr << "GetBoxLayoutAsNestedVectors()=" << GetBoxLayoutAsNestedVectors()
         << endl;

    int corners[] = {1,2,3,5,6,7};
    Box clampBox( corners );
    clampBox *= 0.1/GetDx()[0];
    ClampToGeneralBox( clampBox );
    cerr << "Sum(clamped on ((1,2,3),(5,6,7)) )=" << Sum() << endl;
    cerr << "SumSqr(clamped on ((1,2,3),(5,6,7)) )=" << SumOfSquares() << endl;
    Clamp(0);
    cerr << "Sum(clamped on 0)=" << Sum() << endl;
    cerr << "SumSqr(clamped on 0)=" << SumOfSquares() << endl;
    UnClamp();
    for( unsigned rb=0; rb<GetBoxLayout().size(); ++rb )
    {
        Box box( GetBoxLayout()[rb] );
        int coords[3];  coords[2] = 0;
        int dimensionality = box.GetHiCorner()[2]==box.GetLoCorner()[2]
                           ?  2 : 3;
        for( int i=0;i<dimensionality;++i )
        {
            coords[i]= random() % (1+box.GetHiCorner()[i]-box.GetLoCorner()[i]);
        }
        REAL_T datum = GetDatum( rb, coords[0],coords[1],coords[2]);
        cerr << "Raw datum: ";
        ElementPrinter( lev, component, rb, coords, box.GetLoCorner(), datum );

        //
        // Testing SetDatum() (much more comprehensive test in api_test.py).
        //
        SetDatum(-datum*datum, rb, coords[0],coords[1],coords[2]); 

        //
        // Testing various Apply_* functions.
        // 
        Apply_fabs();
        Apply_plusEquals(0.1, rb);
        Apply_log(rb);

        // If you apply log() or anything else and get inf's or NaN's
        // your ascii output will have 0x0's in it (I think) and then
        // you'll get a broken pipe when you try to save to hdf5.  Or, more
        // likely, operator>>(VisualizableDataset) just doesn't print as much
        // stuff as ascii2hdf5 is expecting (or maybe more...).
        Apply_minusEquals(10.0, rb);
        Apply_timesEquals(10.0,rb);

        REAL_T transformedDatum =
            GetDatum( rb, coords[0],coords[1],coords[2]);
        cerr << "Transformed datum: ";
        ElementPrinter( lev, component, rb, coords, box.GetLoCorner(),
                        transformedDatum );
    }

    return STATUS_OK;
}


/** Helper for UnitTest().  Pretty-printer. */
template<class REAL_T> void
BoxLayoutData<REAL_T>::ElementPrinter(
    int level, int component, int box, int coords[3], Intvect locorner,
    REAL_T datum ) const
{
    cerr << "Data(level=" << level << " component=" << component 
            << " box=" << box
         << "(ijk)=("
         << coords[0] + locorner[0] << "," 
         << coords[1] + locorner[1] << ","
         << coords[2] + locorner[2]
         << ")) = "
         << datum
         << endl;
}


/** Returns a handle into a map of pointers. */
template<class REAL_T> int
BoxLayoutData<REAL_T>::GetFArray( int boxNum ) const
{
    Trace t("BoxLayoutData::GetFArray()");
    boost::intrusive_ptr<FAB<REAL_T> > fab( GetFAB( boxNum ) );
    return SharedArrayHandleMap<REAL_T>::GetHandle( fab->GetFArray() );
}


/** Used in Python wrapping, for constructing a NumPy array. */
template<class REAL_T> vector<vector<vector<double> > >
BoxLayoutData<REAL_T>::GetFArrayAsVectorMatrix( int boxNum ) const
{
    boost::intrusive_ptr<FAB<REAL_T> > fab( GetFAB( boxNum ) );
    Box box( fab->GetBox() );
    int dims[3];
    int coords[3];
    box.GetDims( dims );
    vector<vector<vector<double> > > result;
    result.reserve(dims[0]);
    for( int i=0;i<dims[0];++i )
    {
        coords[0] = i;
        vector<vector<double> > v1;
        v1.reserve(dims[1]);
        for( int j=0;j<dims[1];++j )
        {
            coords[1] = j;
            vector<double> v2;
            v2.reserve(dims[2]);
            for( int k=0;k<dims[2];++k )
            {
                coords[2] = k;
                double x = fab->GetArrayItem( coords );
                v2.push_back(x);
            }
            v1.push_back(v2);
        }
        result.push_back(v1);
    }
    return result;
}


/** In-place surgery: throws away FABs that lie outside arg croppingBox.
 *  Keeps FABs (or parts of them) that are inside.  Renumbers remaining FABs
 *  as 0,1,....  Leaves BoxLayoutData in unclamped state.
 *
 *  Doesn't make any effort to combine boxes; each original box results in
 *  either zero or one new boxes.
 *
 *  See comments under FAB::Crop() for details on what we do about ghost cells.
 *
 *  If you're interested in obtaining something to save to HDF5, then it's wise
 *  to call this function on the BoxLayoutData members of VisualizableDataset
 *  that are real and not contrapadded.  
 *  FIXME1: Enforce that, in VisualizableDataset::Crop().  If we crop unreal
 *  FABs, what do we set m_realFAB to anyway??
 *  FIXME2: Don't call this function on a BLD that's still inside a
 *  VisualizableDataset; get yourself a clone first.  Otherwise, the m_realFAB
 *  pointers in unreal FABs will end up screwy or dangling.
 *
 *  It would be nice if we could crop a BoxLayoutData without having to first
 *  load all the BoxLayoutData's data; after cropping, we might need only a
 *  small piece.  Unfortunately, a BoxLayoutData doesn't know how to go to HDF5
 *  to load data as needed (only VisualizableDataset can do that).  We could
 *  try moving that functionality down to BoxLayoutData but the problem is how
 *  to deal with ghost cell generation; one BoxLayoutData would need to know
 *  about a coarser BoxLayoutData.
*/
template<class REAL_T> Status
BoxLayoutData<REAL_T>::CropToGeneralBox(  Box const & croppingBox )
{
    Trace t("BoxLayoutData::CropToGeneralBox()");
    assert( m_initialized );

    UnClamp();
    
    Status status;
    vector<Box> newBoxes;
    vector< boost::intrusive_ptr<FAB<REAL_T> > > newFABs;
    newBoxes.reserve( m_boxLayout.size() );
    newFABs.reserve( m_boxLayout.size() );
    unsigned nNewBoxes(0);
    for( unsigned i=0;i<m_boxLayout.size();++i )
    {
        m_fabs[i]->Crop( croppingBox );

        Box box( m_boxLayout[i].Intersect( croppingBox, &status ) );
        if( status != STATUS_EMPTY )
        {
            box.SetIdNum( nNewBoxes );
            static_cast<FAB_InterfaceForBoxLayoutData<REAL_T> *>(
                m_fabs[i].get())->SetBoxNum( nNewBoxes );
            ++nNewBoxes;
            newBoxes.push_back( box );
            newFABs.push_back( m_fabs[i] );
        }
    }
    
    assert( nNewBoxes == newBoxes.size() );
    m_boxLayout = BoxLayout<REAL_T>( newBoxes, m_boxLayout.GetDx(),
                                     m_boxLayout.GetOrigin() );
    m_fabs = newFABs;

    return STATUS_OK;
}


/** Make all the FABs think they really belong to the indicated level.  Used
 *  in VisualizableDataset::Crop(), when we crop to a subset of the levels (and
 *  then we need to renumber the remaining levels).
*/
template<class REAL_T> void
BoxLayoutData<REAL_T>::SetLevel( int lev )
{
    for( unsigned i=0;i<m_fabs.size();++i )
    {
        m_fabs[i]->SetLevel( lev );
    }
}


/** Copies arg to a structure, to keep its ref count one more than it would
 *  otherwise be.
 *  Returns item "refcount" (not including this call) on m_pinnedFabs.
*/
template<class REAL_T> int
BoxLayoutData<REAL_T>::PinFAB( boost::intrusive_ptr< FAB<REAL_T> > const & fab )
{
    return m_pinnedFabs.Push( fab );
}


/** See PinFAB().
 *  Returns item "refcount" (not including this call) on m_pinnedFabs.
*/
template<class REAL_T> int
BoxLayoutData<REAL_T>::UnpinFAB( boost::intrusive_ptr< FAB<REAL_T> > const & fab)
{
    return m_pinnedFabs.Pop( fab );
}


/** Delete FABs whose use_count==1. */
template<class REAL_T> void
BoxLayoutData<REAL_T>::FreeUnusedFABs()
{
    Trace t("BoxLayoutData::FreeUnusedFABs()");
    for( unsigned i=0;i<m_fabs.size();++i )
    {
        if( ! m_fabs[i] )
        {
            continue;
        }

        if( m_fabs[i]->GetRefcount() == 1 )
        {
            t.Info( "fab[%d] reset!", i );
            m_fabs[i] = 0;
        }
    }
}

template<class REAL_T> void
BoxLayoutData<REAL_T>::PrintFabUseCounts() const
{
    Trace t("BoxLayoutData::PrintFabUseCounts()");
    for( unsigned i=0;i<m_fabs.size();++i )
    {
        if( ! m_fabs[i] )
        {
            continue;
        }

        if( m_fabs[i]->GetRefcount() > 0 )
        {
            t.Info( "m_fabs[%d]->GetRefcount()=%d", i, m_fabs[i]->GetRefcount() );
        }
    }
}


template<class REAL_T> int
BoxLayoutData<REAL_T>::GetTotalFabUseCount() const
{
    int result = 0;
    for( unsigned i=0;i<m_fabs.size();++i )
    {
        if( m_fabs[i] )
        {
            result += m_fabs[i]->GetRefcount();
        }
    }
    return result;
}


//
// Explicit template instantiations
//
template class BoxLayoutData<float>;
template class BoxLayoutData<double>;
