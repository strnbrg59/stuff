#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include "rcm.h"

static double amoebaObjective( const Vd& params, const void* data );

int main()
{
  try
  {
    cout << (iota(5)+20) << '\n';

    Md md1(2,2);
    md1[0][0] = 4;
    md1[1][1] = 5;
    md1[1][0] = 2;
    md1[0][1] = 3;
    
    cout << "md1:\n" << md1;
    cout << "inverse(md1):\n" << inverse(md1) << '\n';
    cout << "inverse(inverse(md1)):\n" << inverse(inverse(md1)) << '\n';

    cout << "md1*md1\n" << md1*md1;
    cout << "inverse(inverse(md1*md1))\n" << inverse(inverse(md1*md1));
    cout << "md1-md1\n" << md1-md1;
    cout << "md1/md1\n" << md1/md1;

    cout << "exp(md1)\n" << exp(md1);
    cout << "inverse(inverse(exp(md1)))\n" << inverse(inverse(exp(md1)));
    cout << "log(md1)\n" << log(md1);

    cout << "inverse(inverse(matmult(transp(md1),md1)))\n" 
         << inverse(inverse(matmult(transp(md1),md1)));

    cout << "pow(md1,0.5):\n" << pow(md1,0.5);
    cout << "pow(iota(8),2):\n" << pow(iota(8),2);

    Md md5(5,5); md5 = md5 + 10;
    cout << "md5:\n" << md5;
    cout << "urand(md5):\n" << urand(md5);

    Vd vd1(iota(5));
    cout << "matmult(vd1,transp(vd1)):\n" << matmult(vd1,transp(vd1));
    cout << "pow(vd1,2):\n" << pow(vd1,2);        
    cout << "transp(vd1):\n" << transp(vd1);

    cout << "lamPlus(vd1): " << lamPlus(vd1);
    cout << "lamPlus(pow(vd1,2)): " << lamPlus(pow(vd1,2));
    cout << "lamPlus(transp(vd1)):\n" << lamPlus(transp(vd1));
    cout << "lamPlus(md1):\n" << lamPlus(md1);
    cout << "lamTimes(vd1+1.0)): " << lamTimes(vd1+1.0);

    cout << "sum(md1):\n" << sum(md1);
    cout << "mean(md1):\n" << mean(md1);
    cout << "sd(md1):\n" << sd(md1);
    cout << "matmult(transp(sum(md1)),sum(md1)):\n" 
         << matmult(transp(sum(md1)),sum(md1));


    // matmult_xpy and matmult_xyp
    Md md23(2,3);
    for( int i=0;i<2;i++ ) for( int j=0;j<3;j++ ) md23[i][j] = i+j;
    cout << "matmult((transp(md23)),md23):\n"
         << matmult((transp(md23)),md23);
    cout << "matmult_xpy(md23,md23):\n"
         << matmult_xpy(md23,md23);
    cout << "matmult(md23, transp(md23)):\n"
         << matmult(md23, transp(md23));
    cout << "matmult_xyp(md23,md23):\n"
         << matmult_xyp(md23,md23);

    cout << "md23:\n" << md23;
    cout << "ravel(md23):\n" << ravel(md23) << '\n';
    cout << "ravel(Md(ravel(md23))):\n" << ravel(Md(ravel(md23))) << '\n';
    cout << "ravel(Md(transp(ravel(md23)))):\n" 
         << ravel(Md(transp(ravel(md23)))) << '\n';

    cout << "catcols(md1,md1):\n" << catcols(md1,md1);
    cout << "catcols(transp(md1[0])),md1):\n" << catcols(transp(md1[0]),md1);
    cout << "catrows(md1,md1):\n" << catrows(md1,md1);
    cout << "catrows(md1[0],md1):\n" << catrows(md1[0],md1);
    cout << "catrows(iota(5),iota(5)+20):\n" << catrows(iota(5),iota(5)+20);
    cout << '\n';

    cout << "urand(3,4):\n" << urand(3,4);
    cout << "nrand(3,4):\n" << nrand(3,4);
    Md u1000( urand(1,1000) );
    Md s(1,1),ss(1,1);
    cout << "sum(u1000) = " << (s=sum(u1000));
    cout << "sum(pow(u1000,2))="
         << (ss = sum(pow(u1000,2)));
    cout << "variance(u1000)*12=" 
         << 12*(ss/1000 - pow(s/1000,2)) << '\n';
    Md n1000( nrand(1,1000) );
    cout << "sum(n1000) = " << (s=sum(n1000));
    cout << "sum(pow(n1000,2))="
         << (ss = sum(pow(n1000,2)));
    cout << "variance(n1000)=" << ss/1000 - pow(s/1000,2) << '\n';
    cout << '\n';

    Md u5(1,5);
    cout << "u5=urand(1,5)=" << (u5=urand(1,5));
    cout << "indexx(u5)=" << indexx(u5) << '\n';
    cout << "sort(u5)=" << sort(u5) << '\n';

    Md u23( urand(2,3));
    cout << "u23:\n" << u23;
    cout << "drho(5,5,u23):\n" << drho(5,5,u23);
    cout << "varcovar(matmult_xpy(u23,u23)):\n" 
         << varcovar(matmult_xpy(u23,u23));
    cout << "correlmat(matmult_xpy(u23,u23)):\n" 
         << correlmat(matmult_xpy(u23,u23));

    Md nr1000( nrand(1000,3));
    cout << "varcovar(nrand(1000,3)):\n" << varcovar(nr1000);
    cout << "correlmat(nrand(1000,3)):\n" << correlmat(nr1000);

    cout << "drho(2,3,M<string>(\"howdy\")):\n"
         << drho(2,3,M<string>("howdy"));
    M<string> ms23(2,3);
    ms23[0][0] = "howdy"; ms23[0][1] = "doody"; ms23[0][2] = "time";
    ms23[1][0] = "the"; ms23[1][1] = "barney"; ms23[1][2] = "show";
    cout << "ms23:\n" << ms23;
    cout << "transp(ms23):\n" << transp(ms23);

    /*
    Md m1(2,4), m2(3,4);
    cout << matmult(m1,m2);
    */

    cout << "sin(drho(2,2,3.0)):\n";
    cout <<  sin(drho(2,2,Md(3.0)));
    cout << "sin(drho(2,3,3.0)):\n";
    cout <<  sin(drho(2,3,Md(3.0)));

    cout << "quantiles(iota(9),1)= " << quantiles(iota(9),1);
    cout << "quantiles(iota(9),2)= " << quantiles(iota(9),2);
    cout << "quantiles(iota(9),3)= " << quantiles(iota(9),3);
    cout << "quantiles(iota(3),1)= " << quantiles(iota(3),1);
    cout << "quantiles(iota(4),2)= " << quantiles(iota(4),2);
    cout << "median(iota(9))= " << median(iota(9));
    cout << "median(iota(8))= " << median(iota(8));
    cout << "quantiles(nrand(1,10000),3):\n" << quantiles(nrand(1,10000),3);

    cout << "drho(4,4,Md(iota(16))):\n" << drho(4,4, Md(iota(16)));
    cout << "take( drho(4,4,Md(iota(16))), "
         << "convertV<double,int>(iota(2))):\n"
         << take( drho(4,4,Md(iota(16))), convertV<double,int>(iota(2)));

    cout << "take( drho(4,4,Md(iota(16))), "
         << "convertV<double,int>(iota(2)+1)):\n"
         << take( drho(4,4,Md(iota(16))), convertV<double,int>(iota(2)+1));
    cout << "take( drho(4,4,Md(iota(16))), "
         << "convertV<double,int>(transp(iota(2)))):\n"
         << take( drho(4,4,Md(iota(16))), 
                  convertV<double,int>(transp(iota(2))));
    cout << "take( drho(4,4,Md(iota(16))),"
         << "convertV<double,int>(transp(iota(2)+1))):\n"
         << take( drho(4,4,Md(iota(16))), 
                  convertV<double,int>(transp(iota(2)+1)));

    // Regressions
    Vd y( Vd(3.14)*transp(iota(20)) + Vd(nrand(20,1)/nrand(20,1)) );
    Md x( catcols( drho(20,1,Md(1.0)), transp(iota(20)) ) );
    cout << "ols1( 3.14*transp(iota(20)) + nrand(20,1), "
         << "catcols( drho(20,1,1.0), transp(iota(20)) )):\n";
    Ols ols1( y,x );
    cout << "ols1.beta():\n";
    cout << ols1.beta();
    cout << "catcols(catcols(y,ols1.yhat()),ols1.res()):\n";
    cout << catcols(catcols(y,ols1.yhat()),ols1.res());
    cout << "round(catcols(catcols(y,ols1.yhat()),ols1.res()),0.1):\n";
    cout << round(catcols(catcols(y,ols1.yhat()),ols1.res()),0.1);
    cout << "ols1.r2() = " << ols1.r2() << '\n';
    cout << "round(ols1.r2(),0.00001) = " << round(ols1.r2(),0.00001)<<'\n';
    cout << "ols1.tstats():\n" << ols1.tstats() << '\n';
    cout << "ols1.betaVcv():\n" << ols1.betaVcv();
    cout << "ols1.betaCorrelmat():\n" << ols1.betaCorrelmat();
    cout << "ols1.displayAll():\n" << ols1.displayAll();
    cout << '\n';

    Tls tls01( y,x,0.01 );
    cout << "tls01.beta():\n";
    cout << tls01.beta() << '\n';
    cout << "tls01 [y|yhat]=\n" << catcols(tls01.y(), tls01.yhat()) << endl;
    Tls tls05( y,x,0.05 );
    cout << "tls05.beta():\n";
    cout << tls05.beta() << '\n';
    Tls tls10( y,x,0.10 );
    cout << "tls10.beta():\n";
    cout << tls10.beta() << '\n';
    Tls tls25( y,x,0.25 );
    cout << "tls25.beta():\n";
    cout << tls25.beta() << '\n';
/*
    cout << "catcols(catcols(y,tls1.yhat()),tls1.res()):\n";
    cout << catcols(catcols(y,tls1.yhat()),tls1.res());
    cout << "round(catcols(catcols(y,tls1.yhat()),tls1.res()),0.1):\n";
    cout << round(catcols(catcols(y,tls1.yhat()),tls1.res()),0.1);
    cout << "tls1.r2() = " << tls1.r2() << '\n';
    cout << "round(tls1.r2(),0.00001) = " << round(tls1.r2(),0.00001)<<'\n';
    cout << "tls1.tstats():\n" << tls1.tstats() << '\n';
    cout << "tls1.betaVcv():\n" << tls1.betaVcv();
    cout << "tls1.betaCorrelmat():\n" << tls1.betaCorrelmat();
    cout << "tls1.displayAll():\n" << tls1.displayAll();
*/
    cout << '\n';

    // cgram.  First, create an AR(1) process.
    int n = 1000;
    Vd u( nrand(1,n)); 
    Vd e(n,horizontal);
    e[0] = u[0];
    for( int i=1;i<n;i++ ) e[i] = u[i] + 0.5*e[i-1];
    cout << "cgram of e_t = u_t + 0.5*u_{t-1} AR(1) process:\n";
    cout << cgram( e,e,10 ) << '\n';

    // Eigenvalues and vectors.
    Md m(urand(drho(5,5,Md(100))+1));
    m = matmult_xpy(m,m);
    cout << "m:\n" << m << '\n';
    Md eigvects( take( eigen(m), convertV<double,int>(transp(iota(4)+1))));
    cout << "eigvects(m):\n" << eigvects;
    cout << "matmult_xyp(eigvects,eigvects):\n"
         << round(matmult_xyp(eigvects,eigvects),0.001);
    cout << "rowmax(m):\n" << rowmax(m);
    cout << "-m:\n" << -m;

    // Kernel density
    Md nr( lamPlus(nrand(1,400)) );
    //cout << kernelDensity(nr,1.0) << '\n';

    // Diffs
    m = Md( catrows( pow(iota(10),2.0), pow(iota(10),3.0)));
    cout << "m: " << m;
    cout << "diffs(m): " << diffs(m);
    cout << "shuffle(m):\n" << shuffle(m);

/*
    // smooth(): produces alot of output.
    Md walk( lamPlus(nrand(1,200)));
    Md smoothwalk( smooth(walk, 25) );
    int wn = smoothwalk.cols();
    cout << catcols( transp(walk), transp(iota(200)));
    cout << "******";
    cout << catcols( transp(smoothwalk - 20.0), transp(iota(wn)));
    cout << "******";
*/

    // efficientFrontier
    cout << "\nPoints:\n";
    int en=10;
    Md randpoints( round(nrand(en,2),0.01) );
    cout << randpoints;
    V<int> orthant( 2,horizontal );
    orthant[0] = 1; orthant[1] = 1;
    cout << "NW Efficient frontier:\n" 
         << efficientFrontier( randpoints, orthant );
    cout << '\n';

    // Logic
    Md ml1( urand(drho(2,5,Md(10))+1));
    Md ml2( urand(drho(2,5,Md(10))+1));
    cout << "ml1:\n" << ml1<< "ml2:\n" << ml2 << "ml1 < ml2:\n" << (ml1 < ml2);
    cout << "ml1 <= ml2:\n" << (ml1 <= ml2);
    cout << "ml1 > ml2:\n" << (ml1 > ml2);
    cout << "ml1 >= ml2:\n" << (ml1 >= ml2);
    cout << "ml1 == ml2:\n" << (ml1 == ml2);
    cout << "ml1 != ml2:\n" << (ml1 != ml2);
    cout << "!(ml1==ml2):\n" << !(ml1==ml2);

    M<int> ml3( convertM<double,int>(urand(drho(3,5,Md(2)))));
    M<int> ml4( convertM<double,int>(urand(drho(3,5,Md(2)))));
    cout << "ml3:\n" << ml3;
    cout << "ml4:\n" << ml4;
    cout << "ml3 || ml4 :\n" << (ml3 || ml4);
    cout << "ml3 && ml4 :\n" << (ml3 && ml4);
    cout << "(!ml3) && (!ml4) :\n" << ((!ml3) && (!ml4));
    cout << "! (ml3 || ml4) :\n" << (! (ml3 || ml4));

    // stepwiseRegression
    Vd y1( Vd(3.14)*transp(iota(20)) + 5.0*Vd(nrand(20,1)/nrand(20,1)) );
    Md candidates = catcols(catcols(catcols(
                                            transp(iota(20)),
                                            transp(pow(iota(20),2))),
                                            transp(pow(iota(20),3))),
                                            transp(pow(iota(20),4)));
    Regression* bestModel = 
      stepwiseRegression(y1,candidates, new Ols(y1,candidates), 0.90);
    if( bestModel )
        cout << "stepwise best model:\n" << bestModel->displayAll() << '\n';
    Ols ols(y1,catcols(drho(20,1,Md(1.0)),candidates));
    cout << "check with ols on all columns:\n" << ols.displayAll();

    cout << "drho(3,5,iota(6)-3):\n" << drho(3,5,Md(iota(6)-3));
    cout << "fabs(drho(3,5,iota(6)-3)):\n" << fabs(drho(3,5,Md(iota(6)-3)));

    // Amoeba test
    {
    double* data; // we don't use data in this example, but it's useful to
                  // be able to pass some to the objective function as needed.
    const int dim=2;
    Md p(dim+1, dim );    // vertices of the simplex
    Vd y(dim+1,vertical); // function values at those vertices
    
    p[0][0] = 1001; p[0][1] = -10000;  
    init_simplex( p, dim ); // fills out other corners of initial simplex
    init_y( y, p, dim, data, amoebaObjective );
 
    int n_iter=0;
    amoeba( p, y, dim, 0.00000001, amoebaObjective, n_iter, data, 1 );
    }
    // End amoeba test

    for( int i=-6;i<7;i++ )
    {
        double x = i/2.0;
        cout << "gaussian01(" << x << ") = " << gaussian01(x) << '\n';
    }

    cout << ident<int>(3) << endl;

  }

  catch( RcmException re )
  {
      re.print();
  }

}

// For amoeba:
double amoebaObjective( const Vd& params, const void* data ) 
{
    double result     = (params[0]-3.141592654)*params[0] 
                      + (params[1]-2.7182718)*params[1] 
                    + 0.1;
    return result;
}

