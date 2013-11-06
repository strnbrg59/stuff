/* Server program.  Accepts socket hookup from a client, and waits
 *   for goodbye signal before becoming receptive to the next client.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "../communic.h"
#include "../server_error.h"

/**** FUNCTION DECLARATIONS ****/
int servsock( char *sockaddr );

/**** FUNCTION DEFINITIONS ****/
main() {
  int fromlen;
  register int i, s, ns, len;
  struct sockaddr_un fsaun;  /* client socket */
  HANDSHAKE handshake;

  /* set up server socket, bind it, and start it listening */
  s = servsock( ADDRESS );

  while(1) {
      if( (ns = accept(s, &fsaun, &fromlen) ) < 0) {
        server_error("server: accept");
        exit(1);
      }

      /* check handshake signal from client--this is from server_ready() */
      recv( ns, &handshake, sizeof(HANDSHAKE), 0 );

      if( handshake.message == DEADLY ) {
    	  server_error("server: killed");
	      exit(0);
      }
      else {
          send( ns, &handshake, sizeof(HANDSHAKE), 0 );
          /*	printf( "server: received and returned friendly handshake.\n" ); */
          /*	printf( "server: awaiting goodbye handshake...\n" );             */

          /* await message from server_goodbye() */
          recv( ns, &handshake, sizeof(HANDSHAKE), 0 );
          /* this time we don't care what's in the handshake--just
           *   that we get it */
          close( ns );
      }
  }

  close(s);
} /* end of main() */
/***************************/





