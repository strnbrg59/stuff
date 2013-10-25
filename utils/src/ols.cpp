#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::swap;
#include "boost/multi_array.hpp"
#include <cassert>
#include <cmath>
#include "ols.hpp"
#include "utils.hpp"

/* Test stub */
#ifdef UNITTEST
int 
main () {
    int n(4000);
    int k(3);

    // Left-hand side:
    cvector lhs(boost::extents[n]);
    for( int i = 0; i != n; ++i )
    {
        lhs[i] = 0.39 + i + (random()%100 - 49.5)/100.0;
    }

    // Right-hand side:
    matrix rhs(boost::extents[n][k]);
    for( int i = 0; i != n; ++i )
    {
        rhs[i][0] = 1.0;
        rhs[i][1] = i;
        rhs[i][2] = random()%2;
    }

    cvector beta( Ols( lhs, rhs ) );
    cout << "beta:" << endl;
    for( unsigned int i=0;i<beta.shape()[0];++i )
    {
        cout << "  " << beta[i] << endl;
    }

    cout << "resid:\n";
    cvector yhat(MatCvectMult(rhs,beta));
    double resid_min=1E309;
    double resid_max=-1E309;
    double resid_sum = 0;
    for(int i=0;i<n;++i)
    {
        double resid = lhs[i] - yhat[i];
        resid_sum += resid;
        resid_min = resid < resid_min ? resid : resid_min;
        resid_max = resid > resid_max ? resid : resid_max;
    }
    cout << "resid_sum=" << resid_sum << '\n';
    cout << "resid_min=" << resid_min << '\n';
    cout << "resid_max=" << resid_max << '\n';

    matrix xpx(MatMatMult(MatTranspose(rhs),rhs));
    cout << "xpx=\n" << xpx;
    matrix xpxinv(MatInverse(xpx));
    matrix xpxinvxpx(MatMatMult(xpxinv,xpx));
    cout << "xpxinvxpx=\n" << xpxinvxpx;

    return 0;
}
#endif

/** Ordinary least squares regression */
cvector Ols( cvector lhs, matrix rhs )
{
    int n( lhs.shape()[0] );
    int k( rhs.shape()[1] );
    assert( n == int(rhs.shape()[0]) );

    /*    
    // Display input:
    for( int i=0;i<n;++i )
    {
        cout << lhs[i] << "    ";
        for( int j=0;j<k;++j )
        {
            cout << rhs[i][j] << "  ";
        }
        cout << endl;
    }
    */

    matrix xpx( MatMatMult( MatTranspose( rhs ), rhs ) );
    matrix xpx_inv( MatInverse( xpx ) );
    cvector xpy( MatCvectMult( MatTranspose( rhs ), lhs ) );
    cvector beta( MatCvectMult( xpx_inv, xpy ) );
    return beta;
}


matrix MatMatMult( matrix const & mat1, matrix const & mat2 )
{
    int rows1( mat1.shape()[0] );
    int cols1( mat1.shape()[1] );
    int cols2( mat2.shape()[1] );
    assert( cols1 == mat2.shape()[0] );

    matrix result(boost::extents[rows1][cols2]);
    for( int i=0;i<rows1;++i )
    {
        for( int j=0;j<cols2;++j )
        {
            result[i][j] = 0;
            for( int k=0;k<cols1;++k )
            {
                result[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }

    return result;
}


cvector MatCvectMult( matrix const & mat, cvector const & cvect )
{
    int rows1( mat.shape()[0] );
    int cols1( mat.shape()[1] );
    assert( cols1 == cvect.shape()[0] );

    cvector result(boost::extents[rows1]);
    for( int i=0;i<rows1;++i )
    {
        result[i] = 0;
        for( int k=0;k<cols1;++k )
        {
            result[i] += mat[i][k] * cvect[k];
        }
    }

    return result;
}


cvector CvectMinus( cvector const & v1, cvector const & v2 )
{
    int rows( v1.shape()[0] );
    assert( rows == v2.shape()[0] );
    cvector result(boost::extents[rows]);
    for( int i=0;i<rows;++i )
    {
        result[i] = v1[i] - v2[i];
    }
    return result;
}


matrix MatTranspose( matrix const & mat )
{
    int rows( mat.shape()[0] );
    int cols( mat.shape()[1] );
    matrix result(boost::extents[cols][rows]);
    for( int i=0;i<rows;++i )
    {
        for( int j=0;j<cols;++j )
        {
            result[j][i] = mat[i][j];
        }
    }

    return result;
}


/** Arg must be square */
matrix MatInverse( matrix const & mat )
{
    int rows( mat.shape()[0] );
    int cols( mat.shape()[1] );
    assert( rows == cols );

    if( (rows == 1) && (cols == 1) )
    {
        matrix result(boost::extents[1][1]);
        result[0][0] = 1.0/mat[0][0];
        return result;
    }

    matrix result( mat );
    matrix dummy( boost::extents[rows][1] );
    for( int i=0;i<rows;++i ) dummy[i][0] = 1.0;
    gaussj( result, rows, dummy, 1 );
    return result;
}

void MatRound(matrix& mat, double tol)
{
    for(int i=0;i<mat.shape()[0];++i)
    {
        for(int j=0;j<mat.shape()[1];++j)
        {
            mat[i][j] = utils::round(mat[i][j], tol);
        }
    }
}
void CvectRound(cvector& v, double tol)
{
    for(int i=0;i<v.shape()[0];++i)
    {
        v[i] = utils::round(v[i], tol);
    }
}

std::ostream& operator<<(std::ostream& out, matrix const& m)
{
    for(int i=0;i<m.shape()[0];++i)
    {
        for(int j=0;j<m.shape()[1];++j)
        {
            out << m[i][j] << ' ';
        }
        out << '\n';
    }
    return out;
}
std::ostream& operator<<(std::ostream& out, cvector const& v)
{
    for(int i=0;i<v.shape()[0];++i)
    {
        out << v[i] << ' ';
    }
    return out;
}


/** Gauss-Jordan method, from Press, Flannery, Teukolsky & Vetterling,
 *  _Numerical Recipes in C_, Cambridge U. Press, 1988.
*/
#define SWAP(a,b) {double temp=(a);(a)=(b);(b)=temp;}
void gaussj( matrix & a, int n, matrix & b, int m)
{
	int icol,irow,k,l,ll;
	double big,dum,pivinv;

	matrix indxc(boost::extents[1][n]),
           indxr(boost::extents[1][n]),
           ipiv(boost::extents[1][n]);

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
                        cerr << "GAUSSJ: Singular Matrix-1" << endl;
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
                    cerr << "GAUSSJ: Singular Matrix-2" << endl;
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
