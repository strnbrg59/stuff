#include <math.h>
#include <string>
#include <iostream>
#include "rcm.h"

RcmException::RcmException( string message_ ) : message(message_)
{ }

void RcmException::print()
{
    cerr << message << '\n';
}

/** We'd like to throw an RcmException, but that causes a mysterious
 *  segmentation violation.
*/
void rcmError( string message )
{
    throw RcmException(message);
//  cerr << message << '\n';
}

Md countFrom1( const Md& x ) {
// Copies x to an Md whose elements can be addressed by counting
// subscripts from 1 (rather than 0).  This is the convention in the
// Numerical Recipes in C book.  The alternative--to go through all
// those recipes and change do-loop subscripts--is too hazardous.
// It would be equally dangerous to change the pointers in x itself;
// especially if we decremented the pointer to x itself, the
// Md destructor would get fooled.

	int xr = x.rows(), xc = x.cols();
	Md result( xr+1, xc+1 );
	for( int i=0;i<xr;i++ ) for( int j=0;j<xc;j++ )
		result[i+1][j+1] = x[i][j];

	return result;
} 
//------------------------------

Md countFrom0( const Md& x ) {
// sort-of inverse function of count_from_1()

	int xr = x.rows(), xc = x.cols();
	Md result( xr-1, xc-1 );
	for( int i=0;i<xr-1;i++ ) for( int j=0;j<xc-1;j++ )
		result[i][j] = x[i+1][j+1];

	return result;

}
//-------------------------------

bool isSymmetric( const Md& x )
{
    for( int i=0;i<x.rows();i++ ) for( int j=0;j<x.cols();j++ )
    {
        if( fabs(x[i][j] - x[j][i]) > 0.00000001 ) return false;
    }
    return true;
}
