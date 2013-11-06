/* handle trades */

/**** INCLUDES ****/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "xmem.h"
#include "aqc.h"

/**** GLOBAL VARIABLES ****/
extern int N_players;
extern SECURITY **Securitydata;

/**** LOCAL FUNCTION DECLARATIONS ****/

/**** FUNCTIONS VISIBLE EXTERNALLY ****
TRADE *cross( ORDER *mkt, ORDER **best_lim, char *security )
void disp_trade( FILE *outfile, TRADE *trade, char *security );
void notify( TRADE *newtrade, char *security );
void dump_trade( TRADE *trade, HOLDINGS *holding, char *security );
void tradecpy( TRADE *dest, TRADE *source );
****/

/**** FUNCTION DEFINITIONS ****/

TRADE *cross( ORDER *mkt, ORDER **best_lim, char *security ) {
	/* Look for opportunities to cross the orders.
     * mkt is the order that just came in over the wire, best_lim is
     *   the best limit order on the other side.
     * This is only invoked if the latest order is a "best", i.e.
     *   goes to the head of its queue.
     * Don't cross mkt with mkt, or lim with lim, yet.
     * Update the queue heads ("best"'s).
     * Update all affected traders' holdings.
     *
     * Return NULL if there are no limit orders available.
     * Return TRADE structure for the mkt side.
     *
     * Only one of the above will result in a non-NULL trade, as we're
     *   clearing trades as they come in.  The calling routine
     *   knows who placed the latest order, and takes care to
     *   notify him.  Here, we don't notify anybody, but still
     *   attend to updating holdings (each player's holding is
     *   on disk in a file just for him).
     * If there are no limit orders around, we should discard new
     *   market orders;  even the person who placed them would want
     *   to have them discarded.  If not, a sharp character will
     *   make a killing placing an extremely ungenerous limit order.
     * Thus, the only new orders that will result in immediate trades
     *   will be market orders.  Moreover, though a new market order
     *   could well cross with more than one limit order, the maximum
     *   number of shares traded will be the amount in the market
     *   order.  Thus we notify the market order placer of the
     *   return value, TRADE *trade.
    */
    int mkt_uncrossed, lim_uncrossed, num_traded, trade_sign;
	TRADE *mkttrade, *limtrade; /* transaction for mkt and best_lim */
    HOLDINGS *holding;  /* one trader's holding, as opposed to function
      parameter holdings, which contains all traders' holdings */
    SECURITY *sec;

    sec = find_sec( security, Securitydata );
    /* we'll need sec for finding appropriate currency */

    /* check if the limit order book is empty */
    if( ( *best_lim == NULL )	/* no limit orders */
    ||  ( mkt->amount <= 0 ) )
    	return NULL;

    /* set sign to apply to shares field in lattrade */
    if( mkt->buy_or_sell == BUY )
    	trade_sign = 1;
    else
    	trade_sign = -1;

    /* allocate TRADE structures.  alloc_trade inits everything to 0 */
    mkttrade = trade_alloc();
    limtrade = trade_alloc();
    strcpy( mkttrade->name, mkt->name );
    strcpy( limtrade->name, (*best_lim)->name );

    /* find how many shares, at the maximum, will change hands */
    mkt_uncrossed = (int)(mkt->amount);
    lim_uncrossed = (int)((*best_lim )->amount);

    /* cross orders until either market or limit side is exhausted. */
    while( ( *best_lim != NULL ) && ( mkt_uncrossed > 0 ) ) {

    	/* find out how many shares are going to change hands */
        num_traded = MIN( mkt_uncrossed, lim_uncrossed );

        /* set trade prices: mkttrade's price is weighted ave */
        limtrade->price = (*best_lim)->price;
        mkttrade->price =
       	    (mkttrade->price * mkttrade->amount +
            (*best_lim)->price * trade_sign*num_traded) /
            ( mkttrade->amount + trade_sign*num_traded );

        /* set traded amount */
        mkttrade->amount += trade_sign*num_traded;
        limtrade->amount -= trade_sign*num_traded;

        /* decrement amount ordered by amount crossed */
        mkt->amount -= num_traded;
        (*best_lim)->amount -= num_traded;

        /* time stamp */
        time( &(mkttrade->zman) );
        time( &(limtrade->zman) );

        /* update the limit-order trader's account, and the total trade diary */
        holding = load_holding( limtrade->name, security );
        update_holding( limtrade, holding );
        strcpy( limtrade->other_sec, sec->currency );
        limtrade->as_money = NO;
        dump_trade( limtrade, holding, security );
        update_diary( limtrade, holding, security );

        /* update the money .act file for the limit order side of the trade */
        holding = load_holding( limtrade->name, sec->currency );
        update_money( limtrade, holding );
        strcpy( limtrade->other_sec, sec->name );
        limtrade->as_money = YES;
        dump_trade( limtrade, holding, sec->currency );
        update_diary( limtrade, holding, sec->currency );

        /* if limit side now has negative money, put his name on the list
         * from which we initiate forced sales */
        check_neg_money( limtrade->name, sec->currency );
        if( ismoney(sec)==YES )
            check_neg_money( limtrade->name, security );

        trade_free( limtrade );

        /* check for exhaustion of *best_lim: if it's exhausted (it will
         * be if mkt isn't), reset *best_lim to point to the next limit order
         * on the list.  Alloc a new TRADE if *best_lim!=NULL. */
        if( (*best_lim)->amount == 0 ) {
            *best_lim = (*best_lim)->next;
            if( *best_lim != NULL ) {
                (*best_lim)->prev = NULL;
        		limtrade = trade_alloc();
	            strcpy( limtrade->name, (*best_lim)->name );
            }
        }

    	mkt_uncrossed = (int)(mkt->amount);
  	    if( *best_lim != NULL )
	        lim_uncrossed = (int)((*best_lim )->amount);

    }

    /* we've crossed all the mkt order shares/units/amount we could. */
    if( mkttrade != NULL ) {
        /* update market-side's holding and accounts, and check if
         *   he has negative money. */
          holding = load_holding( mkttrade->name, security );
          update_holding( mkttrade, holding );
          strcpy( mkttrade->other_sec, sec->currency );
          mkttrade->as_money = NO;
          dump_trade( mkttrade, holding, security );  /* to .act file */
	      update_diary( mkttrade, holding, security );

          /* update the money .act file for the limit order side of the trade */
          holding = load_holding( mkttrade->name, sec->currency );
          update_money( mkttrade, holding );
          strcpy( mkttrade->other_sec, sec->name );
          mkttrade->as_money = YES;
          dump_trade( mkttrade, holding, sec->currency );
          update_diary( mkttrade, holding, sec->currency );

          check_neg_money( mkttrade->name, sec->currency );
          if( ismoney(sec)==YES )
              check_neg_money( mkttrade->name, security );
    }

    return mkttrade;

} /* end of cross() */
/*--------------------------*/

