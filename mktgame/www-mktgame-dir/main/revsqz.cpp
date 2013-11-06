/* Eliminate all traces of activity after a given date and time:
 * Fix the diary,
 * Fix the .act files,
 * Fix the cash files .
 *
 * fix_diary() creates diary.new in the working directory (not in the
 *   security's  -act directory).
 * fix_actfiles() does not.
 *
 * unix_seconds() has to be in Pacific Standard Time.
 * 
*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../aqc.hpp"

/**** GLOBAL VARIABLES ****/
SECURITY **Securitydata;
NAMES *Names;
int N_players;
int N_sec;

/**** FUNCTION DECLARATIONS ****/
void fix_diary( void );
void fix_actfiles( void );
time_t unix_seconds(int yy,int mm,int dd, int hh,int dakot,int ss);
  /* in PST*/

/**** FUNCTION DEFINITIONS ****/
main() {

/*  fix_diary();   */
  fix_actfiles();  

} /* end of main() */
/*---------------------------*/

void fix_diary( void ) {
  char diaryfilename[40], asc[120];
  time_t cutoff;
  FILE *infile, *outfile;
  HOLDINGS holding;
  ORDER order;
  TRADE trade;

  printf("enter security name: ");
  scanf( "%s", diaryfilename );
  strcat( diaryfilename, "act/" );
  strcat( diaryfilename, DIARYFILE );

  infile = fopen( diaryfilename, "rb" );
  outfile = fopen( "diary.new", "wb" );

  fread( &order, sizeof(ORDER), 1, infile );
  fread( &trade, sizeof(TRADE), 1, infile );
  fread( &holding, sizeof(HOLDINGS), 1, infile );

  cutoff = unix_seconds( 94,5,10, 6,0,00 ); /*PST*/

  while( (!feof( infile ))
  &&     (holding.zman < cutoff)) {

      fwrite( &order, sizeof(ORDER), 1, outfile );
      fwrite( &trade, sizeof(TRADE), 1, outfile );
      fwrite( &holding, sizeof(HOLDINGS), 1, outfile );

      fread( &order, sizeof(ORDER), 1, infile );
      fread( &trade, sizeof(TRADE), 1, infile );
      fread( &holding, sizeof(HOLDINGS), 1, infile );

  }
} /* end of fix_diary() */
/*---------------------------*/

void fix_actfiles( void ) {
  char actfilename[ACTNAMELEN+1], newactfile[ACTNAMELEN+1],
       security[NAMELEN+1];
  int i;
  FILE *infile, *outfile;
  HOLDINGS holding, *holdings;
  ORDER order;
  TRADE trade;
  ACTHEADER actheader; 

  /* prompt for security */
  printf( "Enter security name > " );
  scanf( "%s", security );

  /* load up the HOLDINGSs--necessary for addto_holding() */
  N_players = load_players( &Names );

  /* loop over all players' .act files, copying from them only everything up
   * to the time before the short squeeze was carried out.
  */
  for( i=0; i<N_players; i++ ) {

      /* open files */
      actfile_name( Names[i].name, security, actfilename );
      infile = fopen( actfilename, "rb" );
      outfile = fopen( "new.act", "wb" );

      printf( "About to work on %s.\n", actfilename );

      fread( &actheader, sizeof(ACTHEADER), 1, infile );
      fwrite( &actheader, sizeof(ACTHEADER), 1, outfile );

      fread( &order, sizeof(ORDER), 1, infile );
      fread( &trade, sizeof(TRADE), 1, infile );
      fread( &holding, sizeof(HOLDINGS), 1, infile );

      while( (!feof(infile)) 
      &&     (holding.zman <  unix_seconds( 94,5,10, 6,00,00 )) ) { /*PST*/
          fwrite( &order, sizeof(ORDER), 1, outfile );
          fwrite( &trade, sizeof(TRADE), 1, outfile );
          fwrite( &holding, sizeof(HOLDINGS), 1, outfile );

          fread( &order, sizeof(ORDER), 1, infile );
          fread( &trade, sizeof(TRADE), 1, infile );
          fread( &holding, sizeof(HOLDINGS), 1, infile );
      }

      fclose( infile );
      fclose( outfile );

      remove( actfilename );
      rename( "new.act", actfilename );
  }

} /* end of fix_actfiles() */

/*---------------------------*/
/*---------------------------*/
/*---------------------------*/


