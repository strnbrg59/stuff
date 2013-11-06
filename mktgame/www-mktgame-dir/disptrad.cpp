/* utilities to display the history of trades and holdings */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>  /* for stat(), from which we get file length */
#include <sys/stat.h>   /* for stat(), from which we get file length */
#include "xmem.hpp"
#include "aqc.hpp"

static char *Buy_or_sell[2] = { "buy ", "sell" };

/**** LOCAL FUNCTION DECLARATIONS ****/
static void disp_money( Trade *currtrade, Holdings *currholding,
    FILE *outfile, char *currency );
static void disp_three( Order *currorder, Trade *currtrade,
    Holdings *currholding, FILE *outfile, char *security );

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void get_act_ts( char *name, char *security );
void get_money_ts( char *name, char *currency );
void read_three( Order *currorder, Trade *currtrade,
    Holdings *currholding, long int *filepos, FILE *infile );
void backup_three( FILE *actfile, long int *filepos );

****/

/**** FUNCTION DEFINITIONS ****/

void get_act_ts( char *name, char *security ) {
  /* Display the order, trade and holdings history (from accounts/<name>.act)
   *   in human-readable form.
   * Ts stands for time series.
  */

	cout << "<pre>\n";

  char actfilename[ACTNAMELEN], *currencyid;
  int scrnline = 0;
  long int filepos, flen;
  struct stat sbuf;    /* holds stats on a file */
  FILE *actfile,
       *outfile;   /* a temporary file to which we print, so we can view
                    * it using the more utility. */
  Order currorder;
  Trade currtrade;
  Holdings currholding;
  Security *sec;

  sec = find_sec( security, G::securitydata );
  currencyid = currency_id( sec );

  /* open the output file */
  outfile = stdout;

  /* identify and open the input file */
  actfile_name( name, security, actfilename );
  actfile = fopen( actfilename, "rb" );
  if( actfile==NULL )
      error( "There is no history on you yet." );

  /* display header */
  fprintf( outfile, "                       *** %s -- %s activity %s***<p>\n",
      (name), security, currencyid );
  fprintf( outfile, "    Orders                Trades          Holdings          Time<p>\n");

  /* position yourself at end of file */
  stat( actfilename, &sbuf );
  flen = sbuf.st_size;
  fseek( actfile, 0L, SEEK_END );
  filepos = flen;

  /* loop over the file, backwards, skipping each time over the length of
   *    one Holdings, one Trade, and one Order */
  backup_three( actfile, &filepos );

  while( (filepos >= (int)sizeof(Actheader)) && (filepos>0) ) {
      /* need the filepos>0 condition because filepos is long and sizeof
       * isn't, so a negative filepos can get wrapped around when compared
       * to an int. 
      */ 
      read_three( &currorder, &currtrade, &currholding, &filepos, actfile );
      disp_three( &currorder,  &currtrade, &currholding, outfile, security );
      scrnline ++;
      backup_three( actfile, &filepos );  backup_three( actfile, &filepos );
	  /*
      if( scrnline > SCRNLINES ) {
	  cmd = get_disp_cmd();
          if( cmd == 'n' )
	      scrnline = 0;
          else
	      break;
      }
	  */
  }

  fclose( actfile );
	cout << "</pre>\n";

} // get_act_ts()
//---------------------------

void read_three( Order *currorder, Trade *currtrade,
    Holdings *currholding, long int *filepos, FILE *infile ) {
    /* from the current position in infile (open for "rb"),
     *  read in currorder, currtrade and currholding.  */

    fread( currorder, sizeof(Order), 1, infile );
    fread( currtrade, sizeof(Trade), 1, infile );
    fread( currholding, sizeof(Holdings), 1, infile );

    *filepos += sizeof(Order) + sizeof(Trade) + sizeof(Holdings);

} /* end of read_three() */
/*---------------------------*/

