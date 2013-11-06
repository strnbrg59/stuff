/* Generate market orders as a compound Poisson process, and execute them
 *   through the AQC market.
 *
 * Synopsis:
 *   1. Generate exponentially-distributed r.v.
 *   2. sleep() that long.
 *   3. Upon "awakening", generate a trade and pass it to handle_order().
 *   4. goto step 1.
 *   5. Smart features:
 *     a. doesn't trade if B-A spread is wider in % than TOOWIDE.
 *     b. doesn't "fall off cliffs", ie won't trade more than the amount
 *        bid or offered at the inside price.
*/

/**** INCLUDES ****/
#ifdef __TURBOC__
#include <dos.h>
#include <time.h>   /* for randomize() */
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../aqc.h"

/**** DEFINITIONS ****/
#define PARAMFILE "poisson.par"
#define TOOWIDE 10  /* poisson won't trade if B-A spread > TOOWIDE% */


/**** GLOBAL VARIABLES--for compatibility with rest of aqc library ****/
int N_players;
NAMES *Names;
SECURITY **Securitydata;
int N_sec;

/**** LOCAL FUNCTION DEFINITIONS ****/
static double exprv( double lambda );
static double normrv( double mu, double sigma );
static void   load_params( char *security,
                    double *lambda, double *mu, double *sigma,
                    int *openhour, int *closehour );
static double urand( void );
static int toowide( char *security, double signed_amount);

