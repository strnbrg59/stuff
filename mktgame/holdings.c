/*  File HOLDINGS.C  (formerly portf.c) */
/*  functions for handling the player name file (players.dat) and anything
 *    having to do with the HOLDINGS structure.
 *
 *  Players.dat has login names of all people who may play.
*/

/**** INCLUDES ****/
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "aqc.h"
#include "xmem.h"

/**** GLOBAL VARIABLES ****/
extern int N_players;
extern NAMES *Names;
extern SECURITY **Securitydata;
extern int N_sec;

/**** TYPDEF ****/
typedef struct ZSCORE {
    double W;  /* final, total wealth, adjusted for exchange rate */
    double z;
} ZSCORE;

/**** LOCAL FUNCTION DECLARATIONS ****/
static int alphcompare( NAMES *holding1, NAMES *holding2 ); /* for qsort() */
static int holdingnamecmp( HOLDINGS *holding1, HOLDINGS *holding2 );
static int illegal_name( char *knickname );
static double midspread( ORDER *bestbid, ORDER *bestask );
static void print_holdings( FILE *outfile, int section );
static ZSCORE *zscores( int section, double e_rate );

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void finger( char *ownname );
void dump_players( NAMES *names, int n );
void free_names( NAMES *names );
void free_holdings( HOLDINGS *holdings );
void holding_list( char *name )
int load_players( NAMES **names );
HOLDINGS *load_holdings( NAMES *names, char *security );
HOLDINGS *load_holding( char *name, char *security );
void update_money( TRADE *trade, char *security );
void update_holding( TRADE *trade, HOLDINGS *holding );
double value_portf( char *name );
HOLDINGS *find_holding( char *name, HOLDINGS *holdings );
****/

/**** FUNCTION DEFINITIONS ****/

int load_players( NAMES **names ) {
  /* Load the data, from players.dat, into *holding.  Allocate *holding.
   * This has to be done every time the system has been down.
   * Warn about illegal characters, overly-long names, and duplicate
   *   names. 
   * Return the number of players. */

  char wholeline[121], playerfilename[ACTNAMELEN], *name, *pwd, *secch,
      *realname;
  int i, n=0;  /* counts number of players ( return value ) */
  FILE *infile;

  /* open infile */
  strcpy( playerfilename, PLAYERFILE );
  infile = fopen( playerfilename, "r" );
  if( infile == NULL ) error( "can't open player names file." );

  /* Count how many players we have, so as to know how many elements of
   *    type NAMES to allocate. */
  wholeline[0] = 0;
  while( wholeline[0] != '%' ) {
      if( feof(infile) ) error( "No \% at end of players.dat." );
      n++;
      fgets( wholeline, 120, infile );
  }
  n --;

  /* allocate the NAMES array and zero it out */
  *names = ( NAMES * )x_calloc( (size_t)n, sizeof( NAMES ) );

  /* now load all the players' names, testing for illegalities */
  rewind( infile );
  for( i=0; i<n; i++ ) {
      fgets( wholeline, 120, infile );

      secch = strtok( wholeline, " \n\t" );
      (*names)[i].section = atoi( secch );
      if( ((*names)[i].section < -2) || ((*names)[i].section > 10) ) {
          printf( "%s :", secch );
          error( "Invalid section number in players.dat" );
      }

      realname = strtok( NULL, " \n\t" );
      if( strlen( realname ) > 3*NAMELEN ) {
	  printf( "%s :", realname );
	  error( "Name is too long." );
      }
      strncpy( (*names)[i].realname, realname, 3*NAMELEN );

      name = strtok( NULL, " \n\t" );
      if( strlen( name ) > NAMELEN ) {
	  printf( "%s :", name );
	  error( "Name is too long." );
      }
      if( illegal_name( name )) {
          printf( "%s :", name );
	  error( "Illegal name, legal is alphanumeric only." );
      }
      strncpy( (*names)[i].name, name, NAMELEN );

      pwd = strtok( NULL, " \n\t" );
      if( strlen( pwd ) > NAMELEN ) {
	  printf( "%s :", pwd );
	  error( "Password is too long." );
      }
      strncpy( (*names)[i].pwd, pwd, NAMELEN );
  }

  fclose( infile );

  /* sort on names */
  qsort( (void *)(*names), (size_t)n, sizeof(NAMES), alphcompare );

  return n;
} /* end of load_players() */
/*------------------------------*/

static int alphcompare( NAMES *names1, NAMES *names2 ) {
  return strcmp( names1->name, names2->name );
} /* end of alphcompare() */
/*------------------------------*/

