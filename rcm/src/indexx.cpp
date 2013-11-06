#include "rcm.h"

static void indexx(int n, const double *arrin, int *indx); 

V<int> indexx( const Md& x) 
{
    if( x.rows() != 1 )
    {
        rcmError( "indexx(): argument must be a row vector." );
    }

    int xc = x.cols();
    V<int> result(xc, horizontal);

    if( xc == 1 ) {  // special handling; indexx will bomb otherwise
        result[0] = 0;
		return result;
    }

    // prepare arguments for Numerical Recipes indexx function.
    int* indx = new int[xc];
    double* firstrow = new double[xc]; 

    for( int j=0;j<xc;j++ ) firstrow[j] = x[0][j];
    indexx( xc, firstrow-1, indx-1 );  
    // returns indices in indx.  indexx() counts up from 1, hence the -1's.

    // copy columns of x into result, according to indices in indx.
    for( int j=0;j<xc;j++ ) result[j] = indx[ j ] - 1;

  delete[] indx;
  delete[] firstrow;

  return result;
} 
//------------------------

/** Returns indices to sort arrin in ascending order, like APL's grade-up. 
 *  From Numerical Recipes
*/
static void indexx(int n, const double *arrin, int *indx)
{ 
	int l,j,ir,indxt,i;
	double q;

	for (j=1;j<=n;j++) indx[j]=j;
	l=(n >> 1) + 1;
	ir=n;
	for (;;) {
		if (l > 1)
			q=arrin[(indxt=indx[--l])];
		else {
			q=arrin[(indxt=indx[ir])];
			indx[ir]=indx[1];
			if (--ir == 1) {
				indx[1]=indxt;
				return;
			}
		}
		i=l;
		j=l << 1;
		while (j <= ir) {
			if (j < ir && arrin[indx[j]] < arrin[indx[j+1]]) j++;
			if (q < arrin[indx[j]]) {
				indx[i]=indx[j];
				j += (i=j);
			}
			else j=ir+1;
		}
		indx[i]=indxt;
	}
}
