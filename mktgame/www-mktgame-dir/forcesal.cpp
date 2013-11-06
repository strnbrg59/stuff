/**** INCLUDES ****/
#include <math.h>    /* for ceil() */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xmem.hpp"
#include "aqc.hpp"

/**** TYPEDEF ****/
typedef struct SHORTMONEY {
    char   name[NAMELEN+1];
    char   currency[NAMELEN+1];
    struct SHORTMONEY *next;
} SHORTMONEY;

SHORTMONEY Root;  /* root of the short-money list.  Needs to be global
 *   (actually, static) so there'll be only one such list despite recursive
 *   calls to forced_sale().
 * Root itself won't hold any data per se; if there are forced sales to
 *   be done, they will be indicated by Root.next, Root.next->next, etc.
 *   Set Root.next to NULL if no one is in the red.
*/

/**** FUNCTIONS VISIBLE EXTERNALLY
int check_neg_money( char *name, char *currency );
double latest_money( char *name, char *currency );
****/

/**** LOCAL FUNCTION DECLARATIONS ****/
static int find_units_needed( Order *best_limbid, double money_needed,
                              char *name );

/**** FUNCTION DEFINITIONS ****/
void forced_sale( void ) {
    char name[NAMELEN+1], *forcemkt;
    int units_needed;   /* that need to be sold to raise money_needed */
    double money_needed; /* money that needs to be raised */
    Order *best_limbid, *best_limask, *forced_order;
    Holdings *latestmoney;
    SHORTMONEY *tempptr;
    Security *sec;

/* We need to keep track of where we are on the short-sale list, despite
 * recursive calls to this function.
*/
  /* place sales in the FORCEMKT market for people on the short-money list */

  /* Check the short-money list.  If no one's on it, return;
   * there are no forced sales to do.
  */
  while( Root.next != NULL ) {

      /* get person's name and amount of money he needs to raise */
      strcpy( name, Root.next->name );
      latestmoney = load_holding( name, Root.next->currency );
      money_needed = -latestmoney->amount;
      /* We're going to have to look at his money position in all the
       * currencies */

      /* see if he needs to raise any money at all; his situation may have
       * changed since check_neg_money() put his name on the list
      */
      if( money_needed > 0 ) {

          /* find the appropriate market to do forced sales in */
          sec = find_sec( Root.next->currency, G::securitydata );
          forcemkt = sec->forcemkt;

          /* find necessary number of bond1s for person to sell. */
          /* 1. load the order book for the forced sale market */
          load_ordbook( &best_limbid, &best_limask, forcemkt );

          /* 2. calculate how much needs to be sold */
          units_needed = find_units_needed( best_limbid, money_needed, name );

          /* 3. build the appropriate market order for him. */
          forced_order = create_order( name, MKT, SELL, units_needed, 0 );
          forced_order->comment = 'F';

          /* 4. free order book */
          free_ordbook( best_limbid, best_limask );

          /* 5. pass the order to handle_order() (and set call_server to NO) */
          handle_order( forced_order, forcemkt, NO );
      } /* if money_needed > 0 */

      /* take name off the short-money list */
      tempptr = (Root.next)->next;
      x_free( (char *)Root.next );
      Root.next = tempptr;

  } /* while Root.next != NULL */

} /* end of forced_sale() */
/*----------------------*/

int check_neg_money( char *name, char *currency ) {
    /* returns 1 if name's money holdings are negative, zero otherwise.
     * Side effect: appends names to the short-money list.  Doesn't fill
     *   in the money_needed field (leaves that to forced_sale(), at the
     *   last minute) because interim transactions (between the time this
     *   function is called and the time the forced sale is actually set
     *   up and executed) could affect name's money position.  The most
     *   common such occurence is this: name is forced into a sale of
     *   bond1's, but he's also on the bid side in the bond1 mkt--not at
     *   the top of the list, but high enough that his own mkt order will
     *   cross with his limit order.  Now,
     *   cross() calls this function for the limit side before the mkt
     *   side, so just after name (as limit side) has bought (from himself
     *   as mkt side) it looks like he's deeper in the red than ever before.
     *
     * Called from cross().
    */
    int result;
    Holdings *latestmoney;
    SHORTMONEY *currptr;

    latestmoney = load_holding( name, currency );
    if( latestmoney->amount >= 0 )
        result = 0;

    else {  /* negative money!  need to take action */
        result = 1;

        /* working forward from Root, find last entry in short-money list */
        currptr = &Root;
        while( currptr->next != NULL ) currptr = currptr->next;

        /* allocate and append a new SHORTMONEY structure */
        currptr->next = (SHORTMONEY *)x_calloc( (size_t)1, sizeof(SHORTMONEY) );
        strcpy( currptr->next->name, name );
        strcpy( currptr->next->currency, currency );

    }

    return result;
} /* end of check_neg_money() */
/*--------------------*/

static int find_units_needed( Order *best_limbid, double money_needed,
                              char *name ) {
  /* return number of units of forced-sale security that have to be sold
   * to raise money_needed.  If there isn't enough on the order book to raise
   * money_needed, return the total number of shares on the order book.
   * (So you'd better be sure to put lots of orders there for the Fed
   * account.)
  */
  double units_needed=0, money_found=0;

  while( (money_found < money_needed) && ( best_limbid != NULL ) ) {
      if( !strcmp( best_limbid->name, name ) )
        /* If the person is going to mkt sell to his own limit buy order in
         * the bond1 mkt, have him clear with the whole block at once, other-
         * wise we get into a possibly long sequence of buy-1-sell-1.
        */
          units_needed += best_limbid->amount;
      else {
          money_found += best_limbid->amount * best_limbid->price;
          if( money_found <= money_needed )
              units_needed += best_limbid->amount;
          else
              units_needed += ceil(
                  best_limbid->amount
                - (money_found - money_needed)/best_limbid->price );
      }
     best_limbid = best_limbid->next;
  }

  return (int)units_needed;
} /* end of find_units_needed() */
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
