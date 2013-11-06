/* utilities for canceling orders from the limit book */

/**** INCLUDES ****/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmem.hpp"
#include "aqc.hpp"

/**** DEFINITIONS ****/
#define QUIT -1   // If user decides to quit without canceling anything 
#define DELALL -1 // For deleting all of a player's orders.

/**** LOCAL FUNCTION DECLARATIONS ****/

static int list_orders( FILE *outfile, char *name, char *security,
    Order *best_limbuy, Order *best_limsell );

/**** FUNCTIONS VISIBLE EXTERNALLY...
int cancel_order( char *name, char *security,
                  Order **best_limbuy, Order **best_limsell,
                  Holdings **holdings );
void remove_all_orders( char *security );
void splice_out( Order **best_lim, Order *delorder );
****/

/**** FUNCTION DEFINITIONS ****/
int cancel_order( char *username, String pwd, char *security ) {
  /* Remove one or more orders belonging to "name".  Present a list of
   *   current live orders, and prompt user for the one(s) to remove.
   * Return 0 if successful, -1 if user entered invalid input or function
   *   failed for another reason.
  */

	// We're going to see if this function gets called when we go back
	// to the html virtual document it produces.
//	time_t zman;
//	time(&zman);
//	cout << "cancel_order(): time is " << zman << '\n';

	//----------------------------------------------
	// Pass name, pwd, and security as hidden variables.
	cout << "<FORM METHOD=\"POST\"" <<
		" ACTION=\"http://" << LOCALHOST << CGIBINDIR <<
		"CGIcancel\">\n" <<
	"<INPUT TYPE=\"hidden\" NAME=\"username\" VALUE=" <<
	"\"" << username << "\">\n" <<
	"<INPUT TYPE=\"hidden\" NAME=\"pwd\" VALUE=" <<
	"\"" << pwd << "\">\n";

	cout << "<p>"
	"<INPUT TYPE=\"hidden\" NAME=\"security\" VALUE=" <<
	"\"" << security << "\">\n";
	//-----------------------------------------------

	int	i=0,     // counts orders placed by "name" 
		result;

	Order *best_limbuy, *best_limsell;

	// display a list of all the player's orders 
	load_ordbook( &best_limbuy, &best_limsell, security );
	i = list_orders( stdout, username, security, best_limbuy, best_limsell );
	free_ordbook( best_limbuy, best_limsell );

	if( i == 0 ) {
		printf( "You have no orders on the book.\n" );
		result = -1;
	}

	cout << "<hr>\n"
		 << "<INPUT TYPE=SUBMIT VALUE=\"Submit\"><p>"
		 << "</FORM>\n";

  return result;
} // cancel_order()
//---------------------------------

static int list_orders( FILE *outfile, char *name, char *security,
    Order *best_limbuy, Order *best_limsell ) {
    /* display all of "name"'s live orders, next to serial numbers to
     *   which he can refer to order their deletion.
     * Return the number of live orders belonging to "name" */

    int i=0;
    Order *currorder;

    printf("\nHere are your outstanding orders...<p>\n" );

    currorder = best_limbuy;
    while( currorder != NULL ) {

        if( !strcmp( currorder->name, name ) ) {
			cout << " <INPUT TYPE=\"checkbox\" 
				NAME=\"" <<  i++ << "\">\n";
            disp_order( currorder, outfile, security );
			cout << "<BR>\n";
        }
        currorder = currorder->next;
    }

    currorder = best_limsell;
    while( currorder != NULL ) {
        if( !strcmp( currorder->name, name ) ) {
			cout << " <INPUT TYPE=\"checkbox\" 
				NAME=\"" <<  i++ << "\">\n";
            disp_order( currorder, outfile, security );
			cout << "<BR>\n";
        }
        currorder = currorder->next;
    }

    return i;
} // list_orders() 
//---------------------------------

void mark_for_deletion( char *name, 
    Order **best_limbuy, Order **best_limsell, int delnum ) {

// Put a 'C' in the comment field of all the orders that the user
//	has requested be deleted.

	int i=0;

	// Go through the limit buys.
    Order* currorder = *best_limbuy;
    while( (currorder!=NULL)   &&   ( (i<=delnum) || (delnum==DELALL) ) ) {

        if( !strcmp( name, currorder->name ) ) {

            if( ( i == delnum ) || ( delnum == DELALL ) ) 
        		currorder->comment = 'C';

            i++;
        }

		currorder = currorder->next;
    }

    // Same thing, limit sells.
    currorder = *best_limsell;
    while( (currorder!=NULL)   &&   ( (i<=delnum) || (delnum==DELALL) ) ) {

        if( !strcmp( name, currorder->name ) ) {

            if( ( i == delnum ) || ( delnum == DELALL ) ) 
        		currorder->comment = 'C';

            i++;
        }

		currorder = currorder->next;
    }

} // mark_for_deletion()
//--------------------------------

void remove_order( char *name, char *security,
    Order **best_limbuy, Order **best_limsell, Holdings *holding ) {
    // Splice out the orders that have had their comment fields set
	// 	to 'C'.  CGIcancel.cpp must see to setting the 'C's.
	// Go first through the limbuy's and then through the limsell's.
	// x_free() the order spliced out.

    Order *currorder, *delorder;

    // Go through the limit buys
    currorder = *best_limbuy;
    while( currorder ) {

		if( ( currorder->comment == 'C' )  
		&&  ( !strcmp( name, currorder->name ) ) ) {

			delorder = currorder;
			currorder = currorder->next;
	
			time( &(delorder->zman) );
			time( &(holding->zman) );
			dump_ord( delorder, holding, security );

			splice_out( best_limbuy, delorder );
		}
		else 
			currorder = currorder->next;
	}
		
    // Same procedure, limit sells
    currorder = *best_limsell;
    while( currorder ) {

		if( ( currorder->comment == 'C' ) 
		&&  ( !strcmp( name, currorder->name ))) {

			delorder = currorder;
			currorder = currorder->next;

			time( &(delorder->zman) );
			time( &(holding->zman) );
			dump_ord( delorder, holding, security );

			splice_out( best_limsell, delorder );
		}
		else
			currorder = currorder->next;
	}

} // remove_order() 
//---------------------------------

void splice_out( Order **best_lim, Order *delorder ) {
  /* reset links to exclude delorder from the order book,
   * and x_free's delorder. */

  if( delorder->prev == NULL ) {   /* delorder = best_lim: reset it */
      *best_lim = delorder->next;
      if( *best_lim != NULL )
          (*best_lim)->prev = NULL;
  }

  else {
      if( delorder->next != NULL )
          (delorder->next)->prev = delorder->prev;
      (delorder->prev)->next = delorder->next;
  }

  x_free( (char *)delorder );
} // splice_out() 
//---------------------------------

void remove_all_orders( char *security ) {
// Clear the entire order book for security, recording the deed in
//	every player's .act file.

	int i;
	Order *best_limbuy, *best_limsell;
	Holdings *holding;

	load_ordbook( &best_limbuy, &best_limsell, security );

	for ( i=0; i<G::N_players; i++ ) {

		holding = load_holding( G::names[i].name, security );

		mark_for_deletion( G::names[i].name, 
			&best_limbuy, &best_limsell, DELALL );
		remove_order( G::names[i].name, security,
			&best_limbuy, &best_limsell, holding );
		holding_free( holding );
	}

  /* dump and free the ordbook, and release the server */
  dump_ordbook( best_limbuy, best_limsell, security );
  free_ordbook( best_limbuy, best_limsell );

} /* end of remove_all_orders() */
/*---------------------------------*/
/*---------------------------------*/
