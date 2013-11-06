#include <math.h>
#include "rcm.h"

// --------------------------------
// Member functions of abstract class Regression
//
Regression::Regression( Vd y, Md x ) :
  yhatIsComputed(false),
  betaIsComputed(false),
  resIsComputed(false),
  r2IsComputed(false),
  tstatsIsComputed(false),
  betaVcvIsComputed(false),
  betaCorrelmatIsComputed(false),
  x_(x),
  y_(y),
  yhat_(0.0),beta_(0.0),res_(0.0),r2_(0.0),tstats_(0.0),
  betaVcv_(0.0),betaCorrelmat_(0.0)
{ }


/** (virtual) destructor.  Doesn't do anything but avoid a compiler
 *  warning.
*/
Regression::~Regression() { }

Md Regression::x() { return x_; }
Vd Regression::y() { return y_; }

Vd Regression::yhat()
{
    if( !yhatIsComputed )
    {
        compute_yhat();        // sets yhat_
        yhatIsComputed = true;
    }
    return yhat_;
}
Vd Regression::beta()
{
    if( !betaIsComputed )
    {
        compute_beta();        // sets beta_
        betaIsComputed = true;
    }
    return beta_;
}
Vd Regression::res()
{
    if( !resIsComputed )
    {
        compute_res();        // sets res_
        resIsComputed = true;
    }
    return res_;
}
double Regression::r2()
{
    if( !r2IsComputed )
    {
        compute_r2();         // sets r2_
        r2IsComputed = true;
    }
    return r2_;
}
Vd Regression::tstats()
{
    if( !tstatsIsComputed )
    {
        compute_tstats();     // sets tstats_
        tstatsIsComputed = true;
    }
    return tstats_;
}
Md Regression::betaVcv()
{
    if( !betaVcvIsComputed )
    {
        compute_betaVcv();   // sets betaVcv_
        betaVcvIsComputed = true;
    }
    return betaVcv_;
}
Md Regression::betaCorrelmat()
{
    if( !betaCorrelmatIsComputed )
    {
        compute_betaCorrelmat(); // sets betaCorrelmat_
        betaCorrelmatIsComputed = true;
    }
    return betaCorrelmat_;
}


Md Regression::displayAll()
{
    Md result( catcols(catcols(beta(),tstats()),betaCorrelmat()) );
    return result;
}
//
// End, member functions of abstract class Regression
//

//
// Member functions of concrete class Ols
//

Ols::Ols( Vd y, Md x ) : Regression(y,x) { }

/** Used by stepwiseRegression, where we pass a ptr to a Regression object
 *  as a way for stepwiseRegression to know what kind of regression model to
 *  use.
 *  In stepwiseRegression, we call clone() only once -- to obtain an initial
 *  ptr.  Thereupon, every time we introduce a new column into the RHS and
 *  need to do a new regression, we call clone(Vd,Md).
*/
Regression* Ols::clone() const
{
    Vd dummyY = transp(iota(2));
    Md dummyX = drho(2,1,Md(1.0));
    return new Ols( dummyY, dummyX );
}

/** See comments for clone() */
Regression* Ols::clone( const Vd& y, const Md& x ) const
{
    return new Ols( y, x );
}


/** Called only if resIsComputed==false */
void Ols::compute_res()
{
    res_ = y_ - yhat();
}

/** Called only if betaIsComputed==false */
void Ols::compute_beta()
{
    Md xpxinv( inverse(matmult_xpy(x_,x_)) );
    Md xpy( matmult_xpy(x_,y_) );

    beta_ = matmult( xpxinv,xpy );
}

/** Called only if yhatIsComputed==false */
void Ols::compute_yhat()
{
    yhat_ = matmult( x_, beta() );
}

/** Called only if r2IsComputed==false */
void Ols::compute_r2()
{
    r2_ = (matmult_xpy(yhat(),yhat()) / matmult_xpy(y_,y_))[0][0];
}

/** Called only if tstatsIsComputed==false */
void Ols::compute_tstats()
{
    Vd sdBeta( pow(transp(diag(betaVcv())),0.5) );
    tstats_ = beta()/sdBeta;
}

/** Called only if betaVcvIsComputed==false */
void Ols::compute_betaVcv()
{
    double sdRes( sd(transp(res()))[0] );
    betaVcv_ =  Md(sdRes) * inverse(matmult_xpy(x_,x_));
}

/** Called only if betaCorrelmatIsComputed==false */
void Ols::compute_betaCorrelmat()
{
    Md vcv( betaVcv() );
    int dim = x_.cols();
    Md result(dim,dim);

    for( int i=0;i<dim;i++ )
    {
        for( int j=0;j<dim;j++ )
        {
            if( vcv[i][i] == 0 || vcv[j][j] == 0 )
            {
                result[i][j] = 0;
            }
            else
            {
                result[i][j] = vcv[i][j]/pow(vcv[i][i]*vcv[j][j],0.5);
            }
        }
    }

    betaCorrelmat_ = result;
}

