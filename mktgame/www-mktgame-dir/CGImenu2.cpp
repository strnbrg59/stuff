// Process some of the choices made on second-level 
//	(i.e., security-level) menu--place order, look at account, ticker, etc.
// This does not process all the possible choices, however.  When "finger",
//	or the name of a currency, or "#" are pressed, functions are invoked
//	that never produce the virtual html document of which this is the ACTION.
//
// main() here is the ACTION invoked by the virtual html document
//	that's produced by disp_choices2(), called from CGImenu1.
//	That document sends us three variables: username, and sec_num hidden,
//		and choice (not hidden).

#include <iostream.h>
#include <stdlib.h>
#include "aqc.hpp"
#include "html-util.hpp"

main() {

	cout << HTML;

	//  Get arguments from virtual html form that invoked this.
	Html_argument_array args;
	String username = args["username"];
	String pwd 		= args["pwd"];
	int sec_num 	= atoi(args["sec_num"]);
	String choice 	= args["choice"];

	// Verify user name & password
	verify_user( username, pwd );

    char security[NAMELEN+1];
    Security *sec;

    strcpy( security, G::securitydata[sec_num]->name );
    sec = G::securitydata[sec_num];

	switch( choice.chars()[0] ) 
	{
            case 'b' : disp_orders( security ); break;
	        case 'o' : {
                   build_order( (char*)username.chars(), pwd, security );
                   break;
                   }
            case 'c' : cancel_order( (char *)username.chars(), 
				pwd, security ); break;
            case 'a' : get_act_ts( (char*)username.chars(), security ); break;
            case 'h' : if( ismoney(sec)==YES ) 
				get_money_ts( (char *)username.chars(), security ); break;
    	    case 't' : disp_diary( security ); break;
            case 'e' : {
                if( strcmp(sec->underlying,"NULL" ) )
                        exercise_option(
                            (char *)username.chars(), pwd,
							sec->name, sec->underlying,
                            sec->strikeprice, sec->call_or_put[0]);
                break;
                       }
            case 'p' : prospectus( security ); break;
	}        

} // main()
//----------------------
