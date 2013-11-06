/* Delete limit order books.  Something you want to do before a joint
 *   dividend/investment/capital structure decision.
 * As the order books are cleared, players' .act files are updated to record
 *   the cancelled orders.
*/

/**** INCLUDES ****/
#include <stdio.h>
#include "aqc.h"

/**** GLOBAL VARIABLES (not used) ****/
int N_players;
NAMES *Names;
int N_sec;
SECURITY **Securitydata;

/**** FUNCTION DEFINITIONS ****/
main() {
  char cmd[2];
  int i, handle;

  Securitydata = load_securities();
  N_players = load_players( &Names );

  /* make sure server has been turned off */
#ifndef __TURBOC__
  if( 0 <= (handle = server_avail()) ) error( "Shut off the server, first." );
#endif

  /* go through the securities, and ask if order book is to be deleted */
  for( i=0; i<N_sec; i++ ) {
      printf( "\n%s:  delete order book now? (y|n) > ",
          Securitydata[i]->name );
      scanf( "%s", cmd );

      if( cmd[0] == 'y' )
          remove_all_orders( Securitydata[i]->name );

      fflush( stdin );
  }

return 0;
} /* end of main() */
