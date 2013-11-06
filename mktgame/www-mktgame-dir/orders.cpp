/* handle orders */

/**** INCLUDES ****/
#include <iostream.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmem.hpp"
#include "aqc.hpp"

/**** LOCAL FUNCTION DECLARATIONS ****/
static void dump_orders( int n, Order *best, FILE *ordbook );
static void enqueue( Order *latestord, Order **bestbid, Order **bestask );
static int higher_price_comp( Order *x, Order *y );
static void increment_numorders( char *name, char *security );
static int lower_price_comp( Order *x, Order *y );
static Order *read_orderlist( FILE *ordbook );

/**** EXTERNALLY VISIBLE FUNCTIONS ****
int 	count_orders( Order *best );
void    dump_ord( Order *latestord, Holdings *holding, char *security );
void 	dump_ordbook( Order *best_limbid, Order *best_limask, char *security );
Order   *find_just_better( Order *bestord, Order *latestord );
void    free_ordbook( Order *best_limbid, Order *best_limask );
void    handle_order( Order *latestord, char *security, int call_server );
void 	load_ordbook( Order **best_limbid, Order **best_limask,
                      char *security );
int     nameok( Names *user, Names *names, int n );
****/

/**** FUNCTION DEFINITIONS ****/

static void enqueue( Order *latestord, Order **bestbid, Order **bestask ) {
	/* Place latestord in its proper position in the order book.
     * We work with pointers to the lists' heads because we may alter these
     *   here. */
    Order **bestord, /* generic; bestbid or bestask, as appropriate */
          *tempord,
          *just_better;

    /* classify order as to bid or ask */
    if( latestord->buy_or_sell == BUY )
    	bestord = bestbid;
    else
    	bestord = bestask;

    /* Put it on the appropriate linked list.  The heads of the lists are
     *   the best orders.  The worst order's next field equals NULL.
     *   The best order's prev field equals NULL */

    if( *bestord == NULL ) 		/* empty list */
    	*bestord = latestord;

    else { 		/* find latestord's appropriate place, and splice */
       	just_better = find_just_better( *bestord, latestord );
        /* just_better is the order that will clear just before latestord */

        if( just_better == NULL ) {   /* latestord is best */
       	    latestord->next = *bestord;
            latestord->prev = NULL;   /* superfluous--set in alloc_order() */
            (*bestord)->prev = latestord;
            *bestord = latestord;
        }
        else {
	    tempord = just_better->next;
	    just_better->next = latestord;
            latestord->prev = just_better;
	    latestord->next = tempord;
            if( tempord != NULL )
                tempord->prev   = latestord;
        }
    }

} /* end of enqueue() */
/*--------------------------*/

Order *find_just_better( Order *bestord, Order *latestord ) {
    /* Find the order just one better than latestord, i.e. the one that
     *   will clear just before it.  Look on the side of the order
     *   book that's headed by bestord.
     * If latestord is a bid, look for bid price just higher; if ask,
     *   just lower.  The OrderS are on a linked list that's already
     *   sorted so its head is the best order there.
    */
    Order *currord;
    int (*price_comp)( Order *x, Order *y );
    /* returns 1 if price of x is "better" than of y, 0 if equal, 1 if worse */

    currord = bestord;

    if( latestord->buy_or_sell == BUY )
    	price_comp = higher_price_comp;
    else
    	price_comp = lower_price_comp;

    /* The first value currord takes is the bestord.  So if latestord
     *   is better than that, it goes to the head of the list, as "best" */
    if( (currord == NULL) || ( price_comp( currord, latestord ) == -1 ) )
    	return NULL;

    while( (currord != NULL) && ( currord->next != NULL )
    &&     ( price_comp( currord->next, latestord ) != -1 ) )
    	currord = currord->next;

        return currord;

} /* end of find_just_better() */
/*--------------------------*/

static int higher_price_comp( Order *x, Order *y ) {
    /* Returns 1 if price of x is higher than of y, 0 if equal, 1 if worse.
     * Used by find_just_better() */
    int result;

    if( (x==NULL) || (y==NULL) )
        return -9; 

    if( x->price > y->price )
    	result = 1;
    if( x->price == y->price )
    	result = 0;
    if( x->price < y->price )
    	result = -1;

    return result;
} /* end of higher_price_comp() */
/*--------------------------*/

static int lower_price_comp( Order *x, Order *y ) {
	return higher_price_comp( y,x );
} /* end of lower_price_comp() */
/*--------------------------*/

void dump_ordbook( Order *best_limbid, Order *best_limask,
                   char *security ) {
  /* dump it in binary */
    char ordbookname[ACTNAMELEN];
    int n_limbids=0, n_limasks=0;
    FILE *ordbook;

    /* count how many there are of each kind of order */
    n_limbids = count_orders( best_limbid );
    n_limasks = count_orders( best_limask );

    /* open the file that holds the order book in binary form */
    ordbook_name( security, ordbookname );
    ordbook = fopen( ordbookname, "wb" );

    dump_orders( n_limbids, best_limbid, ordbook );
    dump_orders( n_limasks, best_limask, ordbook );

    fclose( ordbook );

} /* end of dump_ordbook() */
/*--------------------------*/

