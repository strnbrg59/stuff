/* file numactions.c
 * Count number of actions taken by each person.
*/

/**** INCLUDES ****/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqc.h"

/**** GLOBAL VARIABLES (for compatibility with the rest of the aqc programs)**/
int N_players;
NAMES *Names;
SECURITY **Securitydata;
int N_sec;

/**** FUNCTION DEFINITIONS ****/
main() {
  /* counts how many actions each person has taken */
  /* This used to require reading through a whole .act file, but now
   *   we can pick up the required info right from the .act file header.
  */
  char name[NAMELEN+1],
       actfilename[ACTNAMELEN+1];
  int i,j,  /* index */
      actions[100];
  FILE *actfile;
  ACTHEADER actheader;

  N_players = load_players( &Names );
  Securitydata = load_securities();

  /* header */
  printf(  "%8s", "name" );
  printf(  "%10s", "realname" );
  for( i=0; i<N_sec; i++ )
      printf(  "%10s", Securitydata[i]->name );
  printf( "\n" );

  /* loop over all players */
  for( j=0; j<N_players; j++ )  {

      /* print header */
      printf(  "%8s", Names[j].name );
      printf(  "%10s", Names[j].realname );

      strcpy( name, Names[j].name );

      /* loop over securities */
      for( i=0; i<N_sec; i++ ) {
          /* identify and open the .act file */
	  actfile_name( name, Securitydata[i]->name, actfilename );
          actfile = fopen( actfilename, "rb" );

	  fread( &actheader, 1, sizeof(ACTHEADER), actfile );
          actions[i] = actheader.numorders;

	  fclose( actfile );
         
	  printf( "%10d ", actions[i] );
      }
      printf( "\n" );

  }
} /* end of main() */

