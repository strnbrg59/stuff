/* Set initial endowments, and record them in the .act files.
 * If a player already has an .act file for the affected security,
 *   this program does not destroy it, and so could be used to correct
 *   an error.
 * If a player doesn't have an .act file, we place an ACTHEADER record
 *   at the head of the file, before dumping out a HOLDINGS record with the
 *   initial endowment.
 * Reads from initact.par, which has the following format:
 *
 * security: stock    ** can be money, too--just make sure there's a directory for this security
 * endowment: 100     ** floating-point number, if security is money
 * date:   05 30 93
 * time:   19 00 00
 *
 * This is applied to everyone listed in players.dat.
 * So if you want it to to a subset of the players, just
 *   substitute a shorter list for players.dat (but be sure to restore
 *   players.dat before rerunning aqc.exe!
*/

/**** INCLUDES ****/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../aqc.h"

/**** DEFINITIONS ****/
#define PARFILENAME "initact.par"

/**** GLOBAL VARIABLES (for compatibility with the rest of the aqc programs)**/
int N_players;
NAMES *Names;
int N_sec;
SECURITY **Securitydata;

/**** LOCAL FUNCTION DECLARATIONS ****/
static void dump_actheader( char *name, char *security );

/**** FUNCTION DEFINITIONS ****/
main()  {
  char security[NAMELEN+1], garbage[6*NAMELEN],
       charnum[NAMELEN*2];
  int mon, dd, yy, hh, min, ss, i, handle;
  double endowment;
  time_t zman;
  FILE *parfile;
  TRADE *dummytrade;
  HOLDINGS *initholding;

  /* see if server has been shut off */

#ifndef __TURBOC__
  if( 0 <= (handle = server_avail()) ) error( "Shut off the server, first." );
#endif

  /* read data from initact.par */
  parfile = fopen( PARFILENAME, "r" );
  if( parfile==NULL ) { printf("\nCant open parfile.\n"); exit(0); }
  fscanf( parfile, "%s %s", garbage, security );
  fscanf( parfile, "%s %s", garbage, charnum );
  endowment = atof( charnum );
  fscanf( parfile, "%s %d %d %d", garbage, &mon, &dd, &yy );
  fscanf( parfile, "%s %d %d %d", garbage, &hh, &min, &ss );

  /* compute seconds-since-1/1/70 equivalent to date and time */
  zman = unix_seconds( yy, mon, dd, hh, min, ss );

  /* load players (mentioned in players.dat) */
  N_players = load_players( &Names );

  /* allocate TRADE  and HOLDINGS structures we'll write to players' .act files */
  dummytrade = trade_alloc();
  initholding = holding_alloc();
  dummytrade->zman = initholding->zman = zman;
  strcpy( dummytrade->other_sec, "endwmt" );
  initholding->amount = endowment;

  /* go through the players, filling in their names in the
   * TRADE/HOLDINGS structures, and dumping them out to .act files.
  */
  for( i=0; i<N_players; i++ ) {
      strcpy( dummytrade->name, Names[i].name );
      strcpy( initholding->name, Names[i].name );
      dump_actheader( Names[i].name, security );
      dump_trade( dummytrade, initholding, security );
  }   /* for i<N_players */

return 0;
}  /* end of main() */
/*-----------------------------*/
static void dump_actheader( char *name, char *security ) {
  /* Dump an ACTHEADER structure to initialize an .act file.  ACTHEADER
   *   contains information on how many actions a person has taken.
   * Do this *only* if the .act file doesn't exist already.
  */
  char actfilename[ACTNAMELEN+1];
  FILE *actfile;
  ACTHEADER actheader;

  actheader.numorders = 0;

  /* open the .actfile for appending in binary mode */
  actfile_name( name, security, actfilename );
  actfile = fopen( actfilename, "r" );
  /* not "w" or we'd destroy it; we only want to know if it exists */

  if( actfile==NULL ) {
      actfile = fopen( actfilename, "wb" );
      fwrite( &actheader, sizeof(ACTHEADER), 1, actfile );
  }

  fclose( actfile );
} /* end of dump_actheader() */
/*--------------------------------*/
/*--------------------------------*/
/*--------------------------------*/
/*--------------------------------*/
