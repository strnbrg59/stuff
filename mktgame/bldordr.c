/* Build up an ORDER record by prompting user for what he wants to do. */

/**** INCLUDES ****/
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "aqc.h"
#include "xmem.h"

/**** GLOBAL VARIABLES ****/
extern SECURITY **Securitydata;
extern NAMES *Names;
static char *Buy_or_sell[2] = { "buy", "sell" };

/**** LOCAL FUNCTION DECLARATIONS ****/
static int at_orderlimit( char *name, char *security );
static int front_of_firm( ORDER *order, char *security );
static ORDER *prompts( char *name, char *security );
static int verify_order( ORDER *order, char *security );

/**** FUNCTIONS VISIBLE EXTERNALLY ****
ORDER *build_order( char *name, char *security );
ORDER *create_order( char *name, int mkt_or_lim, int buy_or_sell,
                   double amount, double price );
void disp_order( ORDER *order, FILE *outfile, char *security );
****/

/**** FUNCTION DEFINITIONS ****/
ORDER *build_order( char *name, char *security ) {
  /* return NULL if player doesn't like his order, or if verify_order()
   *   rejects it, or if the player is up to his order limit as set out
   *   in security.par.
  */
  /* prevent limit sell orders ahead of the firm--firms are identified
   * by their SECURITY.section being 0.  (That's true of Fed and poisson,
   * too, but it doesn't matter.  We don't want anyone to step in front
   * of the firm, so as to prevent monkey-business leading to the firm's
   * being unable to raise the money it needs.
  */

    int ok = -1;
    ORDER *order=NULL;

    /* check if the person has placed too many orders */
    if( at_orderlimit( name, security ) == YES ) {
        printf( "You have reached your order limit in this security.\n" );
        return NULL;
    }

    order = prompts( name, security );

    if( order != NULL ) {

        if( front_of_firm( order, security ) == YES ) {
            printf( "You may not place an order in front of the firm's.\n" );
            return NULL;
        }

        disp_order( order, stdout, security );
        ok = verify_order( order, security );

        if( ok == -1 )   /* verify_order() has free'd order */
            order = NULL;
	    else
               time( &(order->zman) );  /* time stamp the order */
    }

    return order;
} /* end of build_order() */
/*--------------------------*/

static ORDER *prompts( char *name, char *security ) {
    /* prompt user and scanf info.
     * return NULL on error or if player wants to quit */
    char c, charnum[3*NAMELEN], charstr[3*NAMELEN];
    int mkt_or_lim, buy_or_sell;
    double price, amount, ticksize;
    ORDER *result;

    /* get mkt_or_lim */
    printf( "\nEnter M for market order, L for limit order: " );
    scanf( "%s", charstr );
    c = toupper( charstr[0] );
    if( c == 'M' )
        mkt_or_lim = MKT;
    else
        if( c == 'L' )
            mkt_or_lim = LIM;
            else {
                client_warning( "Error: type either M or L." );
                return NULL;
            }

    /* determine if it's a buy or a sell */
    printf("Enter B for buy, S for sell: ");
    scanf( "%s", charstr );  /* getchar() bombs--next getchar() gets '\n' */
    c = toupper( charstr[0] );
    if( c=='B' )
        buy_or_sell = BUY;
    else
        if( c=='S' )
	        buy_or_sell = SELL;
            else {
	        client_warning( "Error: type either B or S.");
                return NULL;
            }

    /* get number of units, or amount if security is a currency */
    if( ismoney(find_sec( security, Securitydata ))==NO )
        printf("Enter number of units: ");
    else
        printf("Enter amount you wish to trade: ");
    scanf( "%s", charnum );
    if( strlen( charnum ) > 3*NAMELEN ) error( "Too long." ); /* stops memory overwrite cold */
    amount = atof( charnum );
    amount = round(amount, 1.0 );

    /* get price */
    if( mkt_or_lim == MKT )
        price = 0;
    else {
        printf( "Enter price: " );
    	scanf( "%s", charnum );
            price = atof(charnum);
    }
    if( price != 0 ) {  /* set tick size */
        ticksize = 0.10;
        if( fabs(price) < 5 ) ticksize = 0.05;
        if( fabs(price) < 2 ) ticksize = 0.01;
        price = round( price, ticksize );
    }

    result = create_order( name, mkt_or_lim, buy_or_sell, amount, price );
    return result;
} /* end of prompts() */
/*--------------------------*/

void disp_order( ORDER *order, FILE *outfile, char *security ) {
    /* display it in human-readable form */
    int b_or_s;
    SECURITY *sec;

    sec = find_sec( security, Securitydata );

    b_or_s = order->buy_or_sell;

    fprintf( outfile, "%s %5d %s",  Buy_or_sell[b_or_s],
	    (int)(order->amount), security );

    if( order->mkt_or_lim == MKT )
      fprintf( outfile, ".\n" );
    else
      fprintf( outfile, " at %4.2f %s per %s.\n",
          (float)(order->price), sec->currency, security );

} /* end of disp_order() */
/*-----------------------------*/

