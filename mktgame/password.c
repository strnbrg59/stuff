/* installs and checks passwords on aqc accounts */

/**** INCLUDES ****/
#include "aqc.h"
#include <unistd.h> /* for sleep() */

/* #define __disable_curses__   */

#ifdef __disable_curses__
  /* disable curses */
#define initscr()  do_nothing()
#define endwin() do_nothing()
#define noecho() do_nothing()
#define echo() do_nothing()
#define printw printf
#define scanw scanf
#endif

#ifndef __TURBOC__ 
# ifndef __DISABLE_CURSES__
#  include <curses.h>
# endif
#else
#  include <conio.h>  /* for text hiding on password input */
#  include <dos.h>
#endif
#include <stdlib.h>
#include <string.h>

/**** GLOBAL VARIABLE ****/
extern int N_players;
extern NAMES *Names;

/**** FUNCTIONS VISIBLE EXTERNALLY ****
NAMES *get_usrnamepwd( void );
int install_pwd( NAMES *elem, NAMES *names );
void prompt_for_pwd( char *pwd );
****/

/**** LOCAL FUNCTION DEFINITIONS ****/

/**** FUNCTION DEFINITIONS ****/

NAMES *get_usrnamepwd( void ) {
  /* prompt for user's name and password: called by main(). */
    char name[120];
    static NAMES result;

    /* initialize curses, or clear PC screen */
#   ifdef __TURBOC__
      clrscr();
#   else
      initscr();
#endif

    /* get user's name */
    printw( "Enter your knickname: " );
    scanw( "%s", name );
    (result.name)[NAMELEN] = 0;
    strncpy( result.name, name, NAMELEN );

    /* get user's password */
    prompt_for_pwd( result.pwd );

    /* leave curses */
#ifndef __TURBOC__
    endwin(); setlinebuf( stdout );
#endif

    return &result;
} /* end of get_usrnamepwd() */
/*--------------------------*/

int install_pwd( NAMES *elem, NAMES *names) {
  /* Prompt for a password, put it in the NAMES structure, and write
   *   it out to the file that holds the names and passwords.
   * elem is the element of names that holds the data for name.
   * names points to the beginning of the NAMES array.
   * Returns 0 if ok, -1 if failed.
  */
  char password[NAMELEN+1], pwdcheck[NAMELEN+1];
  int rtrnval,
      handle;  /* of server */

  /* get the password, and get it confirmed */
  printf( "You must set a new password.  \n" );
  printf( "It must not begin with a number, and should be all small letters.\n");
  printf( "Please wait a few moments...\n" );
#ifdef __TURBOC__
      delay(5000);
#else
      sleep(5);
#endif

  /* get server's attention */
#ifndef __TURBOC__
  handle = server_avail();
  if( handle < 0 )
      error( "Server unavailable." );
#endif

  /* if we get to here, the server is going to get to us, either right
   * away, or very soon */
#ifndef __TURBOC__
  server_ready( handle );  /* this returns when the server is really ready */
#endif

  /* free and reload names */
  free_names( names );
  N_players = load_players( &Names );
  names = Names;

  /* initialize curses, or clear the PC screen */
#ifdef __TURBOC__
  clrscr();
#else
  initscr();
#endif
  prompt_for_pwd( password );
  printw( "Just to confirm...\n" );
  prompt_for_pwd( pwdcheck );

  /* leave curses */
  endwin();
#ifndef __TURBOC__
    setlinebuf( stdout );
#endif

  if( strcmp( password, pwdcheck ) )
      rtrnval = -1;
  else {
      printf("Thank you.  Protect your password; your login name and\n");
      printf("   password are all it takes to place trades on your behalf!\n");
  }

  if( rtrnval != -1 ) {
      strcpy( elem->pwd, password );
      dump_players( names, N_players );
  }

  /* free the server */
#ifndef __TURBOC__
  server_goodbye( handle );
#endif
  handle=0; handle++;
  return rtrnval;
} /* end of install_pwd() */
/*---------------------------*/

void prompt_for_pwd( char *pwd ) {
  char longpwd[120];

  printw( "Enter your password: " );
#ifdef __TURBOC__
  textbackground( WHITE );
  cputs( "xxxxxxxxxxx" );
  gotoxy( wherex()-9, wherey() );
  scanf( "%s", longpwd );
  textbackground( BLACK );
  gotoxy( 1, wherey()+1 );
#else
  noecho();
  scanw( "%s", longpwd );
  echo();
#endif

  strncpy( pwd, longpwd, NAMELEN );
} /* end of prompt_for_pwd() */































































