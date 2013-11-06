#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "xmem.h"
#include "aqc.h"

/**** GLOBAL VARIABLE ****/
extern int N_sec;

/**** LOCAL FUNCTION DECLARATIONS ****/
int dblcmp( double *x, double *y );

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void actfile_name( char *name, char *security, char *actfilename )
int ismoney( SECURITY *sec );
char *capname( char * );
void client_warning( char *message )
char *currency_id( SECURITY *sec );
void do_nothing( void )
void error( char *message )
SECURITY *find_sec( char *sec_name, SECURITY **securities )
char get_disp_cmd( void )
double mean( VECT *vect, double trimfactor );
void ordbook_name( char *security, char *ordbookname )
ORDER *order_alloc( void )
void    order_free( ORDER *order )
int   *permute( int sup );
HOLDINGS *holding_alloc( void )
void    holding_free( HOLDINGS *holding )
void prospectus( char *security )
double round( double x, double y )
char *shasctime( struct tm *zman )
int string_length( char *str )
void stringlwr( char *str )
TRADE *trade_alloc( void )
void    trade_free( TRADE *trade )
time_t unix_seconds(int yy,int mm,int dd, int hh,int min,int ss);
double variance( VECT *vect, double trimfactor );
void actfile_name( char *name, char *security, char *actfilename )
****/

/**** FUNCTION DEFINITIONS ****/
void error( char *message ) {
  printf( "\n%s\n", message );
  exit(0);
} /* end of error() */
/*--------------------------*/

void client_warning( char *message ) {
  printf( "\n%s\n", message );
} /* end of client_warning() */
/*--------------------------*/

ORDER *order_alloc( void ) {
    /* allocate an ORDER type and initialize it to zero */
    ORDER *result;

    result = (ORDER *)x_calloc( (size_t)1, sizeof(ORDER) );

    return result;
} /* end of order_alloc() */
/*--------------------*/

TRADE *trade_alloc( void ) {
    /* allocate an TRADE type and initialize it to zero */
    TRADE *result;

    result = (TRADE *)x_calloc( (size_t)1, sizeof(TRADE) );

    return result;
} /* end of trade_alloc() */
/*--------------------*/

HOLDINGS *holding_alloc( void ) {
    /* allocate an HOLDINGS type and initialize it to zero */
    HOLDINGS *result;

    result = (HOLDINGS *)x_calloc( (size_t)1, sizeof(HOLDINGS) );

    return result;
} /* end of holding_alloc() */
/*--------------------*/

void    order_free( ORDER *order ) {
  x_free( (char *)order );
}

void    trade_free( TRADE *trade ) {
  x_free( (char *)trade );
}

void    holding_free( HOLDINGS *holding ) {
  x_free( (char *)holding );
}
/*--------------------*/

void actfile_name( char *name, char *security, char *actfilename ) {
  /* generates the name of the file where we keep
   *   the history of <name>'s activities--orders, trades, holdings.
   * Its directory is "<security>act".
  */

  strcpy( actfilename, security );
  strcat( actfilename, "act/" );
  strcat( actfilename, name );
  strcat( actfilename, ".act" );

} /* end of actfile_name() */
/*--------------------*/

char *shasctime( struct tm *zman ) {
  /* like C's asctime(), but leaves off the day of the week and the year,
   * to save horizontal space on the display */
  static char timestr[30];

  strcpy( timestr, asctime(zman) );
  timestr[19] = 0;

  return timestr+4;
} /* end of shasctime() */
/*--------------------*/

double round( double x, double y ) {
  /* round x to nearest y */
  double result;

  result = y * floor( x/y + 0.5 );
  return result;
} /* end of round() */
/*--------------------*/

int string_length( char *str ) {
  int i=0;
  while( str[i] != 0 ) i++;

  return i;
} /* end of string_length() */
/*--------------------*/

char get_disp_cmd( void ) {
    /* prompts and gets command concerning further scrolling of the display
     * for trade history, the order book, the diary, etc.
    */
    char cmdstr[120];

    printf( "Enter n to continue, q to quit > " );
    fflush( stdin );
    scanf( "%s", cmdstr );
    return cmdstr[0];
} /* end of get_disp_cmd() */
/*--------------------*/

void do_nothing( void ) {
/* for substituting for the curses functions when we don't want to use them */
} /* end of do_nothing() */
/*--------------------*/

void ordbook_name( char *security, char *ordbookname ) {
    strcpy( ordbookname, security );
    strcat( ordbookname, "act/" );
    strcat( ordbookname, ORDBOOKNAME );
} /* end of ordbookname() */
/*--------------------*/

void prospectus( char *security ) {
  /* print <security>.pro, the prospectus file for security */
  char wholeline[81], filename[100];
  FILE *infile;

  strcpy( filename, security );
  strcat( filename, ".pro" );

  infile = fopen( filename, "r" );

  if( infile )
  while( !feof(infile) ) {
      fgets( wholeline, 80, infile );
      fputs( wholeline, stdout );
  }
  printf( "\n" );
  fclose( infile );

} /* end of prospectus() */
/*---------------------------*/

