// Handle choices made in the top-level menu.  
// main() here is the ACTION invoked by the virtual html document
//		that's produced by CGInamepwd.
//		That document sends us two variables:
//		username, and choice.

#include <String.h>
#include "aqc.hpp"
#include "html-util.hpp"

main() {

	//  Get arguments from virtual html form that invoked this
	//	program.  Arguments are username, pwd, and choice (the 
	//  choice made from the menu--the security chosen, 
	//	or "finger").

	cout << HTML ;

	Html_argument_array args;
	String username = args["username"];
	String pwd 		= args["pwd"];
	String choice 	= args["choice"];


	// Verify user & password
	verify_user( username, pwd );

	int sec_num;
	if( -1 != (sec_num = is_sec_choice( args["choice"] ) ) )
	// sec_num points to an element of Securitydata, or -1 if
	// cmdstr selects something other than a security.
		if( money_only( G::securitydata[sec_num] ) == YES )
			get_money_ts( (char*)username.chars(), 
				G::securitydata[sec_num]->name );
		else
		    disp_choices2( sec_num, username, pwd );

	else 
	{
		if( args["choice"] == "finger" )
			finger();
		if( args["choice"] == "#" )
			holding_list( (char*)username.chars() );
	}

} // main()
//-------------------------------

