/* Fully-automated, continuous trading system. */

/**** INCLUDES ****/
#include <fstream.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../aqc.hpp"
#ifndef __TURBOC__
#include <unistd.h> /* for alarm() */
#endif

/**** LOCAL FUNCTION DECLARATIONS ****/
void sig_handler( int signame );

/**** FUNCTION DEFINITIONS ****/
main() {
    int handle;
    NAMES user;


    // Disable keyboard-producible signals.
      signal( SIGINT, sig_handler );
      signal( SIGQUIT, sig_handler );
      signal( SIGHUP, sig_handler );
      signal( SIGALRM, sig_handler );

    // load data about the securities:
    G::Securitydata = load_securities();

    // load players' names and initialize N_players:
    G::N_players = load_players( &G::Names );

 	//-- Get userid and pwd from stdin:
	char stdinput[128]; 
	char* username;
	char* userpwd;

	cin.getline( stdinput, 128 );
	username = strtok(stdinput," \t\n");
	if( !username ) 
		error( "Usage: \"aqc userid password\"\n" );
	else 
		strncpy( user.name, username, NAMELEN );

	userpwd  = strtok(NULL," \t\n");		
	if( !userpwd ) 
		error( "Usage: \"aqc userid password\"\n" );
	else 
		strncpy( user.pwd, userpwd, NAMELEN );
	//-------------------

    if( !nameok( &user, G::Names, G::N_players ) )
            error( "Bad user name or password." );
    
    announcement();

    /* warn if server is down */
    if( (handle=server_avail()) < 0 ) 
        client_warning( "**The server is unavailable; you can look, but you can\'t place orders.***" );
    else {
        server_ready( handle ); // If omit this, I think the server 
								// dies (althought maybe not right away).
        server_goodbye( handle );
    }

    // Play ball!
    menu1( user.name );

    update_connect_time( YES );  /* YES=dump_now     */

} /* end of main() */
/*--------------------------*/

// Get rid of the entire sig_handler() function.
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
