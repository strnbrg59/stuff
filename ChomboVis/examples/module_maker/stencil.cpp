//
// Goes with stencil.py.
//

#include "stencil.h"
#include <iostream>
#include <cassert>
using std::vector;
using std::cerr;

Array2D::Array2D( double * fab, vector<int> dims )
  : m_rep( fab), m_dims( dims )
{
}

inline double &
Array2D::operator()( int i, int j )
{
    assert( this->InBounds( i,j ) );
    return m_rep[ j*m_dims[0] + i ];
}


inline const double &
Array2D::operator()( int i, int j ) const
{
    assert( this->InBounds( i,j ) );
    return m_rep[ j*m_dims[0] + i ];
}


inline bool
Array2D::InBounds( int i, int j ) const
{
    return ( i >= 0 ) && ( i < m_dims[0] )
        && ( j >= 0 ) && ( j < m_dims[1] );
}    


void ApplyStencil( int i, int j, // coords of a_dataFrom element
                   const Array2D & a_dataFrom, Array2D & a_dataTo,
                   const vector< vector< double > > & weights )
{
    assert( a_dataFrom.GetDims() == a_dataTo.GetDims() );
    assert( weights.size() > 0 );
    assert( weights.size() == weights[0].size() ); // Must be square

    int rank( weights.size() );
    int r2 = rank/2;
    a_dataTo(i,j) = 0.0;

    int nValid = 0;
    for( int ii=0; ii<rank;++ii )
    {
        for( int jj=0;jj<rank;++jj )
        {
            if( a_dataFrom.InBounds(i-r2+ii, j-r2+jj) )
            {
                a_dataTo(i,j) += weights[ii][jj]
                              *  a_dataFrom(i-r2+ii, j-r2+jj);
                ++nValid;
            }
        }
    }
    a_dataTo(i,j) *= double(nValid)/(rank*rank);
}
              

int F( vector<int> a_dims, double * a_fabFrom, double * a_fabTo )
{
    assert( a_dims.size() == SPACEDIM );

    //
    // Wrap the fabs in something that behaves like a 2D array.
    //
    Array2D fromArray( a_fabFrom, a_dims );    
    Array2D toArray( a_fabTo, a_dims );

    //
    // Define the weights.
    //
    const int rank(3);
    double w[rank][rank] = { {0.1, 0.1, 0.1},
                             {0.1, 0.2, 0.1},
                             {0.1, 0.1, 0.1} };
/*
    const int rank(5);
    double w[rank][rank] = { {0.1, 0.1, 0.1, 0.1, 0.1},
                             {0.1, 0.1, 0.1, 0.1, 0.1},
                             {0.1, 0.1, 0.1, 0.1, 0.1},
                             {0.1, 0.1, 0.1, 0.1, 0.1},
                             {0.1, 0.1, 0.1, 0.1, 0.1}};
*/

    vector< vector< double > > weights( rank );
    for( int m=0;m<rank; ++m )
    {
        weights[m] = vector<double>( w[m], w[m]+rank );
    }

    for( int i=0; i<a_dims[0]; ++i )
    {
        for( int j=0; j<a_dims[1]; ++j )
        {
            ApplyStencil( i, j, fromArray, toArray, weights );
        }
    }

    //
    // toArray is now modified.  Select it in ChomboVis (if it's already
    // selected, then deselect it, then select it).
    //

    return 0;
}