static void dump_orders( int n, Order *best, FILE *ordbook ) {
    /* dump one of the two lists of orders.
     * First, write the number of orders, then dump them out starting
     *   with the best one.
    */
    int i;
    Order *currorder;

    fwrite( &n, sizeof(int), 1, ordbook );
    currorder = best;
    for( i=0; i<n; i++ ) {
    	fwrite( currorder, sizeof(Order), 1, ordbook );
        currorder = currorder->next;
    }

} /* end of dump_orders() */
/*--------------------------*/

int count_orders( Order *best ) {
	int i=0;
    Order *currorder;

    currorder = best;

    while( currorder != NULL ) {
    	currorder = currorder->next;
        i++;
    }

    return i;
} /* end of count_orders() */
/*--------------------------*/

void load_ordbook( Order **best_limbid, Order **best_limask,
                   char *security ) {
	/* Load them as dumped by dump_ordbook.  Be sure to set the
     *   ->next and -prev fields, as the ones in the file won't be correct.
     *
     * If there's no file, this function returns with no effects on
     *   anything whatsoever.
    */
    char ordbookname[ACTNAMELEN+1];
    FILE *ordbook;

    /* open the ordbook file */
    ordbook_name( security, ordbookname );
    ordbook = fopen( ordbookname, "rb" );
    if( ordbook==NULL ) {
        *best_limbid = *best_limask = NULL;  /* disporders needs this */
    	return;
    }

    *best_limbid = read_orderlist( ordbook );
    *best_limask = read_orderlist( ordbook );

    fclose( ordbook );

} /* end of load_ordbook() */
/*--------------------------*/

static Order *read_orderlist( FILE *ordbook ) {
	/* Reads any one of the two linked lists of orders output by
     *   dump_ordbook().
     * Allocates space for all the Orders.
	 * Returns "best" order, i.e. the head of the list.
    */
    int i, n;
    Order *best, *currorder;

    /* read in the number of orders, and prompt if it's strange. */
    fread( &n, sizeof(int), 1, ordbook );
    if( (n>1000) || (n<0) ) error( "Order book corrupted.  Call Sternberg." );

    if( n==0 )
    	best = NULL;

    else {

    	best = order_alloc();
        fread( best, sizeof(Order), 1, ordbook );
        currorder = best;

	    for( i=1; i<n; i++ ) {
        	currorder->next = order_alloc();
            fread( currorder->next, sizeof(Order), 1, ordbook );
            (currorder->next)->prev = currorder;
            currorder = currorder->next;
        }
    }

    return best;
} /* end of read_ordlist() */
/*--------------------------*/

void handle_order( Order *latestord, char *security, int call_server ) {
  /* Enqueue limit orders, and try to execute market orders.
   * We don't get here unless the order was placed by a valid player,
   *   and the order is not NULL.
   * When done, look at the short-money list and place market sell orders
   *   for those people, on the bond1 market.  We put this at the end,
   *   after the server gets released.
   * When call_server = YES, we get the server's attention before executing
   *   the order.  We set call_server to NO when handle_order() is called
   *   from forced_sale() (i.e. recursively).
  */
  int    handle;  /* of the socket */
  Trade *newtrade;
  Order *best_limbid, *best_limask;
  Holdings *holding;     /* holdings of person who placed latestord */

  if( call_server == YES ) {
      /* get server's attention */
      handle = server_avail( latestord->name );
      if( handle < 0 ) {
          client_warning( "Server unavailable." );
          order_free( latestord ); latestord = NULL;
          return;
      }

      /* if we get to here, the server is going to get to us, either right
       * away, or very soon */
      server_ready( handle );  /* this returns when the server is really ready */
  }

  /* load order book */
  load_ordbook( &best_limbid, &best_limask, security );

  /* ready to execute and record the order.  Free server when done */
  if( latestord->mkt_or_lim == LIM ) {
      /* update the limit order book */
      enqueue( latestord, &best_limbid, &best_limask );

      /* record order in the .act file */
      holding = load_holding( latestord->name, security );
      time( &(holding->zman) );
      dump_ord( latestord, holding, security );

      /* notify */
      if( call_server == YES )
          printf("Your order has been added to the order book.\n");
  }

  else {    /* market order */
      /* record order in the .act file */
      holding = load_holding( latestord->name, security );
      time( &(holding->zman) );
      dump_ord( latestord, holding, security );

      /* cross with best limit order(s) */
      if( latestord->buy_or_sell == BUY )
       	  newtrade = cross( latestord, &best_limask, security );
      else
          newtrade = cross( latestord, &best_limbid, security );

      if( call_server == YES )
          notify( newtrade, security );
          /* send him confirmation. newtrade is the trade of the party
           * placing latestord, i.e. the market order.  We don't want to
           * print notification, however, if the market order is a forced
           * sale. */

      order_free( latestord ); latestord = NULL;
  }

  /* increment the counter that tracks how many orders this player has
   *   placed.  By conditioning on call_server==YES we exclude forced
   *   sales, option exercises and forward contract deliveries.
  */
  if( call_server==YES )
      increment_numorders(  holding->name, security );

  /* dump and free order book, free holding */
  dump_ordbook( best_limbid, best_limask, security );
  free_ordbook( best_limbid, best_limask );
  holding_free( holding );

  /* place sales in the bond1 market for people on the short-money list.
   * Do it before we free the server, or there's an outside chance
   *   the forced sale gets stacked behind >8 other orders, and fails
   *   to execute.
   * Call forced_sale() only the first time you get into handle_orders();
   *   it goes through the entire short-money list, and there's no sense
   *   making it recursive when there's no need for that.
  */
  if( call_server == YES ) {
      forced_sale();
      server_goodbye( handle );
  }

} /* end of handle_order() */
/*--------------------------*/

