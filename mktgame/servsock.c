/* Sets up a server socket, binds it and invokes listen().
 *
 * Argument: address (directory path) of the socket we'll create.
 * Returns int handle to the socket
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include "server_error.h"

int servsock( char *sockaddr ) {
  register int s, len;
  struct sockaddr_un saun;

  /* get a socket */
  if( (s = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0 ) {
    server_error("server: socket");   /* prints to stderr */
    exit(1);
  }

  saun.sun_family = AF_UNIX;
  strcpy( saun.sun_path, sockaddr );

  unlink( sockaddr );  /* just in case it's an open file, somehow */
  len = sizeof( saun.sun_family) + strlen(saun.sun_path) + 1;

  /* bind the socket */
  if( bind(s, &saun, len) <0) {
    server_error("server: bind");
    exit(1);
  }

  /* start it listening.  To shut it off, invoke "close(s)" */
  if( listen(s,5) < 0 ) {
    server_error("server: listen");
    exit(1);
  }

  return s;
} /* end of servsock() */

void server_error( char *message ) {
    FILE *outfile;
    outfile = fopen( "server_error.out", "a" );

    fprintf( outfile, "%s\n", message );
}