int *permute( int sup ) {
  /* Produce a sup-long vector containing a permutation of the integers
   *   0,...,sup-1.
   * Algorithm: produce sup random numbers alongside an index, then sort
   *   the random numbers.  The corresponding integers are now a random
   *   permutation of {0,...,sup-1}.
   * This works pretty well, unless sup is big enough that we have a high
   *   prob of getting the same result twice from rand().  With 32-bit
   *   integers, this is nothing to worry about.
  */
  int i, *result;
  time_t zman;
  PAIR *pairs;

  /* see the random number generator */
  time(&zman);
  srand((unsigned int)zman);

  pairs = (PAIR *)x_malloc( sup*sizeof(PAIR) );
  result = (int *)x_malloc( sup*sizeof(int) );

  for( i=0; i<sup; i++ ) {
      pairs[i].index = i;
      pairs[i].mikri = rand();
  }

  qsort( pairs, (size_t)sup, sizeof(PAIR), paircmp );

  for( i=0; i<sup; i++ )
      result[i] = pairs[i].index;

  x_free( (char *)pairs );
  return result;
} /* end of permute() */
/*-------------------*/

int paircmp( PAIR *p1, PAIR *p2 ) {
   /* comparison function for qsort() */
   return p1->mikri - p2->mikri;
} /* end of intcmp() */
/*--------------------*/

time_t unix_seconds(int yy,int mm,int dd, int hh,int min,int ss) {
  /* seconds from 1/1/70 to mmddyy, hhminss Pacific Daylight Time */
  int days;
  time_t result, seconds;

  static int cumdays[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
  int numleaps;
  numleaps=(yy-70)/4 + 1;
  if ( (mm<=2) && (yy%4 == 0) ) numleaps=numleaps-1;
  days = 365*(yy-70) + cumdays[mm-1] + dd + numleaps - 2;

  /* compute seconds-since-1/1/70 equivalent to date and time */
  seconds = (hh+8)*3600L + min*60 + ss;
  result = (long)days*24*3600L + seconds;

  return result;
} /* end of unix_seconds */
/*---------------*/

char *capname( char *name ) {
  /* returns name with first letter capitalized */
  static char result[NAMELEN+1];

  strcpy( result, name );
  result[0] = toupper( result[0] );

  return result;
} /* end of capname */
/*--------------------*/
#ifndef __TURBOC__
void strlwr( char *str ) {
  /* converts all the characters in str to lower-case.
   * This is a regular <string.h> function in Watcom and Turbo C
  */
  int i;

  for( i=0; i<strlen( str ); i++ )
      str[i] = tolower( str[i] );
} /* end of strlwr() */
#endif
/*------------------------*/

void announcement( void ) {
  /* print an announcement, right after someone logs in */
  char wholeline[81];
  FILE *infile;

  infile = fopen( ANNOUNCEMENT, "r" );

if( infile ) {
  fgets( wholeline, 80, infile );
  while( !feof(infile) ) {
      printf( "%s", wholeline );
      fgets( wholeline, 80, infile );
  }
  fclose( infile );
}

} /* end of announcement() */
/*---------------------------*/

SECURITY *find_sec( char *sec_name, SECURITY **securities ) {
  /* return the element of **securities that pertains to the security
   * whose name is sec_name.
  */
  int i=0;
  SECURITY *result;

  while( ( i<N_sec ) && ( strcmp( sec_name, securities[i]->name ) ) )
    i++;

  if( i==N_sec )
     result = NULL;   /* this is the case when security = "endmt" */
  else
      result = securities[i];
  return result;
} /* end of find_sec() */
/*---------------------------*/

int ismoney( SECURITY *sec ) {
   /* Return YES if sec is a money, NO if it's a proper security */
   if( ( sec==NULL ) || ( (sec->forcemkt)[0] == 0 ) )
       return NO;
   else
       return YES;
} /* end of ismoney() */
/*---------------------------*/

char *currency_id( SECURITY *sec ) {
  /* returns e.g. "(prices in pounds)" if there's more than one
   *   currency in the game.  If there's only one, returns "".
  */
  static char result[50];

  /* fill result */
  if( n_currencies() == 1 )
      result[0] = 0;
  else {
      strcpy( result, "(prices in " );
      strcat( result, sec->currency );
      strcat( result, "s) " );
  }

  return result;
} /* end of currency_id() */
/*---------------------------*/

double mean( VECT *vect, double trimfactor ) {
/* trimmed mean */
    int i, low, high;
    double sum=0;

    /* sort vect */
    qsort( vect->data, vect->n, sizeof(double), dblcmp );

    /* find mean of what's within the extremes */
    low = (int)(vect->n * trimfactor);
    high = (int)(vect->n * (1-trimfactor));

    for( i=low; i<high; i++ )
        sum += vect->data[i];

    return sum/(high-low);
} /* end of mean() */
/*------------------------------------- */

double variance( VECT *vect, double trimfactor ) {
/* trimmed variance    */
    int i, low, high;
    double sum=0, sumsq=0, mean;

    /* sort vect */ 
    /* find variance of what's within the extremes  */
    low = (int)(vect->n * trimfactor);
    high = (int)(vect->n *(1-trimfactor));

    for( i=low; i<high; i++ ) {
        sum += vect->data[i];
        sumsq += vect->data[i] * vect->data[i];
    }

    mean = sum/(high-low);
    return (sumsq - sum*sum/(high-low))/(high-low-1);
}
/* end of variance() */
/*-------------------------------------*/

int dblcmp( double *x, double *y ) {
    if( *x>*y ) return 1;
    if( *x<*y ) return -1;
    if( *x==*y ) return 0;
} /* end of dblcmp()*/

/*---------------------------*/
/*---------------------------*/
/*---------------------------*/