void dump_players( NAMES *names, int n ) {
  /* dumps to the same file load_players() reads from, so be careful. */
  char outfilename[ACTNAMELEN];  
  int i;
  FILE *outfile;

  /* open outfile */
  strcpy( outfilename, PLAYERFILE );
  outfile = fopen( outfilename, "w" );

  for( i=0; i<n; i++ ) {
      fprintf( outfile, "%10s ", names[i].name );
      fprintf( outfile, "%10s\n", names[i].pwd );
  }

  fprintf( outfile, "%%" );
  /* Indicates eof; when we load from the players
   *   file, if we allocate one too many elements, we get the same player's
   *   record appearing twice, and that means trouble when we update his
   *   holding and later go looking for it.
  */

  fclose( outfile );
} /* end of dump_players() */
/*------------------------------*/

HOLDINGS *load_holdings( NAMES *names, char *security ) {
  /* load everyone's holding from the last line of their .act file,
   *   if there's an .act file.  If there's not, exit; that's an error.
   * Look in the directory named "<security>act"
   *
   * names holds everyone's names (sorted), of which there are n.
  */
  char actfilename[ACTNAMELEN];
  int i;
  FILE *actfile;
  HOLDINGS *result;

  /* allocate return value */
  result = (HOLDINGS *)x_calloc( (size_t)N_players, sizeof(HOLDINGS) );

  /* loop over everyone's names.  */
  for( i=0; i<N_players; i++ ) {

      actfile_name( names[i].name, security, actfilename );
      actfile = fopen( actfilename, "rb" );

      if( actfile == NULL ) {
          printf("%d %s %s :", i, names[i].name, actfilename );
          error( "No holding on this person!" );
      }
      else {
          /* read from actfile.  Portfolio data is one HOLDINGS-sized
           * chunk of data from the end. */
          fseek( actfile, (-1)*(long)(sizeof(HOLDINGS)), SEEK_END );
          fread( result+i, sizeof(HOLDINGS), 1, actfile );
      }

      fclose( actfile );
  }

  return result;

} /* end of load_holdings() */
/*------------------------------*/

HOLDINGS *load_holding( char *name, char *security ) {
  /* load one player's holding from the last line of his .act file,
   *   if there's an .act file.  If there's not, exit; that's an error.
   * Look in the directory named "<security>act"
  */
  char actfilename[ACTNAMELEN];
  FILE *actfile;
  HOLDINGS *result;

  /* allocate return value */
  result = (HOLDINGS *)x_calloc( 1, sizeof(HOLDINGS) );

  actfile_name( name, security, actfilename );
  actfile = fopen( actfilename, "rb" );

  if( actfile == NULL )
      result = NULL;
  else {
      /* read from actfile.  Portfolio data is one HOLDINGS-sized
       * chunk of data from the end. */
      fseek( actfile, (-1)*(long)(sizeof(HOLDINGS)), SEEK_END );
      fread( result, sizeof(HOLDINGS), 1, actfile );
      fclose( actfile );
  }

  return result;
} /* end of load_holding() */
/*------------------------------*/

void update_holding( TRADE *trade, HOLDINGS *holding ) {
  /* Update holding (one player's holdings in one security )
   * in line with *trade.
  */

  /* time stamp */
  time( &(holding->zman) );

  /* update amount */
  holding->amount += trade->amount;

} /* end of update_holding() */
/*-------------------------------*/

static int holdingnamecmp( HOLDINGS *holding1, HOLDINGS *holding2 ) {
  /* for bsearch() */
    char *name1, *name2;

    name1 = holding1->name;
    name2 = holding2->name;

    return strcmp( name1, name2 );
} /* end of holdingnamecmp() */
/*--------------------------*/

HOLDINGS *find_holding( char *name, HOLDINGS *holdings ) {
  /* find the HOLDINGS of the player named *name.
   * Letter case doesn't matter. */
  HOLDINGS *holdingkey, *result;

  holdingkey = holding_alloc();
  strcpy( holdingkey->name, name );
  result = (HOLDINGS *)bsearch( (void *)holdingkey, (void *)holdings, (size_t)N_players,
		  sizeof(HOLDINGS), holdingnamecmp );

  holding_free( holdingkey );

  return result;

} /* end of find_holding() */
/*------------------------------*/

void free_holdings( HOLDINGS *holdings ) {
  /* called just after getting server's attention and just before
   * reloading the authoritative version of holdings.
  */

  x_free( (char *)holdings );
} /* end of free_holdings() */
/*------------------------------*/

