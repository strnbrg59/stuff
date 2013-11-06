#include <ctype.h>
#include <fstream.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <String.h>
#include <time.h>
#include "xmem.hpp"
#include "aqc.hpp"

/**** LOCAL FUNCTION DECLARATIONS ****/
int dblcmp( const void *x, const void *y );

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void actfile_name( char *name, char *security, char *actfilename )
int ismoney( Security *sec );
char *capname( char * );
void client_warning( char *message )
char *currency_id( Security *sec );
void do_nothing( void )
void error( char *message )
Security *find_sec( char *sec_name, Security **securities )
char get_disp_cmd( void )
double mean( Vect *vect, double trimfactor );
void ordbook_name( char *security, char *ordbookname )
Order *order_alloc( void )
void    order_free( Order *order )
int   *permute( int sup );
Holdings *holding_alloc( void )
void    holding_free( Holdings *holding )
void prospectus( char *security )
double round( double x, double y )
char *shasctime( struct tm *zman )
int string_length( char *str )
void stringlwr( char *str )
Trade *trade_alloc( void )
void    trade_free( Trade *trade )
time_t unix_seconds(int yy,int mm,int dd, int hh,int min,int ss);
double variance( Vect *vect, double trimfactor );
void actfile_name( char *name, char *security, char *actfilename )
void print_file( String filename );
****/

/**** FUNCTION DEFINITIONS ****/
void error( char *message ) {
  printf( "\n%s\n", message );
  exit(0);
} // error()
//--------------------------

void html_error( char *message ) {
	printf( "Content-type: text/html\n\n" );
	printf( "\n%s\n", message );
	exit(0);
} // html_error()
//--------------------------

void client_warning( char *message ) {
  printf( "\n%s\n", message );
} /* end of client_warning() */
/*--------------------------*/

Order *order_alloc( void ) {
    /* allocate an Order type and initialize it to zero */
    Order *result;

    result = (Order *)x_calloc( (size_t)1, sizeof(Order) );

    return result;
} /* end of order_alloc() */
/*--------------------*/

Trade *trade_alloc( void ) {
    /* allocate an Trade type and initialize it to zero */
    Trade *result;

    result = (Trade *)x_calloc( (size_t)1, sizeof(Trade) );

    return result;
} /* end of trade_alloc() */
/*--------------------*/

Holdings *holding_alloc( void ) {
    /* allocate an Holdings type and initialize it to zero */
    Holdings *result;

    result = (Holdings *)x_calloc( (size_t)1, sizeof(Holdings) );

    return result;
} /* end of holding_alloc() */
/*--------------------*/

void    order_free( Order *order ) {
  x_free( (char *)order );
}

void    trade_free( Trade *trade ) {
  x_free( (char *)trade );
}

void    holding_free( Holdings *holding ) {
  x_free( (char *)holding );
}
/*--------------------*/

void actfile_name( char *name, char *security, char *actfilename ) {
  /* generates the name of the file where we keep
   *   the history of <name>'s activities--orders, trades, holdings.
   * Its directory is "<security>act".
  */

	strcpy( actfilename, WWWMKTGAMEDIR );
	strcat( actfilename, security );
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
	strcpy( ordbookname, WWWMKTGAMEDIR );
    strcat( ordbookname, security );
    strcat( ordbookname, "act/" );
    strcat( ordbookname, ORDBOOKNAME );
} /* end of ordbookname() */
/*--------------------*/

void prospectus( char *security ) {
  /* print <security>.pro, the prospectus file for security */
  char wholeline[81], filename[100];
  FILE *infile;

	cout << "<pre>\n";

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

	cout << "</pre>\n";
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
  Pair *pairs;

  /* see the random number generator */
  time(&zman);
  srand((unsigned int)zman);

  pairs = (Pair *)x_malloc( sup*sizeof(Pair) );
  result = (int *)x_malloc( sup*sizeof(int) );

  for( i=0; i<sup; i++ ) {
      pairs[i].index = i;
      pairs[i].mikri = rand();
  }

  qsort( pairs, (size_t)sup, sizeof(Pair), paircmp );

  for( i=0; i<sup; i++ )
      result[i] = pairs[i].index;

  x_free( (char *)pairs );
  return result;
} /* end of permute() */
/*-------------------*/

