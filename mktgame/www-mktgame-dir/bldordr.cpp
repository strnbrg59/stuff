/* Build up an Order record by prompting user for what he wants to do. */

/**** INCLUDES ****/
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "aqc.hpp"
#include "xmem.hpp"

static char *Buy_or_sell[2] = { "buy", "sell" };

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void build_order( char *name, String pwd, char *security );
Order *create_order( char *name, int mkt_or_lim, int buy_or_sell,
                   double amount, double price );
void disp_order( Order *order, FILE *outfile, char *security );
****/

/**** FUNCTION DEFINITIONS ****/
void build_order( char *name, String pwd, char *security ) {
  /* return NULL if player doesn't like his order, or if verify_order()
   *   rejects it, or if the player is up to his order limit as set out
   *   in security.par.
  */
  /* prevent limit sell orders ahead of the firm--firms are identified
   * by their Security.section being 0.  (That's true of Fed and poisson,
   * too, but it doesn't matter.  We don't want anyone to step in front
   * of the firm, so as to prevent monkey-business leading to the firm's
   * being unable to raise the money it needs in an IPO.)
  */

	// WWW version: deal with all those abnormal conditions in the
	// CGI program that is the ACTION of the form this function
	// prints out.


    // check if the person has placed too many orders 
    if( at_orderlimit( name, security ) == YES ) 
        error( "You have reached your order limit in this security.\n" );

	cout <<
		"<title>Build Order</title>\n" <<
		"<h1>Build Order</h1>\n" <<
	
		"<FORM 
		ACTION=\"http://" << LOCALHOST << CGIBINDIR << "CGIbldordr\"
		METHOD=\"POST\"> \n" <<
		
		"<INPUT NAME=\"username\" TYPE=\"hidden\" VALUE=\"" <<
		name << "\">\n" <<

		"<INPUT NAME=\"pwd\" TYPE=\"hidden\" VALUE=\"" <<
		pwd << "\">\n" <<

		"<INPUT NAME=\"security\" TYPE=\"hidden\" VALUE=\"" <<
		security << "\">\n";

	print_file( "build_order-form.html" );

	cout << "</FORM>\n";

} // build_order()
//----------------------

void disp_order( Order *order, FILE *outfile, char *security ) {
    /* display it in human-readable form */
    int b_or_s;
    Security *sec;

    sec = find_sec( security, G::securitydata );

    b_or_s = order->buy_or_sell;

    fprintf( outfile, "%s %5d %s",  Buy_or_sell[b_or_s],
	    (int)(order->amount), security );

    if( order->mkt_or_lim == MKT ) ;
    else
      fprintf( outfile, " at %4.2f %s per %s.",
          (float)(order->price), sec->currency, security );

} /* end of disp_order() */
//-----------------------------