void free_names( NAMES *names ) {
  /* used after getting server's attention, and just before reloading
   * names from file */
  x_free( (char *)names );
} /* end of free_names() */
/*------------------------------*/

void update_money( TRADE *trade, HOLDINGS *holding ) {
  /* time stamp */
  time( &(holding->zman) );

  /* update amount */
  holding->amount -= trade->amount * trade->price;
} /* end of update_money() */
/*------------------------------*/

void finger( char *ownname ) {
  /* list all of a person's holdings--money and securities. */
  /* indicate remaining connect time, too. */

  char name[5*NAMELEN];  /* in case user types in a long string of garbage */
  int i;  /* index */
  double mktval;
  HOLDINGS *holding;

  printf( "Enter name of person you'd like to find out about > " );
  scanf( "%s", name );
  printf( "\n" );
  if( strlen(name) > NAMELEN+1 )
      error( "Invalid name...please restart program.\n" );
  fflush( stdin );

  printf( "*** all holdings -- %s ***\n", name );
  printf( "  security        holdings\n" );

  /* get security holdings and display */
  for( i=0; i<N_sec; i++ ) {
      holding = load_holding( name, Securitydata[i]->name );
      if( holding==NULL ) {
          printf("No one here by that name.\n");
          return;
      }
      printf( "    %s      %7.0f\n", Securitydata[i]->name, (float)(holding->amount) );
      holding_free( holding );
  }

  /* indicate market value */
  mktval = value_portf( name );
  if( mktval != 0 ) /* 0 is the error condition for value_portf() */
      printf( "Market value of holdings = %8.3f\n\n", value_portf( name ));

  /* indicate connect time used and remaining 
  if( !strcmp( name, ownname ) )
      disp_connect_time();
  */

} /* end of finger() */
/*----------------------*/

void holding_list( char *name ) {
/* Produce a list of all players and their holdings of the securities and money.
 * If you change the names of the securities, you'll need to go in here
 *    and change char secnames[][] accordingly.
 * Accessible only by "Fed".
*/
  int section;
  FILE *outfile;

  if( strcmp( name, "Fed" ) )
      return;

  /* prompt for section whose holdings you want to print out */
  printf( "Enter desired section number (-1 for all sections): " );
  scanf( "%d", &section );

  /* open outfile--we'll want a printout at the end of the game */
  outfile = fopen( "holdings.out", "w" );

  /* run it once with stdout, once with outfile */
  /*  print_holdings( stdout, section ); */
  print_holdings( outfile, section );

  fclose( outfile );

} /* end of holding_list() */
/*-----------------------------*/

static void print_holdings( FILE *outfile, int section ) {
  /* called twice by holding_list()---once with stdout, once with a disk file*/
  /* load everybody's holdings once and for all */
  /*char *(tsiun[3]);*/
  char charnum[20];
  int i,j;  /* index */
  ZSCORE *zscore;
  double e_rate;  /* exchange rate */
  HOLDINGS **holdings;

  holdings = (HOLDINGS **)x_malloc( N_sec * sizeof(HOLDINGS *) );
  for( i=0; i<N_sec; i++ )
      holdings[i] = load_holdings( Names, Securitydata[i]->name );

  /* compute zscores and letter grades, based on money holdings */
  printf( "Enter exchange rate: \n" );
  scanf( "%s", charnum );
  e_rate = atof( charnum );
  zscore = zscores( section, e_rate );

  /* header */
  fprintf( outfile,  "%8s", "name" );
  fprintf( outfile,  "%10s%8s", "realname", " " );
  for( i=0; i<N_sec; i++ )
          fprintf( outfile,  "%10s  ", Securitydata[i]->name );
  fprintf( outfile, "%9s  %9s  %9s\n", "totwealth", "zscore", "mkt_value" );

  /* loop over all players */
  for( j=0; j<N_players; j++ )
  if( (Names[j].section == section) || ( section == -1 ) )
  {
      /*fprintf( outfile, "%2d ", Names[j].section );*/
      fprintf( outfile,  "%-10s", Names[j].name );
      fprintf( outfile,  "%-30s", Names[j].realname );
      for( i=0; i<N_sec; i++ ) {
          if( ismoney(find_sec( Securitydata[i]->name, Securitydata )) == NO )
              fprintf( outfile,  "%7d ", (int)holdings[i][j].amount );
          else 
              fprintf( outfile,  "%12.3f ", (float)holdings[i][j].amount );
      }
      fprintf( outfile, "%12.3f %12.3f %12.3f\n", (float)(zscore[j].W), (float)(zscore[j].z), value_portf(Names[j].name ));
  }

  /* free memory */
  for( i=0;i<N_sec;i++ )
      holding_free( holdings[i] );

} /* end of print_holdings() */
/*-------------------------------*/

