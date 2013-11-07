#include <vector>

#define SPACEDIM 2

/**
    Arg a_fabFrom is the fortran-order array we apply the stencil to.  But we
    put the result into a_fabTo; we don't modify a_fabFrom.

    Arg a_dims is a_fabFrom's cell-count (along the x, y and [for 3D] z axes).
*/
int F( std::vector<int> a_dims, double * a_fabFrom, double * a_fabTo );


/** Facade in front of a Fortran-order 1D array, making it feel like a 2D
 *  C array.
*/
class Array2D
{
public:
    Array2D( double *, std::vector<int> dims );

    double & operator()( int, int );
    const double & operator()( int, int ) const;

    /** Returns number of cells along i and j directions. */
    std::vector<int> GetDims() const { return m_dims; }

    /** Returns true if index (i,j) is in bounds. */
    bool InBounds( int, int ) const;
private:
    double * m_rep;
    std::vector<int> m_dims;
};


/**
 *  At boundaries, applies only in-bounds weights and adjusts weighted average
 *  for number of in-bounds weights.
 *
 *  Stencil filter is assumed square and of odd rank, with the central
 *  value being the weight that's applied to a_dataFrom(ii,jj).
*/
void ApplyStencil( int i, int j, // coords of a_dataFrom element
                   const Array2D & a_dataFrom, Array2D & a_dataTo,
                   const std::vector< std::vector< double > > & weights );

