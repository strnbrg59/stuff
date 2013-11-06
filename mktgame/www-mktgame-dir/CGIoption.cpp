#include <iostream.h>
#include <stdlib.h>
#include "xmem.hpp"
#include "aqc.hpp"
#include "html-util.hpp"


main() {

	cout << HTML;

	//  Get arguments from virtual html form that invoked this.
	Html_argument_array args;
	String username = args["username"];
	String pwd 		= args["pwd"];
	String option	= args["option"];
	String underlying = args["underlying"];
	double x		= atof( args["x"] );
	char call_or_put = ((args["call_or_put"]).chars())[0];
	int units		= atoi( args["units"] );

	// Verify user name & password
	verify_user( username, pwd );

	char 	u_ordbookname[ACTNAMELEN+1],  /* order book for underlying */
			o_ordbookname[ACTNAMELEN+1];  /* order book for option */
	int errcode, handle;
	Order *mkt_order, *lim_order;
	Holdings *holdings, *holding;
	Chosen_ones *chosen_ones;
	FILE *infile;  /* just for checking if a file exists */

	/*    0.1. Check for feasibility; is the player long enough options? */
	holdings = load_holdings( G::names, (char *)option.chars() );
	holding  = find_holding( (char*)username.chars(), holdings );
	if( holding->amount < units ) {
		client_warning( "You don't own that many." );
		exit(0);
	}

	/*    1. get ahold of the server, just as for executing a trade */
	if( 0 > (handle = server_avail(username)) ) {
		client_warning( "Server unavailable." );
		exit(0);
	}
	server_ready( handle );  /* this returns when the server is really ready */

	/*    2.1 move order book file for *underlying and *option to a backup file*/
    /* first for underlying */
	infile=fopen("delete.me","r");
	if( infile ) remove( "delete.me" );
	ordbook_name( (char*)underlying.chars(), u_ordbookname );
	errcode = rename( u_ordbookname, "delete.me" );

	/* then for option */
	infile=fopen("delete.me2","r");
	if( infile ) remove( "delete.me2" );
	ordbook_name( (char*)option.chars(), o_ordbookname );
	errcode = rename( o_ordbookname, "delete.me2" );

	/*    2.2 line up enough people with short positions in the option. */
	chosen_ones = choose_shorts( (char*)option.chars(), units );

	/*    2.3 place limit orders for them, on the temporary order books. */
	int i=0;
	while( chosen_ones[i].name[0] != 0 ) {
		/* first for underlying */
		if( call_or_put == 'c' )
			lim_order = create_order( chosen_ones[i].name, LIM, SELL,
				chosen_ones[i].units, x );
		else
			lim_order = create_order( chosen_ones[i].name, LIM, BUY,
				chosen_ones[i].units, x );

		lim_order->comment = 'E';
		handle_order( lim_order, (char*)underlying.chars(), NO );

		/* then for option */
		lim_order = create_order( chosen_ones[i].name, LIM, BUY,
		chosen_ones[i].units, 0.0000000001 );
		lim_order->comment = 'E';
		handle_order( lim_order, (char*)option.chars(), NO );

		i++;
	}

	/*    2.4 place market orders for *name, using handle_order(). */
	/* first for underlying */
	if( call_or_put == 'c' )
		mkt_order = create_order( (char*)username.chars(), MKT, BUY, units, 0 );
	else
		mkt_order = create_order( (char*)username.chars(), MKT, SELL, units, 0 );
	mkt_order->comment = 'E';
	handle_order( mkt_order, (char*)underlying.chars(), NO );

	/* then for option */
	mkt_order = create_order( (char*)username.chars(), MKT, SELL, units, 0 );
	mkt_order->comment = 'E';
	handle_order( mkt_order, (char*)option.chars(), NO );

	/*    2.5 restore true limit order books */
	/* first for underlying */
	remove( u_ordbookname );  /* it exists; there was the dummy order */
	errcode = rename( "delete.me", u_ordbookname );
	/* then for option */
	remove( o_ordbookname );
	errcode = rename( "delete.me2", o_ordbookname );
	errcode ++;  /* just so compiler doesn't say we don't do anything with it */

	/*    3. call forced_sale(); handle_order() won't call it because we're
	 *       calling handle_order with call_server set to NO.
	*/
	forced_sale();

	/*    4. release server. */
	server_goodbye( handle );

	/*    5. free storage */
	x_free( (char *)chosen_ones );

	cout << "Done";

} // main()
//-----------------------



