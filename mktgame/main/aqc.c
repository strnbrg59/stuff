/* Fully-automated, continuous trading system. */

/**** INCLUDES ****/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../aqc.h"
#ifndef __TURBOC__
#include <unistd.h> /* for alarm() */
#endif

/**** GLOBAL VARIABLES ****/
NAMES *Names;  /* Only one function ever changes this--load_players(). */
int N_players;    /* number of players */
int N_sec;        /* number of securities */
SECURITY **Securitydata; /* loaded in loadsec.c */

/**** LOCAL FUNCTION DECLARATIONS ****/
void sig_handler( int signame );

/**** FUNCTION DEFINITIONS ****/
main() {
    int handle;
    NAMES *user;

    /* disable keyboard-producible signals */
#ifndef __TURBOC__
      signal( SIGINT, sig_handler );
      signal( SIGQUIT, sig_handler );
      signal( SIGHUP, sig_handler );
      signal( SIGALRM, sig_handler );
#endif

    /* load data about the securities */
    Securitydata = load_securities();

    /* load players' names */
    N_players = load_players( &Names );

    /* prompt for user name and password */
    user = get_usrnamepwd();
    if( !nameok( user, Names, N_players ) )
            error( "Bad user name or password." );
    
    announcement();

    /* warn if server is down */
    if( (handle=server_avail()) < 0 ) 
        client_warning( "**The server is unavailable; you can look, but you can\'t place orders.***" );
    else {
        server_ready( handle ); /* if omit this, I think the server dies (althought maybe not right away) */
        server_goodbye( handle );
    }

    /* play ball! */
    menu1( user->name );

    update_connect_time( YES );  /* YES=dump_now     */

return 0;
} /* end of main() */
/*--------------------------*/

#ifndef __TURBOC__  /* gets rid of the entire sig_handler() function */
void sig_handler( int signame ) {
    /* handle signals.  We ignore SIGINT and SIGQUIT.  For SIGHUP, we
     *    want to terminate the process (else it consumes incredible
     *    amounts of system resources as it waits at a scanf()), but
     *    we want to be sure it doesn't terminate while it's writing
     *    files (.act, ordbook.dat, diary.out).  So we shut it off after
     *    20 seconds.
    */

    switch( signame ) {
      case SIGINT: break;
      case SIGQUIT: break;
      case SIGHUP: alarm(30); break;
      case SIGALRM: {
	   update_connect_time( YES );  /* dump_now = YES*/
           exit(0);
       }
    }
} /* end of sig_handler() */
#endif
/*--------------------------*/

