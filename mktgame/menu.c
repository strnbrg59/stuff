/* This is the only place where the names of the securities are mentioned
 *   in the code.
 * To change the names of the securities...
 *   1. Do a search-and-replace in this file, searching for the security
 *      name.  It should appear in two places.
 *   2. In menu2(), change the choices shown on the screen so they'll be
 *      mnemonic.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqc.h"

/**** GLOBAL VARIABLE ****/
extern int N_players;
extern NAMES *Names;
extern int N_sec;
extern SECURITY **Securitydata;

/**** LOCAL FUNCTION DEFINITIONS ****/
static void menu2( char *name, int sec_num );
static int money_only( SECURITY *sec );
static void disp_choices1( void );
static void disp_choices2( int sec_num );

/**** FUNCTION VISIBLE EXTERNALLY ****
void menu1( char *name );
****/

/**** STATIC FUNCTION DECLARATIONS ****/
static char *menuchoices( void );

/**** FUNCTION DEFINITIONS ****/
void menu1( char *name ) {
  /* top-level menu--selects the security. */
  
  char cmdstr[120];
  int sec_num;

  cmdstr[0] = ' ';
  init_cncttime_Kname( name );

  disp_choices1();
  while( cmdstr[0]!='q' ) {

      printf( "Main menu: select %s, or q to quit. > ", menuchoices() );
      scanf( "%s", cmdstr );
      fflush( stdin );

      update_connect_time(NO);

      if( -1 != (sec_num = is_sec_choice( cmdstr[0] )) )
          /* sec_num points to an element of Securitydata, or -1 if
           * cmdstr selects something other than a security */
          if( money_only( Securitydata[sec_num] ) == YES )
              get_money_ts( name, Securitydata[sec_num]->name );
          else
              menu2( name, sec_num );
      else
      switch( cmdstr[0] )  {
          case 'f' : finger( name ); break;
          case '#' : holding_list( name ); break;
          case 'q' : break;
          default  : {
              printf( "\nNot a valid choice.\n\n" );
              disp_choices1();
          }
      }

  } /* while !='q' */

} /* end of menu1() */
/*-------------------------------*/

static void menu2( char *name, int sec_num ) {
  /* security-level menu.  menu1() selects the security. */
    char cmdstr[120], security[NAMELEN+1], choicestr[10];
    ORDER *latestord;
    SECURITY *sec;

    strcpy( security, Securitydata[sec_num]->name );
    sec = Securitydata[sec_num];

    if( ismoney(sec) == YES )
        strcpy( choicestr, "bocahtp" );
    else
        strcpy( choicestr, "bocatp" );

    cmdstr[0] = 'b';
    disp_choices2( sec_num );

    while( cmdstr[0]!='x' ) {

        printf( "%s menu: select %s, or x to exit. > ",
            Securitydata[sec_num]->name, choicestr );
        scanf( "%s", cmdstr );
        fflush( stdin );
        update_connect_time(NO);

        switch( cmdstr[0] )  {
            case 'b' : disp_orders( security ); break;
           /* case 's' : disp_order_summary( security ); break; */
            case 'o' : {
                   latestord = build_order( name, security );
                   if( latestord != NULL )
                       handle_order( latestord, security, YES );
                   break;
                   }
            case 'c' : cancel_order( name, security ); break;
            case 'a' : get_act_ts( name, security ); break;
            case 'h' : if( ismoney(sec)==YES ) get_money_ts( name, security ); break;
    	    case 't' : disp_diary( security ); break;
            case 'e' : {
                if( strcmp(sec->underlying,"NULL" ) )
                        exercise_option(
                            name, sec->name, sec->underlying,
                            sec->strikeprice, sec->call_or_put[0]);
                break;
                       }
            case 'p' : prospectus( security ); break;
            case 'x' : break;
            default  : {
                          printf( "That's not one of your choices.\n\n" );
                          disp_choices2( sec_num );
                       }
        }
    }

} /* end of menu2() */
/*----------------------*/

static void disp_choices1( void ) {
  int i;

  printf( "     *** main menu ***\n" );

  for( i=0; i<N_sec; i++ )
      printf( "   %s.  %s\n", Securitydata[i]->menu_choice,
                              Securitydata[i]->menu_fullname );

  printf( "   f.  finger\n" );
  printf( "   q.  quit\n" );

} /* end of disp_choices1() */
/*----------------------*/

static void disp_choices2( int sec_num ) {
    printf( "      *** %s market ***\n", Securitydata[sec_num]->name );
    printf( "   b.   check limit order Book\n" );
/*  printf( "   s.   Summarize limit order book\n" ); */
    printf( "   o.   place an Order\n" );
    printf( "   c.   Cancel an order\n" );
    printf( "   a.   Account activity\n" );
    if( ismoney( Securitydata[sec_num] ) == YES )
        printf( "   h.   cash Holdings in this currency\n" );
    printf( "   t.   Ticker\n" );
    if( strcmp( Securitydata[sec_num]->underlying, "NULL" ))
        printf( "   e.   Exercise options\n" );
    printf( "   p.   Prospectus\n" );
    printf( "   x.   eXit\n" );

} /* end of disp_choices2() */
/*----------------------*/

static char *menuchoices( void ) {
  /* put together that string of one-letter choices for the main menu,
   * from Securitydata[].menu_choice.
  */
  static char result[80];  /* room for 80 securities!! */
  int i;

  for( i=0; i<N_sec; i++ )
      result[i] = Securitydata[i]->menu_choice[0];

  result[i] = 'f'; /* finger */
  result[i+1] = 0;  /* string-ending null */

  return result;
} /* end of menuchoices() */
/*----------------------------*/

static int money_only( SECURITY *sec ) {
  /* If sec is the last currency-type security in Securitydata, then
   *   we won't have a market for it; it will serve as money only,
   *   and when selected from the main menu it will just give an
   *   account of holdings in this currency.
   * Returns YES if sec is in fact the last currency-type security.
  */

  int result=NO, i;

  if( ismoney(sec) == NO )
      result = NO;
  else
      for( i=0; i<N_sec; i++ ) {
          if( (result==YES) && (ismoney(Securitydata[i])==YES) )
              result = NO;
          if( sec == Securitydata[i] )
              result = YES;
      }

  return result;
} /* end of money_only() */
