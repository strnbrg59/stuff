// Strategy for market game client.
//
// 1. Does "socket" file shiq?i exist?
//	Yes: keep polling; another client is working on order book.
//	No {
//		a. create shiq?i
//		b. put a unique ID number into it, to identify client.
//			The ID could be the client's process number.
//
//		c. Does server-shiq?i exist, and does it contain the ID#?  
//			(Server looks for shiq?i, and
//			when it finds shiq?i, creates server-shiq?i and puts
//			the client's ID number into it.  But the server doesn't
//			do anything as long as server-shiq?i exists; removal of
//			server-shiq?i is the client's signal that it's done.
//
//			No: keep polling, but not for long, before dropping to 
//				step 1.
//
//			Yes {
//				i. Do stuff to order book.
//			   ii. Delete server-shiq?i
//			  iii. Delete shiq?i
//			}
//	}
//
//
//	For compatibility with the existing mktgame code, we'll roll this
//	scheme up into functions with the same names we used before:
//
//	int  server_avail(): 
//		Determines if the server program is up at all.  Returns -1 if not.
//		(Returns handle if server is up, but we won't need the handle.)
//
//	void server_ready( int handle ):
//		Grabs the server.  This should be everything down to step 1.c's "Yes"
//			procedure.
//
//	void server_goodbye( int handle ):
//		Steps 1.c.ii & iii.
//

#include <unistd.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <String.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fcommunic.hpp"

/*
main() {
	String username( "andy" );
	cout << server_avail( username );
	server_ready( 0 );
	server_goodbye( 0 );
} 
*/

int server_avail( String username ) {

	// Is server running?  I don't know what library function will
	// tell me if something called "server" is up and running.
	// So instead, the server demonstrates its aliveness by modifying
	// a file every few seconds.  That file is SERVACTIVITYFILE 
	// (defined in fcommunic.hpp).

	struct stat buf;
	stat( SERVACTIVITYFILE, &buf );
	if( time(NULL) - buf.st_mtime > 60 ) {
		cout << "server activity file not touched in " <<
			    time(NULL) - buf.st_mtime << " seconds.\n";		
		return server_down;
	}
	else
		return abs(int( (int *)(char *)username ) * int(time(NULL))); 

} // server_avail()
//----------------------------	

void server_ready( int handle ) {

// Does "socket" file shiq?i exist?
// If yes, keep polling.
	FILE* sockfile;
	sockfile = fopen( SOCKETFILENAME, "r" );
	int i=0;
	while( sockfile && (i<MAXTRIES) ) {
		usleep(200000);
		fclose( sockfile );
		sockfile = fopen( SOCKETFILENAME, "r" );		
		i++;
	}
	// If polled maximum number of tries, give up.  
	if( i== MAXTRIES ) {
		cout << "Server is too busy.\n";
		exit(0);
	}
	// It would be more elegant to return something, but we want to
	// be compatible with the BSD socket version of these functions.

// OK, shiq?i doesn't exist, so...
//		a. create shiq?i
//		b. put a unique ID number into it, to identify client.
//			The ID could be the microseconds component of system time.

	sockfile = fopen( SOCKETFILENAME, "wb" );

	fwrite( &handle, sizeof(int), 1, sockfile );
	fflush( sockfile );
	fclose( sockfile );

//		c. Does server-shiq?i exist, and does it contain the ID#?  
//			(Server looks for shiq?i, and
//			when it finds shiq?i, creates server-shiq?i and puts
//			the client's ID number into it.)
	usleep(100000); // to give the server a chance.
	FILE* servsockfile = fopen( SERVSOCKFILENAME, "rb" );
	i=0;
	while( ( !servsockfile ) && (i<MAXTRIES) ) {
		usleep(200000);
		servsockfile = fopen( SERVSOCKFILENAME, "rb" );
		i++;
	}
	if( i==MAXTRIES ) {
		cout << "Server isn\'t creating " << SERVSOCKFILENAME << '\n';
		exit(0);
	}

	// If we got to here, we found servsockfile.  All that remains, to confirm
	// 	a go-ahead from the server, is to check that this file contains our
	// 	handle.
	// But if by rare chance another client got in there and created its own
	//	socket between the time we created ours and the time we got around
	//	to putting our id number in it (this will happen very rarely), better
	//	just abort.  It's no big deal for the user to press the "Submit" button
	//	on his server again.
	
	// Close and reopen servsockfile; when we opened it, the server might
	// have not finished writing to it.
	fclose( servsockfile );
	usleep(800000);
	servsockfile = fopen( SERVSOCKFILENAME, "rb" );	

	int handle_copy;
	fread( &handle_copy, sizeof(int), 1, servsockfile );
	fclose( servsockfile );

	if( handle_copy != handle ) {
		cout << "Server busy.  Try again.\n" 
		     << " client: " << handle << ", server: " 
		     << handle_copy << '\n';
		exit(0);
	}
		
} // server_ready()
//----------------------------

void server_goodbye( int handle ) {
	handle++; // to avoid compiler warning

	remove( SOCKETFILENAME );
	remove( SERVSOCKFILENAME );
	// Important to remove them in this order, because of the order
	// in which the server looks for their existence!
}
//----------------------------
	