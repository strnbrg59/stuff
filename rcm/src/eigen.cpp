// eigen.cpp : eigenvalues and eigen vectors of a real symmetric matrix.
// Returns vects in columns, with eigvals on top row.

#include <math.h>
#include "rcm.h"

// LOCAL FUNCTION DECLARATIONS 
static void tred2( Md&a, int n, Md& d, Md& e);
static void tqli( Md& d, Md& e, int n, Md& z);


Md eigen( const Md& x) {
// based on driver for tqli.c, on p.145 of num recip example book.

  int xr=x.rows(), xc=x.cols();
  if ( xr!=xc ) 
	rcmError("eigen: accepts only square matrices." );
  if( !isSymmetric(x) )
	rcmError("eigen: accepts only symmetric matrices." );
	
  Md d(xr+1,2), e(xr+1,2), f(xr+1,2), a( countFrom1(x));

  tred2( a, xr, d, e);   
  // reduces input to tridiagonal form.

  tqli( d, e, xr, a);  
  // finds eigensystem by ql method 

  Md eigenvects( countFrom0(a));
  Md eigenvals( transp(countFrom0(d)));
  Md result( catrows( eigenvals, eigenvects ));

  return result;
} // eigen()
//----------------------------------
void tred2( Md& a, int n, Md& d, Md& e )
{
	int l,k,j,i;
	double scale,hh,h,g,f;

	for (i=n;i>=2;i--) {
		l=i-1;
		h=scale=0.0;
		if (l > 1) {
			for (k=1;k<=l;k++)
				scale += fabs(a[i][k]);
			if (scale == 0.0)

				e[i][1]=a[i][l];
			else {
				for (k=1;k<=l;k++) {
					a[i][k] /= scale;
					h += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = f>0 ? -sqrt(h) : sqrt(h);
				e[i][1]=scale*g;
				h -= f*g;
				a[i][l]=f-g;
				f=0.0;
				for (j=1;j<=l;j++) {
				// Next statement can be omitted if eigenvectors not wanted 
					a[j][i]=a[i][j]/h;
					g=0.0;
					for (k=1;k<=j;k++)
						g += a[j][k]*a[i][k];
					for (k=j+1;k<=l;k++)
						g += a[k][j]*a[i][k];
					e[j][1]=g/h;
					f += e[j][1]*a[i][j];
				}
				hh=f/(h+h);
				for (j=1;j<=l;j++) {
					f=a[i][j];
					e[j][1]=g=e[j][1]-hh*f;
					for (k=1;k<=j;k++)
                                          a[j][k] -= (f*e[k][1]+g*a[i][k]);
				}
			}
		} else
			e[i][1]=a[i][l];
		d[i][1]=h;
	}
	// Next statement can be omitted if eigenvectors not wanted 
	d[1][1]=0.0;
	e[1][1]=0.0;
	// Contents of this loop can be omitted if eigenvectors not
	//		wanted except for statement d[i]=a[i][i]; 
	for (i=1;i<=n;i++) {
		l=i-1;
		if (d[i][1]) {
			for (j=1;j<=l;j++) {
				g=0.0;
				for (k=1;k<=l;k++)
					g += a[i][k]*a[k][j];
				for (k=1;k<=l;k++)
					a[k][j] -= g*a[k][i];
			}
		}
		d[i][1]=a[i][i];
		a[i][i]=1.0;
		for (j=1;j<=l;j++) a[j][i]=a[i][j]=0.0;
	}
} // tred2()
//-----------------------------------

#define SIGN(a,b) ((b)<0 ? -fabs(a) : fabs(a))

void tqli( Md& d, Md& e, int n, Md& z )
{
	int m,l,iter,i,k;
	double s,r,p,g,f,dd,c,b;

	for (i=2;i<=n;i++) e[i-1][1]=e[i][1];
	e[n][1]=0.0;
	for (l=1;l<=n;l++) {
		iter=0;
		do {
			for (m=l;m<=n-1;m++) {
				dd=fabs(d[m][1])+fabs(d[m+1][1]);
				if (fabs(e[m][1])+dd == dd) break;
			}
			if (m != l) {
				if (iter++ == 30) rcmError("Too many iterations in TQLI");
				g=(d[l+1][1]-d[l][1])/(2.0*e[l][1]);
				r=sqrt((g*g)+1.0);
				g=d[m][1]-d[l][1]+e[l][1]/(g+SIGN(r,g));
				s=c=1.0;
				p=0.0;
				for (i=m-1;i>=l;i--) {
					f=s*e[i][1];
					b=c*e[i][1];
					if (fabs(f) >= fabs(g)) {
						c=g/f;
						r=sqrt((c*c)+1.0);
						e[i+1][1]=f*r;
						c *= (s=1.0/r);
					} else {
						s=f/g;
						r=sqrt((s*s)+1.0);
						e[i+1][1]=g*r;
						s *= (c=1.0/r);
					}
					g=d[i+1][1]-p;
					r=(d[i][1]-g)*s+2.0*c*b;
					p=s*r;
					d[i+1][1]=g+p;
					g=c*r-b;
					// Next loop can be omitted if eigenvectors not wanted 
					for (k=1;k<=n;k++) {
						f=z[k][i+1];
						z[k][i+1]=s*z[k][i]+c*f;
						z[k][i]=c*z[k][i]-s*f;
					}
				}
				d[l][1]=d[l][1]-p;
				e[l][1]=g;
				e[m][1]=0.0;
			}
		} while (m != l);
	}
} // tqli() 

#undef SIGN