void notify( TRADE *newtrade, char *security ) {
    printf( "Execution..." );
    if( newtrade != NULL )
	disp_trade( stdout, newtrade, security );
    else
        client_warning("No trade--order book empty.");
} /* end of notify() */
/*--------------------------*/

void disp_trade( FILE *outfile, TRADE *trade, char *security ) {
    time_t zman;
    SECURITY *sec;

    sec = find_sec( security, Securitydata );

    zman = trade->zman;

    if( ismoney(sec) == NO )
        fprintf( outfile, "%8s %5d %6.3f %s",
            trade->name, (int)trade->amount,
            (float)(trade->price), asctime(localtime(&zman)) );
    else
        fprintf( outfile, "%8s %8.2f %6.3f %s",
            trade->name, (float)trade->amount,
            (float)(trade->price), asctime(localtime(&zman)) );

} /* end of disp_trade() */
/*--------------------------*/

void dump_trade( TRADE *trade, HOLDINGS *holding, char *security ) {
  /* write (binary) the latest trade to the .act file that holds the
   *   whole history of this player's activities--orders, trades, holding.
  */
  char actfilename[ACTNAMELEN];  /* accounts/<name>.act */
  FILE *actfile;
  ORDER *dummyorder;

  /* open the file for appending in binary mode */
  actfile_name( trade->name, security, actfilename );
  actfile = fopen( actfilename, "ab" );
  if( actfile==NULL ) error( "Trades.c: no .act file!!" );

  /* allocate dummy trade */
  dummyorder = order_alloc();

  /* write */
  fwrite( dummyorder, sizeof(ORDER), 1, actfile );
  fwrite( trade, sizeof(TRADE), 1, actfile );
  fwrite( holding, sizeof(HOLDINGS), 1, actfile );

  /* close */
  fclose( actfile );

  /* de-allocate */
  order_free( dummyorder );

} /* end of dump_trade() */
/*--------------------------*/

void tradecpy( TRADE *dest, TRADE *source ) {
  /* copy the contents of a TRADE structure.
   * We assume *dest has been allocated!!
  */
  strcpy( dest->name, source->name );
  dest->amount = source->amount;
  dest->price  = source->price;
  dest->zman   = source->zman;
} /* end of tradecpy() */
/*--------------------------*/
/*--------------------------*/
/*--------------------------*/