int nameok( Names *user, Names *names, int n ) {
  /* Check if name is among the names in the holding array (which
   *   has n elements).  You can count on holding being sorted by name.
   * Check if password is ok, too.  Fed's password works as a skeleton
   *   key, opening everyone's account.
   * Return 1 if name and password check out, 0 otherwise.
  */
  int result;
  Names *foundit,  /* return value from bsearch() */
        *Fed,
        *key;

  /* allocate key and put latestord->name in its name field */
  key = (Names *)x_calloc( (size_t)1, sizeof(Names) );
  strcpy( key->name, user->name );

//cout << "Looking for name " << user->name << '\n';

  foundit = (Names *)bsearch( (char *)key, (char *)names, (size_t)n,
		    sizeof(Names), namescomp  );

  if( foundit == NULL )   /* didn't find name */
      error( "You don't have an account here." );

  else {
      strcpy( key->name, "Fed" );
      Fed = (Names *)bsearch( key, names, (size_t)n,
		    sizeof(Names), namescomp );

//	cout << "user->pwd = " << user->pwd << " " << strlen(user->pwd) << "<p>\n";
//	cout << "foundit->pwd = " << foundit->pwd << 
//		" " << strlen(foundit->pwd) << "<p>\n";
//	cout << "Fed->pwd = " << Fed->pwd << " " << strlen(Fed->pwd) << "<p>\n";

      if( ( strcmp( user->pwd, foundit->pwd ) )
      &&  ( strcmp( user->pwd, Fed->pwd ) ) )	 
	      error( "Wrong password." );
      else  /* now everything is fine */
	      result = 1;
  }

  return result;
} /* end of nameok() */
/*--------------------------*/

int namescomp( const void *names1, const void *names2 ) {
  /* for use in nameok(), for comparing name fields */
  char *name1, *name2;

  name1 = ((Names*)names1)->name;
  name2 = ((Names*)names2)->name;
//	cout << "namescomp( " << name1 << ", " << name2 << " )\n";
  return strcmp( name1, name2 );
} /* end of namescomp() */
/*--------------------*/

void dump_ord( Order *latestord, Holdings *holding, char *security ) {
  /* write (binary) the latest order to the .act file that holds the
   *   whole history of this player's activities--orders, trades, holdings.
  */
  char actfilename[ACTNAMELEN];  /* accounts/<name>.act */
  FILE *actfile;
  Trade *dummytrade;

  /* open the file for appending in binary mode */
  actfile_name( latestord->name, security, actfilename );
  actfile = fopen( actfilename, "ab" );
  if( actfile==NULL ) error( "Orders.c: no .act file!!" );

  /* allocate dummy trade and holdings */
  dummytrade = trade_alloc();

  /* write */
  fwrite( latestord, sizeof(Order), 1, actfile );
  fwrite( dummytrade, sizeof(Trade), 1, actfile );
  fwrite( holding, sizeof(Holdings), 1, actfile );

  /* close */
  fclose( actfile );

  /* de-allocate */
  trade_free( dummytrade ); dummytrade = NULL;

} /* end of dump_ord() */
/*--------------------*/

void free_ordbook( Order *best_limbid, Order *best_limask ) {

  Order *currorder, *nextorder;

  /* free the bids */
  currorder = best_limbid;
  while( currorder != NULL ) {
      nextorder = currorder->next;
      order_free( currorder );
      currorder = nextorder;
  }

  /* free the asks */
  currorder = best_limask;
  while( currorder != NULL ) {
      nextorder = currorder->next;
      order_free( currorder );
      currorder = nextorder;
  }
} /* end of free_ordbook() */
/*--------------------*/

static void increment_numorders( char *name, char *security ) {
  char actfilename[ACTNAMELEN+1];
  FILE *actfile;
  Actheader actheader;

    /* increment numorders field in actheader */
    actfile_name( name, security, actfilename );
    actfile = fopen( actfilename, "rb+" );
    if( actfile==NULL ) error( "Bldordr.c: no .act file!!" );
    fseek( actfile, 0L, SEEK_SET );
    fread( &actheader, sizeof(Actheader), 1, actfile );
    actheader.numorders ++;  /* increment...and write back... */
    fseek( actfile, 0L, SEEK_SET );
    fwrite( &actheader, sizeof(Actheader), 1, actfile );
    fclose( actfile );
} /* end of increment_numorders() */
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
/*--------------------*/
