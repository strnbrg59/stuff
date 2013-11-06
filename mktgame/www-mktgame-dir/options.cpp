/* Option exercise.
*   We can implement most of this stuff with what we have now.  We need
* to create a temporary limit order book, in which we place limit sells on
* behalf of the randomly selected players with short option positions.  These
* limit sells' ask prices are all equal to the option's exercise price.
* Then we place a market buy on behalf of the player exercising his option.
* All that's left is to reset players' holdings of the options themselves.
*      The temporary limit order book should be made to have the same name
* and path as the real order book, so handle_order() doesn't need to do or
* suspect anything out of the ordinary.  So exercise_option() could copy the
* true order book to a backup file, before setting up the temporary order
* book.
*      The nice thing about using a dummy order book and crossing trades
* with it is that all the usual record-keeping gets done in the familiar
* way.
*/

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqc.hpp"
#include "xmem.hpp"

/**** FUNCTIONS VISIBLE EXTERNALLY ****
void exercise_option( char *, char *, char *, double, char call_or_put );
Chosen_ones *choose_shorts( char *option, int units );
****/

/**** FUNCTION DEFINITIONS ****/
void exercise_option(
	char *username,     /* player exercising the option */
	String pwd,			// password
	char *option,       /* name of option security */
	char *underlying,   /* name of underlying security */
	double x,           /* exercise price */
	char call_or_put )   /* set to CALL or PUT */
{

	cout << "<FORM METHOD=\"POST\" " 
		 << "ACTION=\"http://" << LOCALHOST << CGIBINDIR
		 << "CGIoption\">\n"

		 <<	"<INPUT TYPE=\"hidden\" NAME=\"username\" VALUE=" 
		 <<	"\"" << username << "\">\n" 

		 <<	"<INPUT TYPE=\"hidden\" NAME=\"pwd\" VALUE=" 
		 <<	"\"" << pwd << "\">\n" 

		 <<	"<INPUT TYPE=\"hidden\" NAME=\"option\" VALUE=" 
		 <<	"\"" << option << "\">\n" 

		 <<	"<INPUT TYPE=\"hidden\" NAME=\"underlying\" VALUE=" 
		 <<	"\"" << underlying << "\">\n" 

		 <<	"<INPUT TYPE=\"hidden\" NAME=\"x\" VALUE=" 
		 <<	"\"" << x << "\">\n" 

		 <<	"<INPUT TYPE=\"hidden\" NAME=\"call_or_put\" VALUE=" 
		 <<	"\"" << call_or_put << "\">\n" 

		 << "Exercise <INPUT NAME=\"units\"> units.<p>\n"
		 << "<hr>"
		 << "<INPUT TYPE=SUBMIT VALUE=\"Submit\">\n"
		 << "</FORM>\n";
} // exercise_option()
//-------------------------


Chosen_ones *choose_shorts( char *option, int units_needed ) {
  /* find, at random, enough people short the option security for the
   * long party to exercise units units of it against them.
  */
  int *permut, units_found=0,
       i=0, /* indexes holdings[] */
       j=0; /* indexes result[] */
  Chosen_ones *result;
  Holdings *holdings, *holding;

  result = (Chosen_ones *)x_calloc( G::N_players, sizeof(Chosen_ones) );

  /* produce a permutation of {0,...,N_players-1}. */
  permut = permute( G::N_players );

  /* using the order in the permutation, pick players until you have
   * enough short positions to cross with
  */
  holdings = load_holdings( G::names, option );
  while( units_found < units_needed ) {
      holding = holdings + permut[i];

      if( holding->amount < 0 ) {
          strcpy( result[j].name, holding->name );
          result[j].units = MIN( units_needed - units_found, -(int)(holding->amount) );
          units_found -= (int)(holding->amount);
          j++;
      }

      i++;
  }

  x_free( (char *)permut );
  return result;
} /* end of choose_shorts() */
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
/*-----------------------*/