static int verify_order( ORDER *order, char *security ) {
  /* free *order if user doesn't like it.
   * Also, if order is unreasonably large, expensive, or zero */
  char charstr[5];
  int ok;  /* return value */
  ORDER *bestask, *bestbid;
  SECURITY *sec;
  char firm[5];

  strncpy( firm, order->name, 4 );
  firm[4]=0;
/*  printf( "firm=%s", firm); */

  sec = find_sec( security, Securitydata );
  load_ordbook( &bestbid, &bestask, security );

  printf( "\nIs that ok? (y|n)" );
  scanf( "%s", charstr );

  if( tolower(charstr[0]) == 'y' ) {
      ok = 0;

      /* check for positivity of shares and price */
      if( (order->amount<=0)
      ||  ( (order->price==0) && (order->mkt_or_lim == LIM) ) ) {
            client_warning( "Number of units must be positive, and price non-zero." );
    	    ok = -1;
      }

      /* prevent limit asks placed above best bid, or vice versa.  
       * value_portf() relies on a sensible order book that isn't
       * easily manipulated to give unreasonable midspread numbers.
      */
      if( (order->mkt_or_lim == LIM) && (
          (
             ( order->buy_or_sell == BUY) && 
             ( bestask && ( order->price > bestask->price))
          )
            || 
          (
             ( order->buy_or_sell == SELL) && 
             ( bestbid && ( order->price < bestbid->price))
          ))
      )
      {
          client_warning( "It would be better to place a market order.");
          ok = -1;
      }

      /* disqualify unreasonably large or expensive orders */
      if( (ok != -1) &&
          (order->mkt_or_lim == LIM ) &&
          (( ( ( ismoney(sec)==NO ) && (fabs(order->price) > TOOHIGH) ) ) 
       ||   ( ( ismoney(sec)==YES) && (fabs(order->price) > TOOHIGH/10) )))
      {
          client_warning( "Eep! Too high!" );
          ok = -1;
      }
      if( (ok != -1) &&
          (order->mkt_or_lim == MKT ) && 
             ( strcmp(firm, "firm") && strcmp(order->name, "Fed")) &&
        ( ( ( ismoney(sec)==NO ) && (order->amount > TOOMANY  ) )
       || ( ( ismoney(sec)==YES) && (order->amount > TOOMANY*10) ) ) )
      {
          client_warning( "Eep! Too many! " );
          ok = -1;
      }
      if( (ok != -1) &&
          (order->mkt_or_lim == LIM ) && 
            ( strcmp(firm, "firm") && strcmp(order->name, "Fed")) &&
        ( ( ( ismoney(sec)==NO ) && (order->amount > 3*TOOMANY  ) )
       || ( ( ismoney(sec)==YES) && (order->amount > 3*TOOMANY*10) ) ) )
      {
          client_warning( "Eep! Too many! " );
          ok = -1;
      }

      if( ok == -1 )
          order_free( order );

  }
  else {  /* user entered 'n'; doesn't want to confirm order */
    order_free( order );
    ok = -1;
  }

  free_ordbook( bestbid, bestask );

  return ok;
} /* end of verify_order() */
/*-----------------------------*/

ORDER *create_order(
    char name[NAMELEN+1],   /* player's name (8 chars + 0x0) */
    int mkt_or_lim,    		/* MKT|LIM */
    int buy_or_sell,   		/* BUY|SELL */
    double amount,     		/* unsigned */
    double price ) {
/* returns an ORDER, with zman set to current time */

  ORDER *result;
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

static int at_orderlimit( char *name, char *security ) {
  /* returns YES if name has reached the maximum allowable orders for
   *   security.  returns NO otherwise.
   * Players "Fed" and "poisson" are exempt.
  */
  char actfilename[ACTNAMELEN+1];
  FILE *actfile;
  ACTHEADER actheader;
  SECURITY *sec;

  if( (!strcmp( name, "Fed" )) || (!strcmp( name, "poisson" )) )
      return NO;

  /* read the file header, to see how many orders have already been placed */
  actfile_name( name, security, actfilename );
  actfile = fopen( actfilename, "rb" );
  if( actfile==NULL ) error( "bldordr.c: no .act file!!" );
  fread( &actheader, sizeof(ACTHEADER), 1, actfile );
  fclose( actfile );

  /* find out how many orders are allowed in this security */
  sec = find_sec( security, Securitydata );

  if( sec->maxorders <= actheader.numorders )
      return YES;
  else
      return NO;

} /* end of at_orderlimit() */
/*--------------------*/

static int front_of_firm( ORDER *order, char *security ) {
  /* returns YES if order is a limit order better than one placed by
   * the firm (any player with section # 0 counts as a firm).
   * We need this feature so saboteurs don't hinder the firm's ability
   * to raise money from a securities issue.
  */
  int result,i;
  ORDER *bestask, *bestbid;

  load_ordbook( &bestbid, &bestask, security );

  if( (bestask==NULL) || (order->price >= bestask->price)
  || (order->mkt_or_lim==MKT) || (order->buy_or_sell==BUY) )
      result = NO;
  else {
      /* get pointer to Names element with info on who placed the best ask */
      i=0;
      while( strcmp( bestask->name, Names[i].name ) ) i++;
      if( Names[i].section == 0 )
          result = YES;
      else
          result = NO;
  }

  free_ordbook( bestbid, bestask );
  return result;
} /* end of front_of_firm() */
