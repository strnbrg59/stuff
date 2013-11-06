/* Implement delivery on forward contracts.
 *
 * Parameters file (fwddeliv.par):
 *   cash:        <name of underlying security>
 *   directory:   <name of subdirectory (relative to the one this file is in)
 *                 in which is found the main program (fwd.exe) that runs
 *                 the forward market.  .act files are in its subdirectory
 *                 ACCOUNTS.  We could have let the program figure out the
 *                 dir name on its own (like in aqc, letting it be the name
 *                 of the security+"act", but that would force us to use
 *                 strange, too-short, names for the forward securities. >
 *   date:        <delivery date--for recording in .act files.  Try to keep
 *                 it close to the current time, as the current time is what
 *                 will show up in any forced sales, and a big discrepancy
 *                 here will confuse people. >
 *
 * Basic principles
 *   1. In the course of trading fwd contracts, a player has accumulated
 *      an old-style PORTFOLIO record indicating a number of units (of the
 *      cash) and "money" holdings (which conveniently equal the total
 *      the player has contracted to pay for all his units).
 *   2. Using a dummy limit order book (as in option exercise), have the
 *      player trade on the spot market with Fed, at a price equal to
 *      units/money (money, again, coming from the old-style PORTFOLIO).
 *   3. Call forced_sales(), as these transactions could drive some people's
 *      accounts into the red.
 *   4. We don't have to worry about who delivers to whom; unlike option
 *      exercise, deliveries all take place at the same time, and after
 *      acting as a clearinghouse, Fed's own net position (if any) in
 *      the fwd will be zero.
*/

/**** INCLUDES ****/
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmem.h"
#include "aqc.h"
#include "fwddeliv.h"

/**** DEFINITIONS ****/
#define PARFILENAME "fwddeliv.par"

/**** GLOBAL VARIABLES (for compatibility with the rest of
 **** the aqc programs)--we initialize them below ****/
int N_players;
NAMES *Names;
int N_sec;
SECURITY **Securitydata;

/**** LOCAL FUNCTION DEFINITIONS ****/
static void load_data( char *cash, char *dirpath, int *mon, int *dd, int *yy,
  int *hh, int *min, int *ss );
static OLDPORTFOLIO *load_oldportf( char *name, char *dirpath );

/**** FUNCTION DEFINITIONS ****/
main() {
  char c_ordbookname[ACTNAMELEN+1],  /* order book for cash */
       cash[NAMELEN+1], dirpath[ACTNAMELEN];
  int i, errcode, handle, mon, dd, yy, hh, min, ss;
  FILE *infile;  /* just for checking if a file exists */
  ORDER *mkt_order, *lim_order;
  OLDPORTFOLIO *fwdportf;  /* PORTFOLIO was the precursor of the HOLDINGS structure */

  /* load all the players and their latest holdings--needed by functions
   *   in other files in the project */
  N_players = load_players( &Names );
  Securitydata = load_securities();  /* also sets N_sec */

  /* see if server has been shut off */
# ifndef __TURBOC__
      if( 0 <= (handle = server_avail()) ) error( "Shut off the server, first." );
# endif
  handle=0;handle++; /* so compiler won't complain we're not doing anything with it */

  /* read parameters from fwddeliv.par */
  load_data( cash, dirpath, &mon, &dd, &yy, &hh, &min, &ss );

  /* load all the players and their latest holdings */
  N_players = load_players( &Names );
  Securitydata = load_securities();  /* also sets N_sec */

  /* loop over all players, skipping Fed since he's the clearinghouse */
  for( i=0; i<N_players; i++ ) if( strcmp( Names[i].name, "Fed" ) ) {

      /* load player's position in the forward market */
      fwdportf = load_oldportf( Names[i].name, dirpath );
      if( fwdportf->shares == 0 )
          continue;

      /* move order book file for *cash to a backup file*/
      infile=fopen("delete.me","r");
      if( infile ) remove( "delete.me" );
      ordbook_name( cash, c_ordbookname );
      errcode = rename( c_ordbookname, "delete.me" );

      /* place limit order for player, on the temporary order books. */
      if( fwdportf->shares < 0 )
          lim_order = create_order( fwdportf->name, LIM, SELL,
              abs(fwdportf->shares), fwdportf->money/fwdportf->shares );
      else
          lim_order = create_order( fwdportf->name, LIM, BUY,
              abs(fwdportf->shares), fwdportf->money/fwdportf->shares );
      lim_order->comment = 'D';  /* D for delivery */
      handle_order( lim_order, cash, NO );

      /* place market order for Fed, using handle_order(). */
      if( fwdportf->shares < 0 )
          mkt_order = create_order( "Fed", MKT, BUY, abs(fwdportf->shares), 0 );
      else
          mkt_order = create_order( "Fed", MKT,SELL, abs(fwdportf->shares), 0 );
      mkt_order->comment = 'D';
      handle_order( mkt_order, cash, NO );

      /* restore true limit order books */
      remove( c_ordbookname );  /* it exists; there was the dummy order */
      errcode = rename( "delete.me", c_ordbookname );
      errcode ++;  /* just so compiler doesn't say we don't do anything with it */

  } /* looping over players */

  /*  Call forced_sale(); handle_order() won't call it because we're
   *    calling handle_order with call_server set to NO.
  */
  forced_sale();

return 0;
} /* end of main() */
/*-----------------------------*/

static OLDPORTFOLIO *load_oldportf( char *name, char *dirpath ) {
  /* Loads an old-style HOLDINGS, from the last line of name's .act file
   *   in the forward market whose .act files are stored in dirpath.
  */
  char actfilename[ACTNAMELEN];
  FILE *actfile;
  OLDPORTFOLIO *result;

  /* allocate return value */
  result = (OLDPORTFOLIO *)x_calloc( 1, sizeof(OLDPORTFOLIO) );

  /* open .act file */
  strcpy( actfilename, dirpath );
  strcat( actfilename, "/accounts/" );
  strcat( actfilename, name );
  strcat( actfilename, ".act" );
  actfile = fopen( actfilename, "rb" );

  if( actfile == NULL ) {
      printf( "No activity by %s: assigning zero position.\n", name );
      result->shares = result->money = 0;
  }
  else {
      /* read from actfile.  Portfolio data is one PORTFOLIO-sized
       * chunk of data from the end. */
      fseek( actfile, (-1)*(long)(sizeof(OLDPORTFOLIO)), SEEK_END );
      fread( result, sizeof(OLDPORTFOLIO), 1, actfile );
  }

  fclose( actfile );

  return result;
} /* end of load_oldportf() */
/*------------------------------*/

static void load_data( char *cash, char *dirpath, int *mon, int *dd, int *yy,
  int *hh, int *min, int *ss ) {
  /* read data from fwddeliv.par */

  char garbage[80];
  FILE *parfile;

  parfile = fopen( PARFILENAME, "r" );
  if( parfile==NULL ) { printf("\nCant open parfile.\n"); exit(0); }

  fscanf( parfile, "%s %s", garbage, cash );
  fscanf( parfile, "%s %s", garbage, dirpath );
  fscanf( parfile, "%s %d %d %d", garbage, mon, dd, yy );
  fscanf( parfile, "%s %d %d %d", garbage, hh, min, ss );

} /* end of load_data() */
