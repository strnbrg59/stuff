#include "rcm.h"

/** Variance-covariance matrix of the columns of x. */
Md varcovar(const Md& x)
{
    int xr( x.rows() );
    int xc( x.cols() );

    if( xr == 1 )
    {
        return drho(1,1,Md(0.0));
    }        

    Md ss( matmult_xpy(x,x) );
    Md s( drho( xc, xc, Md(sum(transp(x)))));
    Md result( ss/(xr-1) - s*transp(s)/(xr*(xr-1)) );

    return result;
}

/** Standard deviation by rows. */
Vd sd(const Md& x)
{
    int n = x.cols();
    if( n==1 )
    {
        return Vd(0.0);
    }
    else
    {
        Md mu( mean(x) );
        Md ss( sum(pow(x,2)));
        Md result( pow( ss/(n-1) - mu*mu*n/(n-1), 0.5));
        return Vd(result);
    }
}

double covar( const Vd& x, const Vd& y )
{
    int n = x.size();
#if RCM_DEBUG==1
    if( x.size() != y.size() )
        rcmError( "covar: args not the same length." );
#endif

    double sx=0,sy=0,sxy=0;

    for( int i=0;i<n;i++ )
    {
        sx += x[i];
        sy += y[i];
        sxy += x[i]*y[i];
    }

    double result = (sxy - sx*sy/n)/(n-1);
    return result;
}

double correl( const Vd& x, const Vd& y )
{
    // Deal with orientation issues; sd expects row vectors, but we want
    // this function to give the sensible answer whether x and y are rows
    // or columns.
    double sdx, sdy;
    if( x.getOrientation() == horizontal )
        sdx = sd(x)[0];
    else
        sdx = sd(transp(x))[0];
    if( y.getOrientation() == horizontal )
        sdy = sd(y)[0];
    else
        sdy = sd(transp(y))[0];


    if( (sdx == 0) || (sdy == 0) )
        return 0.0;
    else
        return covar(x,y)/(sdx*sdy);
}

    