static void disp_three( Order *order, Trade *trade,
    Holdings *holding, FILE *outfile, char *security ) {
    /* these three have just been read out of someone's .act file,
     *   by read_three() */
    int noorder=0, notrade=0; /* boolean */
    /* if there's no order, the order variable is filled with zeros;
     * ditto if there's no trade.  The .act file records orders when
     * they come in, and then records trades (and updated holdings)
     * as they happen. */
    Security *sec;

    sec = find_sec( security, G::securitydata );

    /* set noorder and notrade to 1, if appropriate */
    if( (order->name)[0] == 0 )
        noorder = 1;
    if( (trade->name)[0] == 0 )
        notrade = 1;

    /* display order */
    if( noorder == 1 )
        fprintf( outfile, "%13s", " " );
    else {
		fprintf( outfile, "%c", order->comment );
        fprintf( outfile, "%4s %5d",
            Buy_or_sell[order->buy_or_sell], (int)order->amount );

        if( order->mkt_or_lim == MKT )
            fprintf( outfile, "%7s", " " );
        else
            fprintf( outfile, "%7.2f", (float)(order->price) );
    }

    /* display trade */
    if( notrade == 1 )
        fprintf( outfile, "%18s", " " );
    else
        if( (!strcmp( trade->other_sec, "endwmt" ) )
        ||  (ismoney(find_sec(trade->other_sec,G::securitydata)) == YES ) )
            fprintf( outfile, "%9s %5d %7.3f",
                " ", (int)trade->amount, (float)(trade->price) );
        else
            fprintf( outfile, "%4s *** see %ss ***","", trade->other_sec );

    /* display holdings */
    if( ismoney(sec)==NO )
        fprintf( outfile, "%5s %5d", " ", (int)holding->amount );
    else
        fprintf( outfile, "%5s %9.3f", " ", (float)holding->amount );

    /* display time: if notrade, we take time from order; if noorder
     *   we take time from holdings */
    if( notrade == 1 )
        fprintf( outfile, "       %s\n", 
			shasctime(localtime(&(order->zman))) );
    if( noorder == 1 )
        fprintf( outfile, "       %s\n", 
			shasctime(localtime(&(holding->zman))) );

} /* end of disp_three() */
/*---------------------------*/

void backup_three( FILE *actfile, long int *filepos ) {
  /* back up in actfile by the length of one Holdings, one Trade,
   * and one Order.  Decrement *filepos by the amount of this movement
   * in the file.
  */
  long int movement;

  movement = sizeof(Holdings) + sizeof(Trade) + sizeof(Order);

  fseek( actfile, (-1)*movement, SEEK_CUR );
  (*filepos) -= movement;

} /* end of backup_three() */
/*---------------------------*/

void get_money_ts( char *name, char *currency ) {
  /* Display trades that resulted in increments and decrements from the
   *   stock of money (named *currency).  Option 'h' in the currency menu.
  */

//	cout << HTML 
	cout << "<pre>\n";

  char actfilename[ACTNAMELEN], *currencyid, tempstr[40];
  int scrnline = 0;
  long int filepos, flen;
  struct stat sbuf;    /* holds stats on a file */
  FILE *actfile;

  Order currorder;
  Trade currtrade;
  Holdings currholding;
  Security *sec;

  // Identify and open the input file.
  actfile_name( name, currency, actfilename );
  actfile = fopen( actfilename, "rb" );
  if( actfile==NULL ) {
      client_warning( "There is no history on you yet." );
      return;
  }

  sec = find_sec( currency, G::securitydata );
  if( n_currencies() == 1 )
      sprintf( currencyid=tempstr, " " );
  else
      sprintf( currencyid=tempstr, "(prices in %ss) ", sec->name);

  // Display header 
  printf( "                       *** %s -- %s holdings %s***\n",
      (name), currency, currencyid );
  printf( "    Security              TradeS          Holdings          TIME\n");


  // Position yourself at end of file 
  stat( actfilename, &sbuf );
  flen = sbuf.st_size;
  fseek( actfile, 0L, SEEK_END );
  filepos = flen;

  // Loop over the file, backwards, skipping each time over the length of
  // 	one Holdings, one Trade, and one Order.
  backup_three( actfile, &filepos );

  while( (filepos >= (int)sizeof(Actheader) ) && (filepos>0)){
      read_three( &currorder, &currtrade, &currholding, &filepos, actfile );
      if( currtrade.name[0] != 0 )
          disp_money( &currtrade, &currholding, stdout, currency );
          scrnline ++;
      backup_three( actfile, &filepos );  backup_three( actfile, &filepos );

  }

  fclose( actfile );

	cout << "</pre>\n";
} // get_money_ts()
//---------------------------

static void disp_money( Trade *currtrade, Holdings *currholding,
  FILE *outfile, char *currency) {
  /* Display trades in proper securities as well as other currencies,
   *   that affected the stock of *currency.
   * Trades in *currency's own market have to be reported as having
   *   been trades in the other currency, in order to maintain consistency
   *   with the format (and the price will have to be in terms of units
   *   of *currency).
  */
  Security *sec;

  sec = find_sec( currency, G::securitydata );
  if( ( currtrade->as_money == NO ) && ( ismoney(sec)==YES )
  && ( ismoney(find_sec(currtrade->other_sec, G::securitydata))==YES) )
  {
      currtrade->amount *= -1*(currtrade->price);
      currtrade->price = 1.0/currtrade->price;
  }

  fprintf( outfile, "%12s        %7.2f%9.3f      %9.3f       %s\n",
          currtrade->other_sec,
          (float)currtrade->amount,
          (float)(currtrade->price),
          (float)(currholding->amount),
          shasctime(localtime(&(currtrade->zman ))));

} /* end of disp_money() */
/*---------------------------*/
/*---------------------------*/
/*---------------------------*/
/*---------------------------*/
/*---------------------------*/


