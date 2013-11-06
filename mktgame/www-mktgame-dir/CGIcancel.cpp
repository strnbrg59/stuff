// Cancel an order.
//
// main() here is the ACTION invoked by the virtual html document
//	that's produced by cancel_order(), called from CGImenu2.
//	That document sends us the following variables:
//		username, pwd, security,
//
// We've been using radio buttons.  Now let's use checkboxes.  The 
//	complications are:
//	1. The number of arguments in the Html_argument_array is not
//		fixed.
//	2. The deletion routine has to go into a loop, deleting all the
//		orders whose checkboxes were checked.
//	We should assign the checkboxes NAMEs 1,2,3,..., equal to the
//		delnum of that order.

#include <iostream.h>
#include <stdlib.h>
#include "aqc.hpp"
#include "html-util.hpp"

main() {

	//  Get arguments from virtual html form that invoked this.
	//  We'll use the Html_argument_array constructor that finds
	//		the number of arguments by itself.  Below are the
	//		first three.  The rest indicate orders you want
	//		to cancel.
	cout << HTML << '\n';

	const int n_args_beforedelnums = 3;
	Html_argument_array args;
//	args.show_stdin();
	String username = args["username"];
	String pwd 		= args["pwd"];
	String security	= args["security"];

	int n_deletions_requested = args.n_args() - n_args_beforedelnums;
	
	// Verify user name & password
	verify_user( username, pwd );

	/* Proceed to delete orders, using their serial numbers as
	 * indicated in the virtual html document.  There's an outside
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

	/* Ready to remove orders: first get server's attention */
	int	handle = server_avail( username );
		if( handle < 0 ) {
//			update_connect_time( YES );
			error( "Server unavailable." );
		}

	/* The server is going to get to us, either right
	* away, or very soon */
	server_ready( handle );
	// This returns when the server is really ready.

	/* load authoritative version of ordbook and holdings from disk */
	Order *best_limbuy, *best_limsell;
	Holdings *holding;
	load_ordbook( &best_limbuy, &best_limsell, (char*)security.chars() );
	holding = load_holding( (char*)username.chars(), (char*)security.chars() );

	// Mark the to-be-deleted orders with a 'C' in their comment
	// 	field.
	for( int i=0;i<n_deletions_requested;i++ ) {

		int delnum = atoi( args.name(n_args_beforedelnums+i) );
//		cout << "delnum=" << delnum << '\n';

		mark_for_deletion((char *)username.chars(), 
			&best_limbuy, &best_limsell, delnum );
	}

	// Now we're ready to remove the order(s).  Be sure to dump
	// 	the order book, and close the socket.
	remove_order( (char *)username.chars(), (char *)security.chars(),
		&best_limbuy, &best_limsell, holding );

	cout << "Done.\n" 
		 << "If you wish to delete other orders, back up at least\n"
		 << "two levels with your browser.\n";

	/* dump and free the ordbook, and release the server */
	dump_ordbook( best_limbuy, best_limsell, (char*)security.chars() );
	free_ordbook( best_limbuy, best_limsell );
	holding_free( holding );
	server_goodbye( handle );

	// Let's see if we can display the order book...
	disp_orders( (char*)security.chars() );
	// Unfortunately, going "back" in the browser will still 
	// take us to that out-of-date cancel form.

} // main()
//----------------------



