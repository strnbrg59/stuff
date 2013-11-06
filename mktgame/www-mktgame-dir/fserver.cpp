// File-based (rather than socket-based) server for market game.

//	The server looks for shiq?i, and
//	when it finds shiq?i, creates server-shiq?i and puts
//	the client's ID number into it.  But the server doesn't
//	do anything as long as server-shiq?i exists; removal of
//	server-shiq?i is the client's signal that it's done.

#include <unistd.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <String.h>
#include <sys/time.h>
#include "fcommunic.hpp"

main() {
	
	FILE* servsockfile;
	FILE* sockfile;
	FILE* server_activityfile = fopen( SERVACTIVITYFILE, "wb" );

	time_t zman1, zman2;
	int i=0;
	const int sleep_microseconds = 10000;

	while(1) {
		
		usleep( sleep_microseconds ); 
		  // If we don't put this in, this function consumes all
		  // the CPU cycles available.  Nicing still makes the server
		  // look bad; it consumes alot of CPU unless something else does.

		// Create evidence server is alive, every 5 seconds, by just
		// writing anything to the server activity file.  fclient will
		// call stat() and check the mtime.  
		if( (i++)%(500000/sleep_microseconds) == 0 ) {
			fseek( server_activityfile, 0, SEEK_SET );
			fwrite( " ", 1, 1, server_activityfile );
			fflush( server_activityfile );			
		}

		servsockfile = fopen( SERVSOCKFILENAME, "r" );		
		sockfile = fopen( SOCKETFILENAME, "rb" );

		if( sockfile && (!servsockfile) ) {
		// This means the client has made a request for the server's
		//	attention.  servsockfile got deleted when the previous
		//	client called server_goodbye().


			// Close and reopen sockfile, in case we opened it just
			// as the client was writing to it, and before it finished
			// doing so.
			int handle;
			fclose( sockfile );
			usleep(800000);
			sockfile = fopen( SOCKETFILENAME, "rb" );
			fread( &handle, sizeof(int),1, sockfile );
			fclose( sockfile );
			sockfile = NULL;

			servsockfile = fopen( SERVSOCKFILENAME, "wb" );
			fwrite( &handle, sizeof(int),1, servsockfile );
			fflush( servsockfile );
			fclose( servsockfile );
			servsockfile = NULL; // so later tests give correct indications.

			// Record the time, for use below.
			zman1 = time(NULL);
		}

		if( sockfile && servsockfile ) {
			// This means the client is still doing its stuff.
			// Or that the client died: in that case delete both
			//	files so future clients aren't locked out.  For
			//	that, we need to know how long this situation
			//	has been in effect; we should give it a few seconds
			//	before we conclude this isn't just the client doing
			//	its thing.

				fclose( servsockfile );
				servsockfile = NULL;
				fclose( sockfile );
				sockfile = NULL;

				zman2 = time( NULL );
				if( zman2 - zman1 > 10 ) {
					remove( SERVSOCKFILENAME );
					remove( SOCKETFILENAME );
				}
		}

		if( !sockfile && servsockfile ) {
		// This should only occur for very short periods of time--
		//	in the time it takes the server to notice sockfile
		//	and create servsockfile, and (in server_goodbye())
		//	in the time between deletion of sockfile and servsockfile.
			fclose( servsockfile );
		}
	}

} // main()