static ZSCORE *zscores( int section, double e_rate ) {
  /* Compute each player's z-score, based on his holdings of money.
   * Restrict the calculations to players in the indicated section (or
   *   all sections if section is -1.
  */
  /* But with more than one currency, this needs to be interpreted
   *   differently.  So we have e_rate.
  */
  char trimch[20]; /* trimming factor for mean and s.d.     */
  int j,i,
      seen_first_curr, /* flag-- -1 until we get to first of the 2 currencies*/
      n=0;   /* sample size */
  double sum=0, sumsq=0, mu, sd, trimfact;
  ZSCORE *result;
  HOLDINGS *holding;
  VECT vect;

  /* allocate result */
  result = (ZSCORE *)x_malloc( N_players * sizeof(ZSCORE) );
  vect.data = (double *)x_malloc( N_players * sizeof(double) );

  /* get n, sum and sum of squares */
  for( j=0; j<N_players; j++ )
  if( (Names[j].section == section) || ( section == -1 ) ) {
      seen_first_curr = -1;
      for( i=0;i<N_sec;i++ )
	  if( ismoney(Securitydata[i])==YES ) {
              holding = load_holding( Names[j].name, Securitydata[i]->name );
	      seen_first_curr *= -1;
              if( seen_first_curr == 1 )
                  result[j].W = holding->amount;
              else
                  result[j].W += e_rate * holding->amount;
              x_free( (char *)holding );
	  }

      n ++;
      vect.data[n] = result[j].W;
  }

  vect.n = n-1;
  printf( "Enter trimming factor (e.g. 0.05 for 5%) " );
  scanf( "%s", trimch );
  trimfact = atof( trimch );
  mu = mean( &vect, trimfact );  /* trimmed mean */
  sd = pow(variance( &vect, trimfact ), 0.5 );    /* trimmed sd */
  printf( "mu = %f, sd = %f\n", (float)mu, (float)sd );

  /* calculate z-scores and put in *result */
  for( j=0; j<N_players; j++ )
  if( (Names[j].section == section) || ( section == -1 ) ) {
      result[j].z = (result[j].W - mu )/sd;
  }

  return result;
} /* end of zscores() */
/*-----------------------------*/

static int illegal_name( char *knickname ) {
  /* Player's knickname becomes the name of his .act files, and therefore
   *  must not contain any non-alpha, non-punct characters. 
   * It must also not exceed NAMELEN in length (not including terminal \x0). 
  */
  int i=0, result=0;

  while( knickname[i] ) {
      if( !isalnum(knickname[i]) )
	  result = 1;
      i++;
  }
  
  if( i>NAMELEN )
      result = 1;

  return result;
} /* end of illegal_name() */
/*-----------------------------*/

double value_portf( char *name ) {
/* Find market value of a player's portfolio, and return it */
/* Return 0 if any order book is empty on one or two sides. */
/* Doesn't work with more than one currency. */

  int i,  /* index */
      seen_first_curr=NO;  /* flag */
  double result=0, mktval;
  ORDER *bestask, *bestbid;
  HOLDINGS *holding;

  /* get security holdings */
  for( i=0; i<N_sec; i++ ) {
      holding = load_holding( name, Securitydata[i]->name );

      if( ismoney(Securitydata[i])==YES ) 
          if( seen_first_curr==NO ) {
              /* this is the numeraire currency; it won't have an order book */
              seen_first_curr = YES;
              mktval = 1.0;
          }
          else return 0;  /* It won't give the right answer if there's more
                           * than one currency. */    

      else {
          /* find market value of each security */
          /* look at the order book */
          load_ordbook( &bestbid, &bestask, Securitydata[i]->name );

          mktval = midspread( bestbid, bestask );      
          if( mktval == 0 ) /* order book empty on one or two sides */
              return 0;  /* error condition */
      }

      result += holding->amount * mktval;

      holding_free( holding );
  }
  
  return result;  /* but returns 0 above, if any order book is empty */  
  
} /* end of value_portf() */
/*------------------------------*/

static double midspread( ORDER *bestbid, ORDER *bestask ) {
  /* return the midspread */
  double result;

  if( bestbid && bestask ) /* i.e., if there's something on each side */
      result = (bestbid->price + bestask->price)/2.0;  
  else
      result = 0;

  return result;
} /* midspread() */
      