int paircmp( const void *p1, const void *p2 ) {
   /* comparison function for qsort() */
   return ((Pair*)p1)->mikri - ((Pair*)p2)->mikri;
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

  for( i=0; i<(int)strlen( str ); i++ )
      str[i] = tolower( str[i] );
} /* end of strlwr() */
#endif
/*------------------------*/

Security *find_sec( char *sec_name, Security **securities ) {
  /* return the element of **securities that pertains to the security
   * whose name is sec_name.
  */
  int i=0;
  Security *result;

  while( ( i<G::N_sec ) && ( strcmp( sec_name, securities[i]->name ) ) )
    i++;

  if( i==G::N_sec )
     result = NULL;   /* this is the case when security = "endmt" */
  else
      result = securities[i];
  return result;
} /* end of find_sec() */
/*---------------------------*/

int ismoney( Security *sec ) {
   /* Return YES if sec is a money, NO if it's a proper security */
   if( ( sec==NULL ) || ( (sec->forcemkt)[0] == 0 ) )
       return NO;
   else
       return YES;
} /* end of ismoney() */
/*---------------------------*/

char *currency_id( Security *sec ) {
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

double mean( Vect *vect, double trimfactor ) {
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

double variance( Vect *vect, double trimfactor ) {
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

int dblcmp( const void *x, const void *y ) {
    if( *(double*)x > *(double*)y ) return 1;
    if( *(double*)x < *(double*)y ) return -1;

    return 0;
} // dblcmp()
//---------------------

char** get_html_arguments( int n_args ) {
// From stdin, get a string like "name=Ted&pwd=shb" and
// return "Ted" and "shb" in a String[n_args].
//
// I had a helluva time parsing out the last argument without
//		obtaining extra invisible garbage: see the seconddelim 
//		variable.

	const int maxbuflen=2048;
	const int maxtoklen=32;

	char buf[maxbuflen];
	cin.getline( buf, maxbuflen );
//	cout << HTML ;
//	cout << "buf: " << buf << '\n';
//	cout.flush();

	char** result = new (char*)[n_args];
	for( int i=0;i<n_args;i++ )
		result[i] = new char[maxtoklen];
	
	char* tok;
	char seconddelim[5] = "& \n ";
	seconddelim[3] = 13;

	for( i=0;i<n_args;i++ ) {

		if( i==0 )
			tok = strtok( buf, "= \n\t\0" );
		else
			tok = strtok( NULL, "= \n\t\0" );
		if( !tok ) return NULL;

		tok = strtok( NULL, seconddelim );
		if( !tok ) return NULL;

		strncpy( result[i], tok, NAMELEN-1 );
	}

	return result;
} // get_html_arguments()
//-----------------------

void print_file( String filename ) {
// Dump contents of file on stdout.

	ifstream infile( filename );
	if( !infile )
		error( "print_file: Can\'t find infile.\n" );

	char inbuf[81];

	while( !infile.eof() ) {
		
		infile.getline( inbuf, 80 );
		cout << inbuf;

	}
} // print_file()
//--------------------------

String crypt_pwd( const char* plaintxt, char *master_pwd, int serial ) { 
// Encrypt a single word (like, a password).  The scheme is,
//	xor it with a sequence of random bytes, where the random
//	number generator is seeded with a nonreusable int, and
//	the random number generator's output is used only at
//	intervals, which are determined by a master password.
//
// I don't know how to uncrypt, but I don't need to.

	// See random number generator:
	srand( serial );

	//----------------------------------------------
	// xor plaintxt with random numbers.
	char crptxt[ strlen(plaintxt)+1 ];
	crptxt[strlen(plaintxt)] = 0;

	for( int i=0;i<(int)strlen(plaintxt);i++ ) {

		// Run through rand() some.  
		for( int j=0;j<(master_pwd[i]%64);j++ )
			// %64 is to protect master_pwd, in case of successful
			// cracking of this algorithm.
			rand();

		// xor, but be sure result is alphanumerical characters
		crptxt[i] = (plaintxt[i] ^ rand()) % 95 + 33;

	}

	return crptxt;

} // crypt_pwd()

