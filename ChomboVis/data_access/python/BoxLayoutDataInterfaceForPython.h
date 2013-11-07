#ifndef INCLUDED_BOXLAYOUTDATAINTERFACEFORPYTHON_H
#define INCLUDED_BOXLAYOUTDATAINTERFACEFORPYTHON_H

#include "../../utils/StatusCodes.h"
#include <boost/shared_ptr.hpp>
#include <vector>
using std::vector;
class Box;

/** Abstract base class for template class BoxLayoutData.
 *  Serves two purposes:
 *    1. Lets us call the bld pointer polymorphically, in 
 *       python/bldmodule.cpp.
 *    2. Defines, explicitly, the precise subset of BoxLayoutData
 *       functionality that we want to expose at the Python layer (and that is
 *       a fairly restricted subset).
*/
class BoxLayoutDataInterfaceForPython
{
  public:
    virtual ~BoxLayoutDataInterfaceForPython() { }
    virtual Status UnitTest( int level, int component ) = 0;
    virtual boost::shared_ptr< BoxLayoutDataInterfaceForPython > Clone() const=0;

    virtual vector<vector<vector<int> > > GetBoxLayoutAsNestedVectors() const=0;
    virtual double GetDatum( int boxNum, int i, int j, int k=0 ) const = 0;
    virtual Status SetDatum( double x, int boxNum, int i, int j, int k=0 ) = 0;
    virtual vector<double> GetDx() const = 0;
    virtual vector<double> GetOrigin() const = 0;

    virtual vector<vector<vector<double> > > GetFArrayAsVectorMatrix(
        int boxNum ) const = 0;
    virtual int GetFArray( int boxNum ) const = 0;

    virtual Status Clamp( vector<int> boxNums ) = 0;
    virtual Status ClampToGeneralBox( Box const & ) = 0;
    virtual Status CropToGeneralBox( Box const & ) = 0;
    virtual Status UnClamp() = 0;

    // In-place transformations.  From Python, if want to apply these to just
    // one box, then use Clamp().
    virtual Status Apply_fabs( int boxNum=-1 ) = 0;
    virtual Status Apply_log(  int boxNum=-1 ) = 0;
    virtual Status Apply_exp(  int boxNum=-1 ) = 0;
    virtual Status Apply_pow(  double x, int boxNum=-1 ) = 0;
    virtual Status Apply_powBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython > that,
        int boxNum = -1 ) = 0;
    virtual Status Apply_sin( int boxNum=-1 ) = 0;
    virtual Status Apply_cos( int boxNum=-1 ) = 0;
    virtual Status Apply_tan( int boxNum=-1 ) = 0;
    virtual Status Apply_asin( int boxNum=-1 ) = 0;
    virtual Status Apply_acos( int boxNum=-1 ) = 0;
    virtual Status Apply_atan( int boxNum=-1 ) = 0;
    virtual Status Apply_sinh( int boxNum=-1 ) = 0;
    virtual Status Apply_cosh( int boxNum=-1 ) = 0;
    virtual Status Apply_tanh( int boxNum=-1 ) = 0;
    virtual Status Apply_asinh( int boxNum=-1 ) = 0;
    virtual Status Apply_acosh( int boxNum=-1 ) = 0;
    virtual Status Apply_atanh( int boxNum=-1 ) = 0;

    virtual Status Apply_plusEquals( double x, int boxNum=-1) = 0;
    virtual Status Apply_plusEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython >,
        int boxNum=-1) = 0;
    virtual Status Apply_minusEquals( double x, int boxNum=-1) = 0;
    virtual Status Apply_minusEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython >,
        int boxNum=-1) = 0;
    virtual Status Apply_timesEquals( double x, int boxNum=-1) = 0;
    virtual Status Apply_timesEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython >,
        int boxNum=-1) = 0;
    virtual Status Apply_divideEquals( double x, int boxNum=-1) = 0;
    virtual Status Apply_divideEqualsBLD(
        boost::shared_ptr< BoxLayoutDataInterfaceForPython >,
        int boxNum=-1) = 0;

    // Summary statistics.
    //
    virtual double Min( int boxNum=-1 ) const = 0;
    virtual double Max( int boxNum=-1 ) const = 0;
    virtual double Sum( int boxNum=-1 ) const = 0;
    virtual double SumOfSquares( int boxNum=-1 ) const = 0;
    virtual int GetNumBoxes() const = 0;
    virtual int GetNumCells( int boxNum=-1 ) const = 0;

  protected:
    BoxLayoutDataInterfaceForPython() { };

  private:
    BoxLayoutDataInterfaceForPython(
        BoxLayoutDataInterfaceForPython const & );
    BoxLayoutDataInterfaceForPython & operator=(
        BoxLayoutDataInterfaceForPython & );
};

#endif // INCLUDED_BOXLAYOUTDATAINTERFACEFORPYTHON_H
