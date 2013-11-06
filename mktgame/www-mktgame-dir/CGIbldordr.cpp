// Interpret the virtual html form that the trader uses to submit his
// order.  Thus, main() here is the ACTION that gets called by the
// virtual html form that's produced by build_order() in bldordr.cpp.

// INCLUDES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <String.h>
#include "aqc.hpp"
#include "html-util.hpp"

// LOCAL FUNCTION DECLARATIONS 
static Order *interpret_order( 
	String username, 
	String mkt_or_limit,
	String buy_or_sell,
	String units,
	String price );


// FUNCTION DEFINITIONS
main() {

	cout << HTML << '\n' << "<pre>\n";

	// Get arguments from virtual html form that invoked this.
	// Arguments are: username, pwd, security, mkt_or_limit,
	//		buy_or_sell, units, and price.
	Html_argument_array args;
	String username = args["username"];
	String pwd		= args["pwd"];
	String security = args["security"];
	String mkt_or_limit = args["mkt_or_limit"];
	String buy_or_sell = args["buy_or_sell"];
	String units = args["units"];
	String price = args["price"];

/*
	cout << "username= " << username << '\n'
		<< "pwd= " << pwd << '\n'
		<< "security= " << security << '\n'
		<< "mkt_or_limit= " << mkt_or_limit << '\n'
		<< "buy_or_sell= " << buy_or_sell << '\n'
		<< "units= " << units << '\n'
		<< "price= " << price << '\n';
*/
	// Verify user name & password
	verify_user( username, pwd );
//	cout << "CGIbldordr.cpp: verify_user() passed.<BR>\n";

	Order *order = interpret_order( username, 
		mkt_or_limit, buy_or_sell, units, price );
//	cout << "CGIbldordr.cpp: interpret_order<BR>finds units="
//		 << units << ", price=" << price << "<P>\n";

    if( order != NULL ) {

        if( front_of_firm( order, (char*)security.chars() ) == YES ) {
            printf( "You may not place an order in front of the firm's.\n" );
            return 0;
        }

//      disp_order( order, stdout, security );
        int ok = verify_order( order, (char*)security.chars() );

		if( ok == -1 )   // verify_order() has free'd order 
			return 0;
		else
			time( &(order->zman) );  // time stamp the order 

		handle_order( order, (char *)security.chars(), YES );
	}

	cout << "</pre>\n";

    disp_orders( (char*)security.chars() ); 

} // main()
//--------------------------

static Order *interpret_order( 
	String username, 
	String mkt_or_limit,
	String buy_or_sell,
	String units,
	String price ) 
{

	double  dbl_units = atof( units );
    dbl_units = round( dbl_units, 1.0 );

    /* get price */
	double dbl_price;
	if( mkt_or_limit == "mkt" )
		dbl_price = 0;
	else {
		dbl_price = atof(price);
	}

	double ticksize;
    if( dbl_price != 0 ) {  /* set tick size */
		ticksize = 0.10;
		if( fabs(dbl_price) < 5 ) ticksize = 0.05;
		if( fabs(dbl_price) < 2 ) ticksize = 0.01;
		dbl_price = round( dbl_price, ticksize );
	}

	Order *result;
	result = order_alloc();

	strncpy( result->name, username.chars(), NAMELEN );
	
	if( mkt_or_limit == "mkt" )
		result->mkt_or_lim = MKT;
	else
		result->mkt_or_lim = LIM;

	if( buy_or_sell == "buy" )
		result->buy_or_sell = BUY;
	else
		result->buy_or_sell = SELL;

	result->comment = ' ';

	result->amount = dbl_units;
	result->price = dbl_price;

	time( &(result->zman) );

	return result;

} // interpret_order()
//--------------------------

//-----------------------------
