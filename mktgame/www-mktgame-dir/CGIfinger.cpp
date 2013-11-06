// Finger function: find out about some trader's standing.
//
// main() here is the ACTION invoked by the virtual html document
//	that's produced by the finger() function in holdings.cpp, which
//	is called by CGImenu2.
//	That document sends us one variable--the name of the trader you
//		want to finger.

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include "aqc.hpp"
#include "html-util.hpp"

main() {

	cout << HTML << '\n';

	//  Get arguments from virtual html form that invoked this.
	Html_argument_array args;
	String name = args["name"];

	cout << "<pre>\n";

	printf( "\n" );

	printf( "*** all holdings -- %s ***\n", name.chars() );
	printf( "  security        holdings\n" );

	// Get security holdings and display.
	for( int i=0; i<G::N_sec; i++ ) {
		Holdings* holding = load_holding( (char*)name.chars(), 
			G::securitydata[i]->name );
		if( holding==NULL ) {
			printf("No one here by that name.\n");
			exit(0);
		}
		printf( "    %s      %7.0f\n", 
		G::securitydata[i]->name, (float)(holding->amount) );
		holding_free( holding );
	}

	// Indicate market value 
	double mktval = value_portf( (char*)name.chars() );
	if( mktval != 0 ) /* 0 is the error condition for value_portf() */
		printf( "Market value of holdings = %8.3f\n\n", 
			value_portf( (char*)name.chars() ));

	cout << "</pre>\n";

} // main()


