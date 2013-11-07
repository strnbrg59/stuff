#include <cmath>

static bool IsNormalNumber( double x,
                            double loBound=-HUGE_VAL,
                            double hiBound= HUGE_VAL )
{
    bool result( finite(x) && (x>=loBound) && (x<=hiBound) );
#ifdef DO_DEMO
    cout << "IsNormalNumber(" << x << ")=" << result << endl;
#endif
    return result;
}

// Functor to ignore NaN's, inf's and (optionally) anything outside a range
// of finite numbers, when finding max & min.  Can't pass the
// same one to both std::min and std::max, hence the pair of functors here.
struct NormalNumberComparatorForMin
{
    NormalNumberComparatorForMin( double loBound=-HUGE_VAL,
                                  double hiBound= HUGE_VAL )
      : m_loBound(loBound),
        m_hiBound(hiBound)
    { }
    bool operator()( double a, double b ) 
    {
        if      ( ! IsNormalNumber(a,m_loBound,m_hiBound) ) return false;
        else if ( ! IsNormalNumber(b,m_loBound,m_hiBound) ) return true;
        else                            return a<b;
        // If a or b are inf/-inf, the built-in operator< does the right thing.
    }

  private:
    double m_loBound;
    double m_hiBound;
};

struct NormalNumberComparatorForMax
{
    NormalNumberComparatorForMax( double loBound=-HUGE_VAL,
                                  double hiBound= HUGE_VAL )
      : m_loBound(loBound),
        m_hiBound(hiBound)
    { }
    bool operator()( double a, double b ) 
    {
        if      ( ! IsNormalNumber(a,m_loBound,m_hiBound) ) return true;
        else if ( ! IsNormalNumber(b,m_loBound,m_hiBound) ) return false;
        else                            return a<b;
        // If a or b are inf/-inf, the built-in operator< does the right thing.
    }

  private:
    double m_loBound;
    double m_hiBound;
};

