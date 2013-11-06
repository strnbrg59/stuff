/* function for determining if the server is paying attention to us.
 *
 * This version is for DOS; it really doesn't do anything but alert us to
 *   what the UNIX version _would_ be doing, e.g. attempting to get the
 *   server's attention, determining if the server is ready, and letting the
 *   server go.
*/
#include <dos.h>
#include <stdio.h>

/**** FUNCTIONS VISIBLE EXTERNALLY ****/
int  server_avail( void );
void server_ready( int handle );
void server_goodbye( int handle );

/**** FUNCTION DEFINITIONS ****/

int server_avail( void ) {
  return 1;
}  /* end of server_avail() */
/*------------------------*/

void server_ready( int handle ) {
  if( handle<0 ) handle++; /* to avoid compiler warning */
  printf( "waiting for server...\n" );
  delay(300);
} /* end of server_ready() */
/*----------------------------*/

void server_goodbye( int handle ) {
  if(handle<0) handle++; /* to avoid compiler warning */
  delay(300);
} /* end of server_goodbye() */
/*------------------------------*/






































