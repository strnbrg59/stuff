/* utilities for canceling orders from the limit book */

/**** INCLUDES ****/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmem.h"
#include "aqc.h"

/**** GLOBAL VARIABLE ****/
extern NAMES *Names;
extern int N_players;

/**** DEFINITIONS ****/
#define DELALL 0  /* to delete all of the user's live orders */
#define QUIT -1  /* if user decides to quit without canceling anything */

/**** LOCAL FUNCTION DECLARATIONS ****/

static int get_delnum( int i );
static int list_orders( FILE *outfile, char *name, char *security,
    ORDER *best_limbuy, ORDER *best_limsell );
static int remove_order( char *name, char *security,
    ORDER **best_limbuy, ORDER **best_limsell, int delnum,
    HOLDINGS *holding );


/**** FUNCTIONS VISIBLE EXTERNALLY...
int cancel_order( char *name, char *security,
                  ORDER **best_limbuy, ORDER **best_limsell,
                  HOLDINGS **holdings );
void remove_all_orders( char *security );
void splice_out( ORDER **best_lim, ORDER *delorder );
****/

/**** FUNCTION DEFINITIONS ****/
int cancel_order( char *name, char *security ) {
  /* Remove one or more orders belonging to "name".  Present a list of
   *   current live orders, and prompt user for the one(s) to remove.
   * Return 0 if successful, -1 if user entered invalid input or function
   *   failed for another reason.
  */
  int result=0,
      handle,  /* of socket */
      i=0,     /* counts orders placed by "name" */
      delnum;  /* serial # of order to delete.  If DELALL, delete them all.
                *   if QUIT, quit without deleting any orders. */
  ORDER *best_limbuy, *best_limsell;
  HOLDINGS *holding;

  /* display a list of all the player's orders */
  load_ordbook( &best_limbuy, &best_limsell, security );
  i = list_orders( stdout, name, security, best_limbuy, best_limsell );
  free_ordbook( best_limbuy, best_limsell );

  if( i == 0 ) {
      printf( "You have no live orders.\n" );
      result = -1;
  }
  else {
      delnum = get_delnum( i );  /* i is number of live orders by "name" */
      if( delnum == QUIT )
          result = -1;

      /* Proceed to delete the delnum-th order.  There's an outside
       * possibility that in the time between when the user typed the delnum
       * and the time the server gets nabbed (a few lines of code below here)
       * someone else would have removed the delnum-th order by crossing
       * with it.  In that case, the wrong order will get cancelled (i.e.
       * the one after the delnum-th), or if the delnum-th order is the last
       * one on the list, no order will get cancelled, because remove_order()
       * stops when it gets to the bottom of the order book.
       * So the wrong order will get removed, but at least we won't be
       * freeing a NULL pointer!  It shouldn't happen very often, in
       * any case.
      */
      else {

          /* Ready to remove orders: first get server's attention */
    	  handle = server_avail();
	      if( handle < 0 ) {
		  update_connect_time( YES );
	          error( "Server unavailable." );
	      }


    	  /* if we get to here, the server is going to get to us, either right
	       * away, or very soon */
    	  server_ready( handle );
	      /* this returns when the server is really ready */

	      /* load authoritative version of ordbook and holdings from disk */
    	  load_ordbook( &best_limbuy, &best_limsell, security );
	      holding = load_holding( name, security );

    	  /* now we're ready to remove the order(s).  Be sure to dump
	       *   the order book, and close the socket */
          remove_order( name, security,
                        &best_limbuy, &best_limsell, delnum, holding );
	      printf( "Done.\n" );

    	  /* dump and free the ordbook, and release the server */
	      dump_ordbook( best_limbuy, best_limsell, security );
          free_ordbook( best_limbuy, best_limsell );
          holding_free( holding );
    	  server_goodbye( handle );

      } /* else--user would indeed like to delete one or more orders */

  }  /* else--there are some live orders */

  return result;
} /* end of cancel_order() */
/*---------------------------------*/

static int get_delnum( int i ) {
  /* prompt user for serial number of ORDER he wants to delete.
   * Valid responses are in {1,...,i}.  In addition, a response of
   *   '*' causes all his orders to be deleted (and DELALL to be
   *   returned), while 'q' implies "quit; don't delete anything."
  */
  char wholeline[120], *cmd;
  int cmdnum, result, j, isint;

  printf( "Enter serial number of order you want to cancel,\n" );
  printf( "  or * to cancel them all,\n");
  printf( "  or q to quit\n" );
  fflush( stdin );

  scanf( "%s",  wholeline );  /* gets(wholeline) causes segmentation error!*/
  cmd = strtok( wholeline, " " );

  /* check if it's a valid number */
  isint = 1;
    for( j=0; j<string_length(cmd); j++ )
      isint *= isdigit(cmd[j]);
  if( isint ) {
      cmdnum = atoi( cmd );
      if( (cmdnum>0) && (cmdnum<=i) )
          result = cmdnum;
      else {
          printf("Invalid number: must be between 1 and %d.\n", i );
          result = QUIT;
      }
  }

  else {  /* user has not typed a number: check for '*' or 'q' */
      if( cmd[0] == '*' )
          result = DELALL;
      else {
          if( cmd[0] == 'q' )
              result = QUIT;
          else {
              printf("Invalid entry.\n");
              result = QUIT;
          }
      }
  }

  return result;
} /* end of get_delnum() */
/*---------------------------------*/

