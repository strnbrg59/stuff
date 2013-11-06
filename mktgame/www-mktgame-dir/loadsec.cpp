/* Load data on the securities from a file.  First line of file is
 *   the number of securities.  Thereafter the format is as follows:
 *   (menulisting: gives the full name of the security as it's going
 *   to appear in the menu display, and then the one-letter code the
 *   user uses to select this security from the menu.)
 *
 *   (option: says what, if anything, the security is an option on,
 *   whether it's a call or put, and strike price, in that order.)
 *   You don't need to say American or European, as only American options
 *   need any sort of special treatment anyway.
 *
 *   Currencies have a forcemkt line but no option line.  Proper securities
 *   have an option line but no forcemkt line.
 *
 *   Checks for existence of directories for .act files.
 *
 *   Security.par looks like this...

N_sec: 5

name: dbond
currency: dolar
menulisting: dolar_bond d
option: NULL c 0
maxorders: 10

name: pbond
currency: pound
menulisting: pound_bond p
option: NULL c 0
maxorders: 10

name: option
currency: dolar
menulisting: dbond_call_option o
option: dbond p 8.75
maxorders: 10

name: dolar
currency: pound
menulisting: dolar D
forcemkt: dbond
maxorders: 10

name: pound
currency: dolar
menulisting: pound P
forcemkt: pbond
maxorders: 10
 ***********************
*/

#include <sys/types.h>
#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <String.h>
#include "xmem.hpp"
#include "aqc.hpp"

/**** DEFINITIONS ****/
#define LINELEN 80

/**** FUNCTIONS VISIBLE EXTERNALLY ****/
int is_sec_choice( String menuoption );
int n_currencies( void );

/**** LOCAL FUNCTION DECLARATIONS ****/
static void load_sec( Security *security, FILE *infile );
static void sec_alloc( Security ***security, FILE *infile );
static int directories_exist( Security **securitydata );

/**** FUNCTION DEFINITIONS ****/
Security **load_securities( void ) {
  int i;
  FILE *infile;
  Security **result;

  /* open input file */
  infile = fopen( SecurityFILE, "r" );
  if( infile==NULL ) error( "Failed to open SecurityFILE." );

  /* find out how many securities there are, and allocate space for them */
  sec_alloc( &result, infile );  /* also sets G::N_sec */

  /* load the security data */
  for( i=0; i<G::N_sec; i++ )
      load_sec( result[i], infile );

  fclose( infile );

  if( !directories_exist( result ) )
      error( "Can't run without .act-file directories. Exiting now." );
  
  return result;
} /* end of load_securities() */
/*----------------------------*/

static void sec_alloc(
  Security ***security, FILE *infile ) {
  /* find out how many securities there are, and allocate space for them */
  char wholeline[LINELEN+1];
  int i;

  fgets( wholeline, LINELEN, infile );
  strtok( wholeline, " \n\t" );
  G::N_sec = atoi( strtok( NULL, " \n\t" ));
  if( (G::N_sec < 1) || (G::N_sec > 8) ) {
      printf( "%d is an unreasonable number of securities--check %s file.\n",
          G::N_sec, SecurityFILE );
      exit(0);
  }
  *security = (Security **)x_malloc( G::N_sec * sizeof(Security *) );

  for( i=0; i<G::N_sec; i++ ) {
      (*security)[i]   = (Security *)x_calloc( 1, sizeof(Security) );
      /* important to calloc, because we're counting on proper securities
       * (i.e. non-currencies) having 0x00's in their forcemkt field.
      */
  }
} /* end of sec_alloc() */
/*----------------------------*/

static void load_sec( Security *sec, FILE *infile ) {
  /* Load the data on one security.  Parameters point at the i-th element
   *   of the respective 2-D arrays of pointers.
  */
  char wholeline[LINELEN+1], garbage[LINELEN/2];

  fgets(wholeline, LINELEN, infile ); /* picks up the '\n' in separator line */

  /* get security name and currency */
  fscanf( infile, "%s %s", garbage, sec->name );
  if( strlen( sec->name ) > NAMELEN-3 )
      error( "security name is too long." );
  fscanf( infile, "%s %s", garbage, sec->currency );


  /* get menu listing */
  fscanf( infile, "%s %s %s", garbage, sec->menu_fullname, sec->menu_choice );

  /* get forcemkt or option data, as the case may be */
  fgets( wholeline, LINELEN, infile );  /* picks up trailing '\n' */
  fgets( wholeline, LINELEN, infile );
  if( !strcmp( strtok( wholeline, " " ), "forcemkt:" ) ) {
      strcpy( sec->forcemkt, strtok( NULL, " \n" ) );
      strcpy( sec->underlying, "NULL" );
  }
  else {
      strcpy( sec->underlying, strtok( NULL, " " ) );
      strcpy( sec->call_or_put, strtok( NULL, " " ) );
      sec->strikeprice = atof( strtok( NULL, " \n" ) );
  }

  /* get maxorders--maximum allowed number of market or limit orders */
  fgets( wholeline, LINELEN, infile );
  sscanf( wholeline, "%s %d", garbage, &(sec->maxorders) );
} /* end of load_sec() */
/*----------------------------*/

int is_sec_choice( String menu_choice ) {
   // Used in menu.c.  For menu_choice the one-letter designator of a
   //   security, returns element of Securitydata that points to that
   //   security's data.  For menu_choice something else (e.g. #,q,error)
   //   returns -1.  (Can't return 0, as that's an element of Securitydata.)
   //
   int i, result=-1;

   for( i=0; i<G::N_sec; i++ )
       if( menu_choice == String(G::securitydata[i]->menu_fullname) ) {
           result = i;
           break;
       }

   return result;
} // is_sec_choice()
/*----------------------------*/

int n_currencies( void ) {
  /* returns number of currencies */
  int i, result=0;

  for( i=0; i<G::N_sec; i++ )
      if( ismoney( G::securitydata[i] ) == YES )
          result++;

  return result;
} /* end of n_currencies() */
/*----------------------------*/

static int directories_exist( Security **securitydata ) {
  /* Checks for existence of a directory for the .act files of each
   *   security.
  */
  char dirname[ACTNAMELEN];
  int i, result=1;
  DIR *dp;

  for( i=0;i<G::N_sec;i++ ) {
	  strcpy( dirname, WWWMKTGAMEDIR );
      strcat( dirname, securitydata[i]->name );
      strcat( dirname, "act" );
      if( (dp=opendir(dirname))==NULL ) {
	  printf( "Directory %s does not exist", dirname );
	  result = 0;
      }
      else closedir( dp );
  }

  return result;
} /* end of directories_exist() */
/*----------------------------*/
