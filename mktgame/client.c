/* function for determining if the server is paying attention to us.
 * First we see if server is paying attention, ( server_attention() )
 *   then we sleep(10), then we tell the server we're done ( server_goodbye).
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "communic.h"
#include "server_error.h"

/**** FUNCTION DECLARATIONS ****/
int  server_avail( void );
void server_ready( int handle );
void server_goodbye( int handle );

/**** FUNCTION DEFINITIONS ****/

int server_avail( void ) {
  /* Return the socket handle if we have the server's attention,
   *    return -1 if we don't.
   * By returning the socket handle, we can close() the connection
   *    in the calling routine, when we don't need to monopolize
   *    the server any more.
  */
  char sockaddress[80];
  register int i, s, len;
  int result;
  struct sockaddr_un saun;
  HANDSHAKE handshake;

  /* request a socket for itself */
  if( (s = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0 ) {
    server_error("client: socket");   /* prints to stderr */
    exit(1);
  }

  /* Create the address we'll be connecting to */
  saun.sun_family = AF_UNIX;
  strcpy( saun.sun_path, ADDRESS );
 
  len = sizeof( saun.sun_family) + strlen(saun.sun_path) + 1;

  /* request a connection to the server */
  if( (connect(s, &saun, len) < 0) ) 
      result = -1;
  else 
      result = s;

  return result;
}  /* end of server_avail() */
/*------------------------*/
void server_ready( int handle ) {
   /* We'll send the server a handshake.  When the server responds,
    *   it'll be a sign the server is giving us its undivided attention.
  */
  HANDSHAKE handshake;

  handshake.message = FRIENDLY;

  /* printf( "waiting for server...\n" );*/
  send( handle, &handshake, sizeof(HANDSHAKE), 0 );
  recv( handle, &handshake, sizeof(HANDSHAKE), 0 ); 

} /* end of server_ready() */
/*----------------------------*/

void server_goodbye( int handle ) {
    /* the server won't service another client until this function
     *   sends the server a goodbye handshake.
    */ 
    HANDSHAKE handshake;
    int rtrnval;

    handshake.message = GOODBYE;

    rtrnval = send( handle, &handshake, sizeof(HANDSHAKE), 0 );

    close( handle );

} /* end of server_goodbye() */
/*------------------------------*/

void server_error( char *message ) {
    FILE *outfile;
    outfile = fopen( "server_error.out", "a" );

    fprintf( outfile, "%s\n", message );
}



