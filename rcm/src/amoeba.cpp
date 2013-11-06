/* Generic setup to use amoeba(), Nelder & Mead's downhill simplex method. 
 * This version for C++ : we use M's rather than double**'s.
 * 
 * To use this with any function just follow the following instructions: 
 *   1. Define your function in place of likelihood(). 
 *   2. Set the first row of p (in main()) to your initial guess. 
 *   3. Use init_simplex() to fill the rest of p with the other points of 
 *      the initial simplex. 
 *   4. Use init_y() to initialize y, the values of your function at points 
 *      of the initial simplex. 
 *   5. Invoke amoeba() & let 'er rip. 
 *   6. Finds _minimum_ of a function.
 *   7. See main() below for an example of a driver program.
 *   9. amoeba() has trouble with functions whose minimum value is zero, 
 *         so add a constant in those cases.
 *  10. Link to rpnprog.a, nrutil.a, and libm.
*/ 
 
/**** INCLUDES ****/ 
#include <math.h> 
#include <stdio.h> 
#include "rcm.h"

//----- CONSTANTS ----------
const int NMAX=10000; // maximum iterations allowed
const double ALPHA=1.0;
const double BETA=0.5; 
const double GAMMA=2.0; 
 
/**** LOCAL FUNCTION DECLARATIONS ****/ 
static double amotry( Md& p, Vd& y, Vd& psum, int ndim, 
  double (*funk)( const Vd&, const void*), 
  int ihi, int& n_iter, double fac, const void* data) ; 
static void show_status( const Vd& y, const Md& p, int ndim ); 
 
/**** FUNCTION DEFINITIONS ****/ 

/** Given p[0] the initial guess, fills the rest of p with other sides 
 *  of a simplex of size scale. 
*/
void init_simplex( Md& p, int dimension, double scale ) 
{ 
     if( scale == -99 ) {
        // default value; scale will be set to 1/10 mean of guess.
        scale = (sum( fabs( p ) ))[0] / ( 10*p.cols());
        if( scale == 0 )
            rcmError( "Don\'t set init guess to exactly zero\n" );
    }

    for( int i=1;i<dimension+1;i++ ) 
        for( int j=0;j<dimension;j++ ) 
            if( i == j+1 ) 
                p[i][j] = p[0][j] + scale; 
            else 
                p[i][j] = p[0][j]; 
 
} // init_simplex()
//=========================================================================

/** Initialize y to the function values at the points of the simplex p. */
void init_y( Vd& y, const Md& p, int dimension, const void* data, 
  double (*likelihood)( const Vd&, const void* ) ) 
{ 


    for( int i=0; i<dimension+1; i++ )
    {
        V<int> subset = V<int>(i);
        subset.setOrientation(vertical);
        y[i] = likelihood( take(p,subset), data );   // takes i-th row.
    }

} // init_y()
//=========================================================================