int verify_order( Order *order, char *security ) {
  /* free *order if user doesn't like it.
   * Also, if order is unreasonably large, expensive, or zero */
  int ok;  /* return value */
  Order *bestask, *bestbid;
  Security *sec;
  char firm[5];

  strncpy( firm, order->name, 4 );
  firm[4]=0;

  sec = find_sec( security, G::securitydata );
  load_ordbook( &bestbid, &bestask, security );

//printf( "\nIs that ok? (y|n)" );
//scanf( "%s", charstr );

	ok = 0;

	// Check for positivity of shares and price.
	if( (order->amount<=0)
	||  ( (order->price==0) && (order->mkt_or_lim == LIM) ) ) {
		client_warning( 
			"Number of units must be positive, and price non-zero." );
		ok=-1;
	}

      // disqualify unreasonably large or expensive orders.
	if( (ok != -1) &&
        ( strcmp(firm, "firm") && strcmp(order->name, "Fed")) &&
        ( ( ( ismoney(sec)==NO ) && (order->amount > TOOMANY  ) )
       || ( ( ismoney(sec)==YES) && (order->amount > TOOMANY*10) ) ) )
      {
        	client_warning( "Eep! Too many! " );
			ok = -1;
      }

	// Prevent limit asks placed above best bid, or vice versa.  
	//	value_portf() relies on a sensible order book that isn't
	//  easily manipulated to give unreasonable midspread numbers.
	// In the past, we've just been disqualifying such orders.
	//	Now, we'll turn them into market orders.  Can we just
	// 	recursively call this same function, after changing the
	//	type from limit to market?  No, because the calling function
	//	only expects an "ok" from this function, and then calls
	//	another function to execute the order.

	if( ( (order->mkt_or_lim == LIM) && (
		(
			( order->buy_or_sell == BUY) && 
			( bestask && ( order->price >= bestask->price))
		)
			|| 
		(
			( order->buy_or_sell == SELL) && 
			( bestbid && ( order->price <= bestbid->price))
		))
		)
	&& ( ok != -1 ) )
		{
//			client_warning( "It would be better to place a market order.");
			cout << "Converted to a market order...<BR>\n";
			order->mkt_or_lim = MKT;		
			ok = +1;
      }

      if( ok == -1 )
          order_free( order );

	  free_ordbook( bestbid, bestask );

	  return ok;
} // end of verify_order() 
//-----------------------------

Order *create_order(
    char name[NAMELEN+1],   /* player's name (8 chars + 0x0) */
    int mkt_or_lim,    		/* MKT|LIM */
    int buy_or_sell,   		/* BUY|SELL */
    double amount,     		/* unsigned */
    double price ) {
/* returns an Order, with zman set to current time */

  Order *result;
  result = order_alloc();

  strcpy( result->name, name );
  result->mkt_or_lim = mkt_or_lim;
  result->buy_or_sell = buy_or_sell;
  result->comment = ' ';
  result->amount = amount;
  result->price = price;
  time( &(result->zman) );

  return result;
} /* end of create_order() */
/*--------------------*/

int at_orderlimit( char *name, char *security ) {
// Returns YES if name has reached the maximum allowable orders for
//	security.  returns NO otherwise.
//	Players "Fed" and "poisson" are exempt.
//	Player "guest" gets no orders at all.

  char actfilename[ACTNAMELEN+1];
  FILE *actfile;
  Actheader actheader;
  Security *sec;

	if( (!strcmp( name, "Fed" )) || (!strcmp( name, "poisson" )) )
		return NO;
	if( !strcmp( name, "guest" ) )
		return YES;

  /* read the file header, to see how many orders have already been placed */
  actfile_name( name, security, actfilename );
  actfile = fopen( actfilename, "rb" );
  if( actfile==NULL ) error( "bldordr.c: no .act file!!" );
  fread( &actheader, sizeof(Actheader), 1, actfile );
  fclose( actfile );

  /* find out how many orders are allowed in this security */
  sec = find_sec( security, G::securitydata );

  if( sec->maxorders <= actheader.numorders )
      return YES;
  else
      return NO;

} /* end of at_orderlimit() */
/*--------------------*/

int front_of_firm( Order *order, char *security ) {
  /* returns YES if order is a limit order better than one placed by
   * the firm (any player with section # 0 counts as a firm).
   * We need this feature so saboteurs don't hinder the firm's ability
   * to raise money from a securities issue.
  */
  int result,i;
  Order *bestask, *bestbid;

  load_ordbook( &bestbid, &bestask, security );

  if( (bestask==NULL) || (order->price >= bestask->price)
  || (order->mkt_or_lim==MKT) || (order->buy_or_sell==BUY) )
      result = NO;
  else {
      /* get pointer to Names element with info on who placed the best ask */
      i=0;
      while( strcmp( bestask->name, G::names[i].name ) ) i++;
      if( G::names[i].section == 0 )
          result = YES;
      else
          result = NO;
  }

  free_ordbook( bestbid, bestask );
  return result;
} /* end of front_of_firm() */
