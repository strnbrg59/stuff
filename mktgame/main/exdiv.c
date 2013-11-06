/* Credit (money) dividends.  Input file, exdiv.par, looks s.th. like this:
 * Reduce order book by amount of dividend.
 *
 * security: stock
 * dividend: 0.55
 * date:   05 30 93
 * time:   19 00 00
 *
 * This dividend is applied to everyone listed in players.dat.
 * So if you want to apply the dividend to a subset of the players, just
 *   substitute a shorter list for players.dat (but be sure to restore
 *   players.dat before rerunning aqc.exe!
 * If the security in exdiv.par is a currency, that means we just add the
 *   "dividend" to everyone's money balance in that currency.  Otherwise
 *   the dividend is paid in the security's currency.
*/

/**** INCLUDES ****/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../aqc.h"

/**** DEFINITIONS ****/
#define PARFILENAME "exdiv.par"

/**** GLOBAL VARIABLES (for compatibility with the rest of the aqc programs)**/
int N_players;
NAMES *Names;
int N_sec;
SECURITY **Securitydata;

/**** FUNCTION DECLARATIONS ****/
void reduce_orderbook( char *security, double dividend );

/**** FUNCTION DEFINITIONS ****/
main() {
  char sec_name[NAMELEN+1], garbage[6*NAMELEN],
       charnum[NAMELEN*2];
  int mon, dd, yy, hh, min, ss, i, owned, handle;
  double dividend;
  time_t zman;
  FILE *parfile;
  TRADE *dummytrade;
  HOLDINGS *holdings, *moneyholding;
  SECURITY *sec;

  /* see if server has been shut off */
#ifndef __TURBOC__
  if( 0 <= (handle = server_avail()) ) error( "Shut off the server, first." );
#endif

  /* read data from exdiv.par */
  parfile = fopen( PARFILENAME, "r" );
  if( parfile==NULL ) { printf("\nCant open parfile.\n"); exit(0); }
  fscanf( parfile, "%s %s", garbage, sec_name );
  fscanf( parfile, "%s %s", garbage, charnum );
  dividend = atof( charnum );
  fscanf( parfile, "%s %d %d %d", garbage, &mon, &dd, &yy );
  fscanf( parfile, "%s %d %d %d", garbage, &hh, &min, &ss );

  /* compute seconds-since-1/1/70 equivalent to date and time */
  zman = unix_seconds( yy, mon, dd, hh, min, ss );

  /* load all the players and their latest holdings */
  N_players = load_players( &Names );
  Securitydata = load_securities();  /* also sets N_sec */
  sec = find_sec( sec_name, Securitydata );
  holdings = load_holdings( Names, sec_name );

  /* allocate dummy TRADE structure we'll write to players' money files */
  dummytrade = trade_alloc();
  strcpy( dummytrade->other_sec, sec_name );
  if( strlen(sec_name) > NAMELEN-3 ) error("exdiv.c: no room to append \"div\" to security name" );
  strcat( dummytrade->other_sec, "div" );
  dummytrade->amount = 0;
  dummytrade->price  = 0;
  dummytrade->zman   = zman;

  /* go through the players, crediting them dividends in their money files */
  for( i=0; i<N_players; i++ ) {

      strcpy( dummytrade->name, Names[i].name );

      if( ismoney(sec)==YES ) {
          moneyholding = load_holding( Names[i].name, sec->name );
          printf( "%s\n", Names[i].name );
    	  moneyholding->amount += dividend;
          moneyholding->zman = zman;

      /* dump out the money record */
      dump_trade( dummytrade, moneyholding, sec->name );
      }
      else {
          /* load player's holding of currency the dividend will get paid in */
          moneyholding = load_holding( Names[i].name, sec->currency );

          /* pull up player's holdings of security */
          owned = (int)holdings[i].amount;
          printf( "%s\n", holdings[i].name );

          moneyholding->amount += owned*dividend;
          moneyholding->zman = zman;

          /* dump out the money record */
          dump_trade( dummytrade, moneyholding, sec->currency );
      }

      /* add to the short-money list, if necessary */
      check_neg_money( (Names[i]).name, sec->currency );
  } /* looping over players */


  /* reduce limit order book by amount of dividend */
  reduce_orderbook( sec_name, dividend );

  /* invoke forced sales, where necessary (no confirmation on screen) */
  forced_sale();

return 0;
} /* end of main() */
/*-----------------------------*/

void reduce_orderbook( char *security, double dividend ) { 
  /* reduce prices by dividend */
  ORDER *bestask, *bestbid, *currorder;
  
  load_ordbook( &bestbid, &bestask, security );

  currorder = bestask;
  while( currorder != NULL ) {
      currorder->price -= dividend;
      currorder = currorder->next;
  }

  currorder = bestbid;
  while( currorder != NULL ) {
      currorder->price -= dividend;
      currorder = currorder->next;
  }

  dump_ordbook( bestbid, bestask, security );

} /* end of reduce_orderbook() */