/**** FUNCTION DEFINITIONS ****/
main() {
  static char security[NAMELEN+1];
  int noorder,        /* boolean */
      openhour, closehour,      /* during which this program trades */
      maxorder,
      sleeptime;      /* interval between trades, in seconds */
  double lambda,    /* param of exponential dsn */
         signed_amount,  /* amount (or units) to be ordered. <0 => sell */
         mu, sigma; /* params of normal dsn     */
  time_t zman;
  struct tm *tmnow;
  ORDER *order;

#ifndef __32BITBYTES__
      randomize();
#else
      time( &zman );
      srand( (unsigned int)zman );
#endif

  Securitydata = load_securities();

  /* load players' names into global variable */
  N_players = load_players( &Names );

  /* load the parameters */
  load_params( security, &lambda, &mu, &sigma, &openhour, &closehour );

  /* loop forever (until c-shell kill command) */
#ifdef __WATCOMC__
  while( i<5 ) {        /* otherwise there's no way to stop it! */
      i++;
#else
  while(1) {
#endif
      /* go to sleep */
      sleeptime = (int)(round( exprv( lambda ), 1.0 ));
      printf( "sleeptime = %d\n", sleeptime );
      sleep( sleeptime );

      /* awake! submit an order if it's between openhour and closehour */
      time( &zman );
      tmnow = localtime( &zman );
      if(/*  (tmnow->tm_wday > 0) && (tmnow->tm_wday<6) && */
      (tmnow->tm_hour >= openhour) && (tmnow->tm_hour < closehour) ) {

          /* initialize order (it gets x_free'd in handle_order() ) */
          order = order_alloc();
          strcpy( order->name, "poisson" );
          order->mkt_or_lim = MKT;

          /* generate the order: a normal r.v., whose sign indicates
           * buy/sell, and whose magnitude indicates the number of shares
          */
          signed_amount = round( normrv( mu, sigma ), 1.0 );

          /* determine buy-or-sell.  Don't place zero trades; handle_order()
           *   will pass them to cross(), which will return a NULL pointer
          */
          if( signed_amount > 0 )
              order->buy_or_sell = BUY;
          if( signed_amount < 0 )
              order->buy_or_sell = SELL;
          if( signed_amount == 0 )
              noorder = 1;
          else
              noorder = 0;

          /* check B-A spread and avoid falling off cliffs */
    	  maxorder = toowide(security, signed_amount);
          order->amount = MIN( maxorder, fabs( signed_amount ));
          if( order->amount == 0 ) noorder = 1;

          time( &(order->zman) );    /* time-stamps the order */

          if( noorder == 0 )
              handle_order( order, security, YES );
    	  else
	          order_free( order );

      }
  }
return 0;
} /* end of main() */
/*-----------------------------*/

static double exprv( double lambda ) {
  /* exponential r.v. */
  double unif, result;

  unif = urand();
  result = -log( 1-unif )/lambda;

  return result;
} /* end of exprv() */
/*-----------------------------*/

static double normrv( double mu, double sigma ) {
  /* sum 12 u(-1/2, 1/2) r.v.'s & get n(0,1) */
  int i;
  double result=0;

  for( i=0;i<12;i++ )
      result += ( urand() - 0.5 );

  result = sigma * result  + mu;
  return result;
} /* end of normrv() */
/*-----------------------------*/
static void load_params( char *security,
                    double *lambda, double *mu, double *sigma,
                    int *openhour, int *closehour ) {
  char wholeline[61], *charnum;
  FILE *infile;

  /* open parameter file */
  if( (infile=fopen(PARAMFILE,"r")) == NULL ) {
      printf( "load_params: failed to open PARAMFILE" );
      exit(0);
  }

  /* get security */
  fgets( wholeline, 60, infile );
  strtok( wholeline, " \t\n" ); /* gets rid of "security" */
  strcpy( security, strtok( NULL, " \t\n" ) );

  /* get lambda */
  fgets( wholeline, 60, infile );
  strtok( wholeline, " \t\n" );   /* gets rid of "lambda" */
  charnum = strtok( NULL, " \t\n" );
  *lambda = atof( charnum );
  printf( "lambda = %f\n", (float)(*lambda) );

  /* get mu */
  fgets( wholeline, 60, infile );
  strtok( wholeline, " \t\n" );
  charnum = strtok( NULL, " \t\n" );
  *mu = atof( charnum );
  printf( "mu = %f\n", (float)(*mu) );

  /* get sigma */
  fgets( wholeline, 60, infile );
  strtok( wholeline, " \t\n" );
  charnum = strtok( NULL, " \t\n" );
  *sigma = atof( charnum );
  printf( "sigma = %f\n", (float)(*sigma) );

  /* get openhour */
  fgets( wholeline, 60, infile );
  strtok( wholeline, " \t\n" );
  charnum = strtok( NULL, " \t\n" );
  *openhour = atoi( charnum );
  printf( "openhour = %d\n", *openhour );

  /* get closehour */
  fgets( wholeline, 60, infile );
  strtok( wholeline, " \t\n" );
  charnum = strtok( NULL, " \t\n" );
  *closehour = atoi( charnum );
  printf( "closehour = %d\n", *closehour );

  fclose( infile );

} /* end of load_params() */
/*-----------------------------*/

static double urand( void ) {
  /* uniform(0,1) r.v. */
  const double rand_max = pow(2.0,31.0)-1;
  double result;

  result = (rand()+1)/(rand_max + 2);  /* we don't want 0 or 1 possible */
  return result;
} /* end of urand() */
/*-----------------------------*/

static int toowide( char *security, double signed_amount) {
  /* return 0 if B-A spread is wider than TOOWIDE%.  
   * Otherwise, return the size of the inside bid order (if signed_amount<0 ),
   *   or the inside ask order (if signed_amount>0).
  */
  int result;
  ORDER *bestask, *bestbid;

  load_ordbook( &bestbid, &bestask, security );

  if( (bestask==NULL) || (bestbid==NULL) || (bestask->price == 0) ) {
      free_ordbook( bestbid, bestask );
      return 0;
  }

  if( fabs((bestask->price - bestbid->price)/bestask->price) > TOOWIDE/100.0 )
      result = 0;
  else
      if( signed_amount<0 )
          result = (int)(bestbid->amount);
      else
          result = (int)(bestask->amount);

  printf( "bestask->price = %f, bestbid->price = %f, result = %d\n",
      (float)(bestask->price), (float)(bestbid->price), result );

  free_ordbook( bestbid, bestask );
  return result;
} /* end of toowide() */
/*-----------------------------*/
/*-----------------------------*/
