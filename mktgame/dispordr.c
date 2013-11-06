#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xmem.h"
#include "aqc.h"

/**** GLOBAL VARIABLES ****/
extern SECURITY **Securitydata;

/**** FUNCTION DECLARATIONS ****/
static int fill_orderbook( ORDER *best_ord, ORDERBOOK *orderbook );

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void disp_orders( char *security )
void disp_order_summary( char *security );
****/

void disp_orders( char *security ) {
    /* display all the orders on both sides of the market, in two columns */

    char blanks[2], cmd,
        *currencyid;  /* says "prices in dollars", e.g. */
    int i=0;
    FILE *outfile;
    ORDER *currbuy, *currsell, *best_limbid, *best_limask;
    SECURITY *sec;

    sec = find_sec( security, Securitydata );

    currencyid = currency_id( sec );

    /* open files and load order book */
    load_ordbook( &best_limbid, &best_limask, security );
    outfile = stdout;

    /* prepare header for display */
    blanks[0] = ' '; blanks[1]=0;
    fprintf( outfile, "         *** limit order book -- %s %s***\n",
        security, currencyid );
    fprintf( outfile, "%13s***Bids***%18s***Offers***\n", " ", " " );

    /* loop over the order book and print it out */
    currbuy = best_limbid;
    currsell = best_limask;
    while( (currbuy != NULL) || (currsell != NULL) ) {

        if( currbuy != NULL ) {
            fprintf( outfile, "%12s %5d %6.2f",
                    (currbuy->name),
                    (int)(currbuy->amount), (float)(currbuy->price) );

            currbuy = currbuy->next;
        }
        else
            fprintf( outfile, "%12s %5s %6s", blanks, blanks, blanks );


        if( currsell != NULL ) {
            fprintf( outfile, "     %12s %5d %6.2f\n",
                    (currsell->name),
                    (int)(currsell->amount), (float)(currsell->price) );

            currsell = currsell->next;
        }
        else
            fprintf( outfile, "\n" );

        if( ++i > SCRNLINES ) {
	        cmd = get_disp_cmd();
	        if( cmd == 'n' )
	            i = 0;
    	    else
	            break;
    	}

    } /* looping over orders in ordbook */

    if( ( i==0 ) && ( cmd != 'n' ) ) fprintf(outfile,
        "**** order book is empty ****\n" );

    /* close files and free order book */
    if( outfile != stdout )
        fclose( outfile );
    free_ordbook( best_limbid, best_limask );

} /* end of disp_orders() */
/*---------------------------------*/

void disp_order_summary( char *security ) {

    /* Display a summary of the order books: count how many shares are
     *   bid and offered at each price level, without listing individuals'
     *   orders separately.  Kind of a frequency count.
    */

    int bidlev=0, asklev=0,  /* number of price levels on bid and ask sides */
        i;
    FILE *outfile;
    ORDER *best_limbid, *best_limask;
    ORDERBOOK *bidbook, *askbook;  /* frequency counts */
    SECURITY *sec;

    sec = find_sec( security, Securitydata );

    /* open files and load order book */
    load_ordbook( &best_limbid, &best_limask, security );
    outfile = stdout;

    /* allocate and fill ORDERBOOKs */
    bidbook = (ORDERBOOK *)x_calloc( (size_t)MAXDEPTH, sizeof(ORDERBOOK) );
    askbook = (ORDERBOOK *)x_calloc( (size_t)MAXDEPTH, sizeof(ORDERBOOK) );
    bidlev = fill_orderbook( best_limbid, bidbook );
    asklev = fill_orderbook( best_limask, askbook );

    /* display */
    if( MAX( bidlev, asklev ) == 0 )
        fprintf( outfile, "Order book is empty.\n" );

    else {
        fprintf( outfile, "         *** limit order book -- %s (prices in %ss) ***\n", security, sec->currency );
        fprintf( outfile, "             ***Bids***                  ***Offers***\n" );

        for( i=0; i < MAX(bidlev, asklev); i++ ) {
            if( i < bidlev )
                fprintf( outfile, "             %5d %6.2f        ",
                    (int)bidbook[i].amount, (float)(bidbook[i].price) );
            else
                fprintf( outfile, "                                 " );
            if( i < asklev )
                fprintf( outfile, "          %5d %6.2f    \n",
                    (int)askbook[i].amount, (float)(askbook[i].price) );
            else
                fprintf( outfile, "\n" );
        }

        if( i==MAXDEPTH )
            fprintf( outfile, "Bottommost listing represents all remaining price levels\n" );
    }

    /* close files and free allocated memory */
    if( outfile != stdout )
        fclose( outfile );
    free_ordbook( best_limbid, best_limask );
    x_free( (char *)bidbook );
    x_free( (char *)askbook );

} /* end of disp_order_summary() */
/*--------------------*/

static int fill_orderbook( ORDER *best_ord, ORDERBOOK *orderbook ) {
  /* Fill an ORDERBOOK array with frequency counts of one side of
   *   the order book.
   * Return the number of price levels (maximum of MAXDEPTH).
  */
  int lev=0;  /* return value */
  double currprice;
  ORDER *currorder;

  currorder = best_ord;

  while( (currorder != NULL) && (lev < MAXDEPTH-1) ) {

      lev++;

      /* collect all orders at currlevel's price */
      currprice = currorder->price;
      orderbook[lev-1].price = currprice;

      while(  (currorder != NULL ) && (currprice==currorder->price) ) {
          orderbook[lev-1].amount += currorder->amount;
          currorder = currorder->next;
      }
  }

  /* now count everything else into the MAXDEPTH-1 level */
  if( currorder != NULL ) {
      lev++;
      orderbook[MAXDEPTH-1].price = 0;
      while( currorder != NULL ) {
          orderbook[MAXDEPTH-1].amount += currorder->amount;
          currorder = currorder->next;
      }
  }

  return lev;
} /* end of fill_orderbook() */
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
