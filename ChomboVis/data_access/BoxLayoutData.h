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

#ifndef INCLUDED_BOXLAYOUTDATA_H
#define INCLUDED_BOXLAYOUTDATA_H

#include "FAB.h"
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include "BoxLayout.h"
#include "../utils/StatusCodes.h" // Wrapped funcs can't return void.
#include "./python/BoxLayoutDataInterfaceForPython.h"

template<class REAL_T> class BoxLayoutData
  : public BoxLayoutDataInterfaceForPython
{
  public:
    // Memory management
    ~BoxLayoutData();
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > Clone() const;
    static boost::shared_ptr< BoxLayoutData<REAL_T> > * NewVector(
        int numComponents,
        BoxLayout<REAL_T> const &,
        int level );
    static void GrowVector(
        boost::shared_ptr< BoxLayoutData<REAL_T> > * * currentBlds,
        int numComponents );
    static void ShrinkVectorByOne(
        boost::shared_ptr< BoxLayoutData<REAL_T> > * * currentBlds,
        int numComponents );
    int PinFAB(   boost::intrusive_ptr<FAB<REAL_T> > const & );
    int UnpinFAB( boost::intrusive_ptr<FAB<REAL_T> > const & );
    void FreeUnusedFABs();
    void PrintFabUseCounts() const;
    int  GetTotalFabUseCount() const;


    // Getters
    BoxLayout<REAL_T> const & GetBoxLayout() const;
    vector<vector<vector<int> > > GetBoxLayoutAsNestedVectors() const;
    boost::intrusive_ptr<FAB<REAL_T> > GetFAB( int boxNum ) const;
    vector<vector<vector<double> > > GetFArrayAsVectorMatrix(
        int boxNum ) const;
    int GetFArray( int boxNum ) const;
    bool FABsAreFilled() const;
    double GetDatum( int boxNum, int i, int j, int k=0 ) const;
    int GetNumBoxes() const { return m_boxLayout.size(); }
    vector<double> GetDx() const;
    vector<double> GetOrigin() const;
    bool IsInitialized() const { return m_initialized; }

    // Setters
    void SetFAB( int boxNum, boost::intrusive_ptr<FAB<REAL_T> > );
    Status SetDatum( double x, int boxNum, int i, int j, int k=0 );
    Status Clamp( vector<int> boxNums );
    Status Clamp( int boxNum );
    Status ClampToGeneralBox( Box const & clampingBox );// Any rectangular prism
    Status CropToGeneralBox(  Box const & croppingBox );// Any rectangular prism
    Status UnClamp();
    void SetLevel( int );


    //
    // In-place transformations
    //
    // (The return values for these used to be BoxLayoutData*.  But we never
    // used that (we achieve chaining by returning "self" from the corresponding
    // Python functions).  The real reason for replacing BoxLayoutData* with
    // Status is that returning a pointer doesn't work well with
    // boost::shared_ptr.
    Status Apply_fabs( int boxNum = -1 );
    Status Apply_log(  int boxNum = -1 );
    Status Apply_exp(  int boxNum = -1 );
    Status Apply_sin(  int boxNum = -1 );
    Status Apply_cos(  int boxNum = -1 );
    Status Apply_tan(  int boxNum = -1 );
    Status Apply_asin(  int boxNum = -1 );
    Status Apply_acos(  int boxNum = -1 );
    Status Apply_atan(  int boxNum = -1 );
    Status Apply_sinh(  int boxNum = -1 );
    Status Apply_cosh(  int boxNum = -1 );
    Status Apply_tanh(  int boxNum = -1 );
    Status Apply_asinh(  int boxNum = -1 );
    Status Apply_acosh(  int boxNum = -1 );
    Status Apply_atanh(  int boxNum = -1 );

    Status Apply_pow(  double x, int boxNum = -1 );
    Status Apply_plusEquals(double x, int boxNum=-1);
    Status Apply_minusEquals(double x,int boxNum=-1);
    Status Apply_timesEquals(double x,int boxNum=-1);
    Status Apply_divideEquals(double x,int boxNum=-1);
    Status Apply_powBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
        int boxNum = -1 );
    Status Apply_plusEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
        int boxNum=-1);
    Status Apply_minusEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
        int boxNum=-1);
    Status Apply_timesEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
        int boxNum=-1);
    Status Apply_divideEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
        int boxNum=-1);


    //
    // Summary statistics
    //
    double Min( int boxNum = -1 ) const;
    double Max( int boxNum = -1 ) const;
    double Sum( int boxNum = -1 ) const;
    double SumOfSquares( int boxNum = -1 ) const;
    int    GetNumCells( int boxNum = -1 ) const;

    Status UnitTest( int level, int component );

  private:
    BoxLayoutData();  // implemented in hImpl file
    BoxLayoutData<REAL_T>( BoxLayoutData<REAL_T> const & ); // not implemented
    BoxLayoutData<REAL_T> &
        operator=( BoxLayoutData<REAL_T> const & ); // not implemented
    void Init( BoxLayout<REAL_T> const &, int level, int component );

    // In-place transformations -- implementation details
    Status  Apply_niladicFabOperator(
        double (*pf)(double), int boxNum = -1 );
    Status  Apply_monadicFabOperator(
        double (*pf)(double,double), REAL_T x, int boxNum = -1 );
    Status  Apply_monadicCorrespondingFabOperator(
        double (*pf)(double,double),
        boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
        int boxNum = -1 );

    template<class U> U AccumOverBoxes( U (FAB<REAL_T>::*pbf)() const,
        U (*accumFunc)(U,U), U init, int boxNum =-1 ) const;

    // Utilities.
    void ElementPrinter( int level, int component, int box, int coords[3],
        Intvect locorner, REAL_T datum ) const;

    //
    // Data.
    //
    std::vector< boost::intrusive_ptr<FAB<REAL_T> > > m_fabs;      // by box
    bool m_initialized;
    BoxLayout<REAL_T> m_boxLayout;
    vector<int> m_clampingBoxNums;
    int m_level;
    int m_component;
    FAB_Pinner< boost::intrusive_ptr< FAB<REAL_T> > > m_pinnedFabs;

    friend class Dummy; // Does nothing.  Just to avoid compiler warning
                        // about "no public ctors and no friends".
};


template<typename T> ostream &
operator<<( ostream & ost, vector<T> const & v )
{
    ost << "< ";
    for( unsigned i=0;i<v.size();++i )
    {
        ost << v[i] << " ";
    }
    ost << "> ";
    return ost;
}


/** Does nothing.  For avoiding a compiler warning. */
class Dummy { };

#endif // INCLUDED_BOXLAYOUTDATA_H
