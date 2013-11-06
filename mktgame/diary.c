/* functions to update and display the diary history.
 * We use read_three() and backup_three() from disptrad.c, but we have
 *    our own disp_two() and disp_one() functions here.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>  /* for stat(), from which we get file length */
#include <sys/stat.h>   /* for stat(), from which we get file length */
#include "xmem.h"
#include "aqc.h"

/**** GLOBAL VARIABLES ****/
extern SECURITY **Securitydata;

/**** LOCAL FUNCTION DEFINITIONS ****/
static void disp_two( TRADE *currtrade, HOLDINGS *currholding,
    char *security, FILE *outfile );

/**** FUNCTIONS VISIBLE EXTERNALLY ***
void update_diary( TRADE *trade, HOLDINGS *holding, char *security )
void disp_diary( char *security )
****/

void update_diary( TRADE *trade, HOLDINGS *holding, char *security ) {
  /* Same format as an .act file, except it's
   *   for all players, and gets appended to every time anyone trades.
  */
    char outfilename[ACTNAMELEN];
    FILE *outfile;
    ORDER *dummyord;

    /* allocate dummyord.  We always write a zero order because the diary
     *   is only updated when there's a trade.  We bother to write out
     *   an order so the diary file will be of the same format as the
     *   .act files, and so compatible with functions that read and
     *   manipulate them.
     *   With this compatibility, we will be able to readily recreate
     *     anyone's .act file by just pulling out his records in the
     *     diary file.
    */
    dummyord = order_alloc();

    /* open outfile */
    strcpy( outfilename, security );
    strcat( outfilename, "act/" );
    strcat( outfilename, DIARYFILE );
    if( (outfile = fopen( outfilename, "ab" )) == NULL )
        outfile = fopen( outfilename, "wb" );

    fwrite( dummyord, sizeof(ORDER), 1, outfile );
    fwrite( trade, sizeof(TRADE), 1, outfile );
    fwrite( holding, sizeof(HOLDINGS), 1, outfile );

    order_free( dummyord );
    fclose( outfile );
} /* end of update_diary() */
/*--------------------------*/

void disp_diary( char *security ) {
  /* Display the trade and holding history (from diary.out)
   *   in human-readable form.
   * Same as get_act_ts(), except here we aren't after just one person's
   *   trading history.
  */

  char diaryfilename[ACTNAMELEN], cmd;
  int scrnline = 0;
  long int filepos, flen;
  struct stat sbuf;    /* holds stats on a file */
  FILE *diaryfile,
       *outfile;
  ORDER currorder;
  TRADE currtrade;
  HOLDINGS currholding;
  SECURITY *sec;

  /* open the output file */
  outfile = stdout;

  /* identify and open the input file */
  strcpy( diaryfilename, security );
  strcat( diaryfilename, "act/" );
  strcat( diaryfilename, DIARYFILE );
  diaryfile = fopen( diaryfilename, "rb" );
  if( diaryfile==NULL ) {
      client_warning( "There have been no trades yet." );
      return;
  }

  /* display header */
  sec = find_sec( security, Securitydata );
  fprintf( outfile, "                      *** ticker history -- %s %s***\n", security, currency_id(sec) );
  fprintf( outfile, "         NAME             TRADES          HOLDINGS          TIME\n");

  /* position yourself at end of file */
  stat( diaryfilename, &sbuf );
  flen = sbuf.st_size;
  fseek( diaryfile, 0L, SEEK_END );
  filepos = flen;

  /* loop over the file, backwards, skipping each time over the length of
   *    one HOLDINGS, one TRADE, and one ORDER */
  backup_three( diaryfile, &filepos );

  while( filepos >= 0 ) {
      read_three( &currorder, &currtrade, &currholding, &filepos, diaryfile );
      if( ( ismoney(find_sec(security,Securitydata))==NO )
      ||  ( ismoney(find_sec(currtrade.other_sec,Securitydata))==YES ) ) {
          disp_two( &currtrade, &currholding, security, outfile );
          scrnline ++;
      }
      backup_three( diaryfile, &filepos );  backup_three( diaryfile, &filepos );

      if( scrnline > SCRNLINES ) {
	  cmd = get_disp_cmd();
          if( cmd == 'n' ) scrnline = 0;
          else  break;
      }
  }

  fclose( diaryfile );

} /* end of disp_diary() */
/*---------------------------*/

static void disp_two( TRADE *trade, HOLDINGS *holding,
    char *security, FILE *outfile ) {
    /* trade and holding have just been read out diary.out,
     *   by read_three() */
    SECURITY *sec;

    sec = find_sec( security, Securitydata );

    /* display player's name */
    fprintf( outfile, "%13s", trade->name );

    /* display trade */
    fprintf( outfile, "%9s %5d %7.3f",
                " ", (int)trade->amount, (float)(trade->price) );

    /* display holdings */
    if( ismoney(sec) == NO )
        fprintf( outfile, "%5s %5d", " ", (int)holding->amount );
    else
        fprintf( outfile, "%5s %9.3f", " ", (float)holding->amount );

    /* display time */
    fprintf( outfile, "       %s\n", shasctime(localtime(&(holding->zman))) );

} /* end of disp_two() */
/*---------------------------*/
/*---------------------------*/
/*---------------------------*/
/*---------------------------*/