//
// Member functions of concrete class Tls -- trimmed least squares
//

static double trimmedSquares( const Vd& beta, const void* data );

Tls::Tls( Vd y, Md x, double trimFactor ) : 
Regression(y,x),
m_trimFactor(trimFactor) { }

/** See comments for Ols::clone() */
Regression* Tls::clone() const
{
    Vd dummyY = transp(iota(2));
    Md dummyX = drho(2,1,Md(1.0));
    return new Tls( dummyY, dummyX, 0.05 );
}

/** See comments for Ols::clone() */
Regression* Tls::clone( const Vd& y, const Md& x ) const
{
    return new Tls( y, x, 0.05 );
}


/** Called only if resIsComputed==false */
void Tls::compute_res()
{
    res_ = y_ - yhat();
}

/** Called only if betaIsComputed==false */
void Tls::compute_beta()
{
    amoebaData[0] = (Md*)&y_;
    amoebaData[1] = &x_;
    amoebaData[2] = (Md*)&m_trimFactor;

    beta_ = estimate( amoebaData );
}

/** private */
Md Tls::estimate( const void* data )
{
    Vd y = *(((Vd** )data)[0]); 
    Md x = *(((Md** )data)[1]);

    // Initial guess -- OLS beta
    Vd beta_guess( matmult(inverse(matmult_xpy(x,x)),
                           matmult_xpy(x,y)));
    // Set up amoeba
    int dim = x.cols();
    Md p( dim+1, dim );	       // vertices of the simplex
    Vd val( dim+1, vertical ); // function value (trimmed squares) at 
                               // those vertices

    for( int i=0;i<dim;i++ )
        p[0][i] = beta_guess[i];
    init_simplex( p, dim );
    init_y( val, p, dim, data, trimmedSquares );

    int n_iter=0;
    amoeba( p, val, dim, 0.000001, trimmedSquares, n_iter, 
            data, 1 );

    V<int> subset = 0;
    subset.setOrientation(vertical);
    Md result = take( p, subset );
    return transp(result);
} // Tls::estimate()
//--------------------

/** Function we feed to amoeba().  Can't be a member of Tls because 
 *  init_simplex() and init_y() expect that.
*/
static double trimmedSquares( const Vd& beta, const void* data )
{
    // "parse" the data
    Md** mdata = (Md **)data;
    Vd& y = *((Vd*)mdata[0]);
    Md& x = *(mdata[1]);
    double trimFactor = *(double*)mdata[2];

    Md res = transp( Md(y) - matmult_xyp(x,beta) );
    Md ressq = sort(res*res);

    int first_index = int( trimFactor * res.cols() + 0.5 );
    int last_index  = int( res.cols() - trimFactor*res.cols() + 0.5 );
    if( trimFactor > 0.5 )
        rcmError( "Tls: trim factor must be <= 0.5" );
    if( last_index == first_index )
        last_index +=1;

    double result=0;
    for( int i=first_index; i<last_index; i++ ) 
        result += ressq[0][i];

    // We add the +1 because amoeba has trouble when the objective function
    // approaches zero.
    return result + 1.0;
}
//-----------------


/** Called only if yhatIsComputed==false */
void Tls::compute_yhat()
{
    yhat_ = matmult( x_, beta() );
}

/** Called only if r2IsComputed==false */
void Tls::compute_r2()
{
    r2_ = (matmult_xpy(yhat(),yhat()) / matmult_xpy(y_,y_))[0][0];
}

/** Called only if tstatsIsComputed==false */
void Tls::compute_tstats()
{
    cout << "Tls::compute_tstats() not implemented yet.\n" << std::flush;
//  tstats_ = beta()/sdBeta;
}

/** Called only if betaVcvIsComputed==false */
void Tls::compute_betaVcv()
{
    cout << "Tls::compute_betaVcv() not implemented yet.\n" << std::flush;
//  betaVcv_ =  Md(sdRes) * inverse(matmult_xpy(x_,x_));
}

/** Called only if betaCorrelmatIsComputed==false */
void Tls::compute_betaCorrelmat()
{
    Md vcv( betaVcv() );
    int dim = x_.cols();
    Md result(dim,dim);

    for( int i=0;i<dim;i++ )
    {
        for( int j=0;j<dim;j++ )
        {
            if( vcv[i][i] == 0 || vcv[j][j] == 0 )
            {
                result[i][j] = 0;
            }
            else
            {
                result[i][j] = vcv[i][j]/pow(vcv[i][i]*vcv[j][j],0.5);
            }
        }
    }

    betaCorrelmat_ = result;
}