static int list_orders( FILE *outfile, char *name, char *security,
    ORDER *best_limbuy, ORDER *best_limsell ) {
    /* display all of "name"'s live orders, next to serial numbers to
     *   which he can refer to order their deletion.
     * Return the number of live orders belonging to "name" */

    int i=0;
    ORDER *currorder;

    printf("\nHere are your live orders..." );
    printf("\nserial number             description\n" );

    currorder = best_limbuy;
    while( currorder != NULL ) {

        if( !strcmp( currorder->name, name ) ) {
            printf( "   %d             ", ++i );
            disp_order( currorder, outfile, security );
        }
        currorder = currorder->next;
    }

    currorder = best_limsell;
    while( currorder != NULL ) {
        if( !strcmp( currorder->name, name ) ) {
            printf( "   %d             ", ++i );
            disp_order( currorder, outfile, security );
        }
        currorder = currorder->next;
    }

    return i;
} /* end of list_orders() */
/*---------------------------------*/

static int remove_order( char *name, char *security,
    ORDER **best_limbuy, ORDER **best_limsell, int delnum, HOLDINGS *holding ) {
    /* Splice out "name"'s delnum-th order from the order book, looking
     *   first through the limbuy's and then through the limsell's.
     * x_free() the order spliced out.
     * Delete all if delnum=DELALL.
     * Calling routine deals with delnum=QUIT.
    */

    int i=0, /* counts orders by the same person n*/
	        result=0;
    ORDER *currorder,
          *delorder;  /* the order to delete */

    /* go through the limit buys */
    currorder = *best_limbuy;
    while( (currorder!=NULL)   &&   ( (i<delnum) || (delnum==DELALL) ) ) {

        if( !strcmp( name, currorder->name ) ) {
            i++;
            if( ( i == delnum ) || ( delnum == DELALL ) ) {
                delorder = currorder;
                currorder = currorder->next;

		        /* time-stamp and record in the .act file */
        		delorder->comment = 'C';
                time( &(delorder->zman) );
                time( &(holding->zman) );
                dump_ord( delorder, holding, security );

                splice_out( best_limbuy, delorder );
            }
            else
                currorder = currorder->next;
        }
        else
            currorder = currorder->next;
    }

    /* same thing, limit sells */
    currorder = *best_limsell;
    while( (currorder!=NULL)   &&   ( (i<delnum) || (delnum==DELALL) ) ) {

        if( !strcmp( name, currorder->name ) ) {
            i++;
            if( ( i == delnum ) || ( delnum == DELALL ) ) {
                delorder = currorder;
                currorder = currorder->next;

    		    /* time-stamp and record in the .act file */
        		delorder->comment = 'C';
                time( &(delorder->zman) );
                time( &(holding->zman) );
                dump_ord( delorder, holding, security );

                splice_out( best_limsell, delorder );
            }
            else
                currorder = currorder->next;
        }
        else
            currorder = currorder->next;
    }

    if( i==0 )
        result = -1;
    return result;
} /* end of remove_order() */
/*---------------------------------*/

void splice_out( ORDER **best_lim, ORDER *delorder ) {
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
} /* end of splice_out() */
/*---------------------------------*/

void remove_all_orders( char *security ) {
  /* Clear the entire order book for security, recording the deed in
   *   every player's .act file.
   * Called from delordbk.prj.
  */
  int i;
  ORDER *best_limbuy, *best_limsell;
  HOLDINGS *holding;

  load_ordbook( &best_limbuy, &best_limsell, security );

  for ( i=0; i<N_players; i++ ) {

      holding = load_holding( Names[i].name, security );

      remove_order( Names[i].name, security,
                    &best_limbuy, &best_limsell, DELALL, holding );
      holding_free( holding );
  }

  /* dump and free the ordbook, and release the server */
  dump_ordbook( best_limbuy, best_limsell, security );
  free_ordbook( best_limbuy, best_limsell );

} /* end of remove_all_orders() */
/*---------------------------------*/
/*---------------------------------*/
