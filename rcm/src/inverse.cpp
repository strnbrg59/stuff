#ifndef INCLUDE_INVERSE_CC
#define INCLUDE_INVERSE_CC

#include "rcm.h"

static void gaussj( Md& a, int n, Md& b, int m);

Md inverse(const Md& mat)
{
    if( mat.rows() == 1 && mat.cols() == 1 )
        return Md(1.0/mat[0][0]);
    else
    {
        Md result( mat );
        Md dummy( mat.rows(), 1 );
        dummy = dummy + Md(1.0);
        gaussj( result, result.rows(), dummy, 1 );
        return result;
    }
}

#include <math.h>

#define SWAP(a,b) {double temp=(a);(a)=(b);(b)=temp;}

void gaussj( Md& a, int n, Md& b, int m) {
	int icol,irow,k,l,ll;
	double big,dum,pivinv;

	Md indxc(1,n),
       indxr(1,n),
       ipiv(1,n);

	for (int j=0;j<n;j++) ipiv[0][j]=0;
	for (int i=0;i<n;i++) {
		big=0.0;
		for (int j=0;j<n;j++)
			if (ipiv[0][j] != 1)
				for (int k=0;k<n;k++) {
					if (ipiv[0][k] == 0) {
						if (fabs(a[j][k]) >= big) {
							big=fabs(a[j][k]);
							irow=j;
							icol=k;
						}
					} 
                    else if (ipiv[0][k] > 1) 
                    {
                        rcmError("GAUSSJ: Singular Matrix-1");
                    }
				}
		++(ipiv[0][icol]);
		if (irow != icol) {
			for (l=0;l<n;l++) SWAP(a[irow][l],a[icol][l])
			for (l=0;l<m;l++) SWAP(b[irow][l],b[icol][l])
		}
		indxr[0][i]=irow;
		indxc[0][i]=icol;
		if (a[icol][icol] == 0.0) 
                {
                    rcmError("GAUSSJ: Singular Matrix-2");
                }
		pivinv=1.0/a[icol][icol];
		a[icol][icol]=1.0;
		for (l=0;l<n;l++) a[icol][l] *= pivinv;
		for (l=0;l<m;l++) b[icol][l] *= pivinv;
		for (ll=0;ll<n;ll++)
			if (ll != icol) {
				dum=a[ll][icol];
				a[ll][icol]=0.0;
				for (l=0;l<n;l++) a[ll][l] -= a[icol][l]*dum;
				for (l=0;l<m;l++) b[ll][l] -= b[icol][l]*dum;
			}
	}
	for (l=n-1;l>=0;l--) {
		if (indxr[0][l] != indxc[0][l])
			for (k=0;k<n;k++)
				SWAP(a[k][(int)indxr[0][l]],
                                     a[k][(int)indxc[0][l]]);
	}
}

#undef SWAP


#endif
