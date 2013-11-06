/* Discount Window: program to automatically place limit buy orders
 *   in the FORCEMKT market  (in which forced sales take place) for
 *   the Fed account.
 *   The purpose is to maintain a certain floor interest rate on borrowing.
 * We only deal with the limit buy side.  That's the only side we need to
 *   care about.  If it's empty, forced sales will fail and players will
 *   get away with negative money holdings (i.e. interest-free loans).  We
 *   don't need to worry about the limit sell side emptying; if it does,
 *   all that means is that attempts to lend (market sales) will fail.
 * Since this is going to run continuously, make sure you release all the
 *   space you routinely malloc!
*/

/**** INCLUDES ****/
#ifdef __TURBOC__
  #include <time.h>
  #include <dos.h>
  #include <alloc.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../aqc.hpp"

/**** DEFINITIONS ****/
#define AMOUNT 640000        /* number of units to bid or ask */
#define WEEKSECS 604800.0   /* number of seconds in a week */
#define PARFILENAME "discwind.par"

/**** GLOBAL VARIABLES (not used) ****/
int N_players;
NAMES *Names;
SECURITY **Securitydata;
int N_sec;

/**** LOCAL FUNCTION DECLARATIONS ****/
static void discwind( int matur_yy, int matur_mon, int matur_dd,
               int matur_hh, int matur_min, int matur_ss,
               double faceval, double intrate, int sell_too, char *security);
static void load_params( int *matur_yy, int *matur_mon, int *matur_dd,
                  int *matur_hh, int *matur_min, int *matur_ss,
                  double *faceval,
                  double *intrate, int *freq, int *sell_too, char *security);

/**** FUNCTION DEFINITIONS ****/
main() {
  char      security[NAMELEN+1];
  int       matur_yy, matur_mon, matur_dd, matur_hh, matur_min, matur_ss,
            freq,
            sell_too;    /* set to 1, this means we place sell orders, too */
  long int  leftcore;
  double    faceval, intrate;

  Securitydata = load_securities();

  load_params( &matur_yy, &matur_mon, &matur_dd,
               &matur_hh, &matur_min, &matur_ss,
               &faceval,
               &intrate,
               &freq,
               &sell_too,
               security );


  /* loop around; sleep for freq minutes, then awaken and revise quote */
  while( 1 ) {

#ifdef __TURBOC__
        delay( 8000*freq );  /* argument to delay is in milliseconds */
#else
        sleep( 60*freq );  /* argument to sleep is in seconds */
#endif
      printf( "discwind about to place order...\n" );

      discwind( matur_yy,   matur_mon,   matur_dd,
                matur_hh,   matur_min,   matur_ss,
                faceval,    intrate, BUY, security );
      if( sell_too == 1 )
          discwind( matur_yy,   matur_mon,   matur_dd,
                    matur_hh,   matur_min,   matur_ss,
                    faceval,    intrate, SELL, security );

#ifndef __32BITBYTES__
          leftcore = coreleft();
          printf( "core left = %ld", leftcore );
#endif

  }
return 0;
} /* end of main() */
/*---------------------*/

static void discwind( int matur_yy, int matur_mon, int matur_dd,
                      int matur_hh, int matur_min, int matur_ss,
                      double faceval, double intrate,
                      int buy_or_sell, char *security ) {

  /* If buy_or_sell == BUY, we place orders on the buy side of the order book,
   *   if SELL, then on the sell side.  Normally, this function will get
   *   called only for BUY.  If we call it for SELL as well, then it will
   *   be completely determining the interest rate in our game.
  */

  int handle;  /* of socket */
  time_t maturzman, zman_now;
  double weeks_to_maturity, price;
  ORDER *best_limbuy, *best_limsell, *bestorder, *currorder, *new_order;


  /* Get server's attention.  If server is unavailable, return; that
   *   returns us to the loop in main, and the only effect is that
   *   revision of the limit order is postponed to the next time
   *   the loop wakes out of sleep().
  */
  handle = server_avail("gigi");
  if( handle < 0 ) return;
  server_ready( handle );

  /* Load FORCEMKT order book */
  load_ordbook( &best_limbuy, &best_limsell, security );

  /* go through order book, looking for orders by "Fed" */
  if( buy_or_sell == BUY )
      bestorder = best_limbuy;
  else
      bestorder = best_limsell;
  currorder = bestorder;

  while( currorder != NULL ) {
      if( !strcmp( currorder->name, "Fed" ) )

          /* remove order using splice_out().  Don't bother to record
           * this deed in Fed's .act file
          */
          if( buy_or_sell == BUY )
              splice_out( &best_limbuy, currorder );
          else
              splice_out( &best_limsell, currorder );

      currorder = currorder->next;
  }
  /* dump and free the order book */
  dump_ordbook( best_limbuy, best_limsell, security );
  free_ordbook( best_limbuy, best_limsell );

  /** Place a new order, in accordance with the prescribed interest rate **/

  maturzman = unix_seconds( matur_yy, matur_mon, matur_dd, matur_hh, matur_min,
                         matur_ss );
  time( &zman_now );

  /* set price */
  weeks_to_maturity = (maturzman - zman_now)/WEEKSECS;
  price = round( faceval * pow( 1+intrate, -weeks_to_maturity ), 0.01 );

  /* place an order */
  new_order = create_order( "Fed", LIM, buy_or_sell, (double)AMOUNT, price );
  handle_order( new_order, security, NO );
  printf( "new order placed: %d units at %f\n", AMOUNT, (float)price );

  /* release the server */
  server_goodbye( handle );

} /* end of discwind() */
/*---------------------*/

static void load_params( int *matur_yy, int *matur_mon, int *matur_dd,
                  int *matur_hh, int *matur_min, int *matur_ss,
                  double *faceval,
                  double *intrate,
                  int    *freq,
                  int    *sell_too,
                  char   *security ) {
  /* read data from discwind.par, which looks like this:
   *  date:   07 14 93
   *  time:   14 00 00
   *  faceval: 1
   *  intrate: .25
   *  freq:    1
   *  sell_too: 1
   *  security: pbond
  */

  char garbage[30], charnum[30];
  FILE *parfile;

  parfile = fopen( PARFILENAME, "r" );
  if( parfile==NULL ) { printf("\nCant open parfile.\n"); exit(0); }

  /* load maturity date */
  fscanf( parfile, "%s %d %d %d", garbage, matur_mon, matur_dd, matur_yy );
  fscanf( parfile, "%s %d %d %d", garbage, matur_hh, matur_min, matur_ss );

  /* load face value */
  fscanf( parfile, "%s %s", garbage, charnum );
  *faceval = atof( charnum );

  /* load interest rate */
  fscanf( parfile, "%s %s", garbage, charnum );
  *intrate = atof( charnum );

  /* load freq--the frequency, in minutes, with which we revise the quotes */
  fscanf( parfile, "%s %d", garbage, freq );

 /* load sell_too--if 1, then we place sell orders, too */
  fscanf( parfile, "%s %d", garbage, sell_too );

  /* load security: name of market we're going to operate in */
  fscanf( parfile, "%s %s", garbage, security );

} /* end of load_params() */
