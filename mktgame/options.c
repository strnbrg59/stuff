/* Option exercise.
*   We can implement most of this stuff with what we have now.  We need
* to create a temporary limit order book, in which we place limit sells on
* behalf of the randomly selected players with short option positions.  These
* limit sells' ask prices are all equal to the option's exercise price.
* Then we place a market buy on behalf of the player exercising his option.
* All that's left is to reset players' holdings of the options themselves.
*      The temporary limit order book should be made to have the same name
* and path as the real order book, so handle_order() doesn't need to do or
* suspect anything out of the ordinary.  So exercise_option() could copy the
* true order book to a backup file, before setting up the temporary order
* book.
*      The nice thing about using a dummy order book and crossing trades
* with it is that all the usual record-keeping gets done in the familiar
* way.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqc.h"
#include "xmem.h"

/**** GLOBAL VARIABLE ****/
extern int N_players;
extern NAMES *Names;

/**** TYPEDEFS ****/
typedef struct CHOSEN_ONES {
    char name[NAMELEN+1];   /* player's name */
    int  units;             /* units (short) that are going to be exercised against him */
} CHOSEN_ONES;

/**** LOCAL FUNCTION DECLARATIONS ****/
static CHOSEN_ONES *choose_shorts( char *option, int units );

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void exercise_option( char *, char *, char *, double, char call_or_put );
****/

/**** FUNCTION DEFINITIONS ****/
void exercise_option(
  char *name,         /* player exercising the option */
  char *option,       /* name of option security */
  char *underlying,   /* name of underlying security */
  double x,           /* exercise price */
  char call_or_put )   /* set to CALL or PUT */
{
  char u_ordbookname[ACTNAMELEN+1],  /* order book for underlying */
       o_ordbookname[ACTNAMELEN+1];  /* order book for option */
  int i, units, errcode, handle;
  ORDER *mkt_order, *lim_order;
  HOLDINGS *holdings, *holding;
  CHOSEN_ONES *chosen_ones;
  FILE *infile;  /* just for checking if a file exists */


  /*    0. prompt for number of units to exercise */
  printf( "Enter number of units to exercise: " );
  scanf( "%d", &units );
  printf( "\n" );

  /*    0.1. Check for feasibility; is the player long enough options? */
  holdings = load_holdings( Names, option );
  holding  = find_holding( name, holdings );
  if( holding->amount < units ) {
      client_warning( "You don't own that many." );
      return;
  }

  /*    1. get ahold of the server, just as for executing a trade */
  if( 0 > (handle = server_avail()) ) {
          client_warning( "Server unavailable." );
          return;
  }
  server_ready( handle );  /* this returns when the server is really ready */

  /*    2.1 move order book file for *underlying and *option to a backup file*/
    /* first for underlying */
  infile=fopen("delete.me","r");
  if( infile ) remove( "delete.me" );
  ordbook_name( underlying, u_ordbookname );
  errcode = rename( u_ordbookname, "delete.me" );

  /* then for option */
  infile=fopen("delete.me2","r");
  if( infile ) remove( "delete.me2" );
  ordbook_name( option, o_ordbookname );
  errcode = rename( o_ordbookname, "delete.me2" );

  /*    2.2 line up enough people with short positions in the option. */
  chosen_ones = choose_shorts( option, units );

  /*    2.3 place limit orders for them, on the temporary order books. */
  i=0;
  while( chosen_ones[i].name[0] != 0 ) {
    /* first for underlying */
      if( call_or_put == 'c' )
          lim_order = create_order( chosen_ones[i].name, LIM, SELL,
                        chosen_ones[i].units, x );
      else
          lim_order = create_order( chosen_ones[i].name, LIM, BUY,
                        chosen_ones[i].units, x );

      lim_order->comment = 'E';
      handle_order( lim_order, underlying, NO );

    /* then for option */
      lim_order = create_order( chosen_ones[i].name, LIM, BUY,
                    chosen_ones[i].units, 0.0000000001 );
      lim_order->comment = 'E';
      handle_order( lim_order, option, NO );

      i++;
  }

  /*    2.4 place market orders for *name, using handle_order(). */
    /* first for underlying */
  if( call_or_put == 'c' )
      mkt_order = create_order( name, MKT, BUY, units, 0 );
  else
      mkt_order = create_order( name, MKT, SELL, units, 0 );
  mkt_order->comment = 'E';
  handle_order( mkt_order, underlying, NO );

    /* then for option */
  mkt_order = create_order( name, MKT, SELL, units, 0 );
  mkt_order->comment = 'E';
  handle_order( mkt_order, option, NO );

  /*    2.5 restore true limit order books */
    /* first for underlying */
  remove( u_ordbookname );  /* it exists; there was the dummy order */
  errcode = rename( "delete.me", u_ordbookname );
     /* then for option */
  remove( o_ordbookname );
  errcode = rename( "delete.me2", o_ordbookname );
  errcode ++;  /* just so compiler doesn't say we don't do anything with it */

  /*    3. call forced_sale(); handle_order() won't call it because we're
   *       calling handle_order with call_server set to NO.
  */
  forced_sale();

  /*    4. release server. */
  server_goodbye( handle );

  /*    5. free storage */
  x_free( (char *)chosen_ones );

} /* end of exercise_option() */
/*-----------------------*/

static CHOSEN_ONES *choose_shorts( char *option, int units_needed ) {
  /* find, at random, enough people short the option security for the
   * long party to exercise units units of it against them.
  */
  int *permut, units_found=0,
       i=0, /* indexes holdings[] */
       j=0; /* indexes result[] */
  CHOSEN_ONES *result;
  HOLDINGS *holdings, *holding;

  result = (CHOSEN_ONES *)x_calloc( N_players, sizeof(CHOSEN_ONES) );

  /* produce a permutation of {0,...,N_players-1}. */
  permut = permute( N_players );

  /* using the order in the permutation, pick players until you have
   * enough short positions to cross with
  */
  holdings = load_holdings( Names, option );
  while( units_found < units_needed ) {
      holding = holdings + permut[i];

      if( holding->amount < 0 ) {
          strcpy( result[j].name, holding->name );
          result[j].units = MIN( units_needed - units_found, -(int)(holding->amount) );
          units_found -= (int)(holding->amount);
          j++;
      }

      i++;
  }

  x_free( (char *)permut );
  return result;
} /* end of choose_shorts() */
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