/** p is [0..ndim][0..ndim-1] : its rows are vectors which are the 
 *  vertices of the starting simplex.  *y is [0..ndim] is pre-initialized 
 *  values of funk evaluated at the ndim+1 rows of **p.  nfunk will be set 
 *  to the number of funk evaluations taken. 
*/
void amoeba( Md& p, Vd& y, int ndim, double ftol, 
  double (*funk)( const Vd&, const void* ), int &n_iter, const void* data, 
  int quiet ) 
{ 
    int i,j,ilo,ihi,inhi,mpts=ndim+1; 
    double ytry,ysave,sum,rtol;
 
        
    Vd psum(ndim,horizontal);
    for (j=0;j<ndim;j++) { 
        for (i=0,sum=0.0;i<mpts;i++) 
            sum += p[i][j]; 
        psum[j]=sum;
    } 

    for (;;) { 
    
        ilo=0; 
        ihi = y[0]>y[1] ? (inhi=1,0) : (inhi=0,1); 
        for (i=0;i<mpts;i++) { 
            if (y[i] < y[ilo]) ilo=i; 
            if (y[i] > y[ihi]) { 
                inhi=ihi; 
                ihi=i; 
            } else if (y[i] > y[inhi]) 
                if (i != ihi) inhi=i; 
        } 
        if( fabs(y[ihi])+fabs(y[ilo])==0 ) 
             printf("amoeba: \nyihi+yilo=0"); 
        rtol=2.0*fabs(y[ihi]-y[ilo]); // /(fabs(y[ihi])+fabs(y[ilo])); 
 
        if( ( n_iter%1 == 0 ) && ( !quiet) ) { 
              printf("\n amoeba: n_iter = %d", n_iter ); 
              printf("amoeba:  y= %6.5g; p=",  y[0] );
              for( int k=0;k<p.cols();k++ ) printf( " %6.5g ", p[0][k] );
              printf( "amoeba: \n" );
        } 
 
        if (rtol < ftol) break; 
        if (n_iter >= NMAX) {
            printf("\namoeba: Too many iterations in AMOEBA, returning anyway...\n"); 
            break;
        }

        ytry=amotry(p,y,psum,ndim,funk,ihi,n_iter,-ALPHA,data); 
        if (ytry <= y[ilo]) 
            ytry=amotry(p,y,psum,ndim,funk,ihi,n_iter,GAMMA,data); 
        else if (ytry >= y[inhi]) { 
            ysave=y[ihi]; 
            ytry=amotry(p,y,psum,ndim,funk,ihi,n_iter,BETA,data); 
            if (ytry >= ysave) { 
                for (i=0;i<mpts;i++) { 
                    if (i != ilo) { 
                        for (j=0;j<ndim;j++) { 
                            psum[j]=0.5*(p[i][j]+p[ilo][j]); 
                            p[i][j]=psum[j]; 
                        } 
                        y[i]=(*funk)(psum,data); 
                    } 
                } 
                n_iter += ndim; 
 
                for (j=0;j<ndim;j++) { 
                    for (i=0,sum=0.0;i<mpts;i++) 
                        sum += p[i][j]; 
                    psum[j]=sum;
                } 
 
            } 
        } 
    } 
 
    if( !quiet ) {
       printf("\namoeba: n_iter=%d ", n_iter); 
       show_status( y,p,ndim ); 
    }
/*
    printf("\namoeba: amoeba value at nodes = "); 
    for(j=0;j<ndim+1;j++) 
         printf("amoeba: %f\n", y[j][0] ); 
    printf(  "amoeba: p=%6.5g %6.5g\n", p[0][0], p[0][1] );
*/
} // amoeba()
//-----------------------------------------------------------------------
 
static double amotry( Md& p, Vd& y, Vd& psum, int ndim, 
  double (*funk)( const Vd&, const void* ), 
  int ihi,int& n_iter, double fac, const void* data) 
{ 
    int j; 
    double fac1,fac2,ytry;
 
    Vd ptry(ndim,horizontal); 
    fac1=(1.0-fac)/ndim; 
    fac2=fac1-fac; 
    for (j=0;j<ndim;j++) ptry[j]=psum[j]*fac1-p[ihi][j]*fac2; 
    ytry=(*funk)(ptry,data); 
    n_iter++; 
//  printf("\namoeba: %d ", n_iter); 
//  printf("\namoeba:  y= %f, x=%f,%f",ytry,ptry[1],ptry[2]); 
  
    if (ytry < y[ihi]) { 
        y[ihi]=ytry; 
        for (j=0;j<ndim;j++) { 
            psum[j] += ptry[j]-p[ihi][j]; 
            p[ihi][j]=ptry[j]; 
        } 
    } 

    return ytry; 
} // amotry()
 
//=========================================================================
 
static void show_status( const Vd& y, const Md& p, int ndim ) 
{ 
  // Upon kbhit, show where we're at, then continue. 
 
  // to the console... 
  printf("\namoeba: Here's y:\n"); 
  for( int i=0;i<ndim+1;i++ ) 
      printf("amoeba: %f, ", y[i] ); 
 
  printf("\namoeba: Here are the first two rows of p:\n"); 
  for( int i=0;i<2;i++ ) { 
      for( int j=0;j<ndim;j++ ) 
          printf("amoeba: %f, ", p[i][j]); 
      printf("\namoeba: "); 
  } 
 
  // to outfile...
  printf( "\namoeba: Here's y:\n"); 
  for( int i=0;i<ndim+1;i++ ) 
      fprintf( stdout, "amoeba: %f, ", y[i] ); 
 
  printf( "\namoeba: Here are the first two rows of p:\n"); 
  for( int i=0;i<2;i++ ) { 
      for( int j=0;j<ndim;j++ ) 
          printf( "amoeba: %f, ", p[i][j]); 
      printf( "\n"); 
  } 
 
} // show_status() 
//=========================================================================


