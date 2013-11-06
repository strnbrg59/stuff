// Get name and password from stdin and check them for validity.

/**** INCLUDES ****/
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <String.h>
#include "aqc.hpp"
#include "html-util.hpp"

/// FUNCTION DEFINITIONS
main() {

	cout << HTML << '\n';
    int handle;
    Names user;

	// Exclude Lynx browsers.
	if( String(getenv("HTTP_USER_AGENT")).contains("Lynx") ) {
		cout << "For security reasons, this site should not be\
				 used with a Lynx browser.\n";
//		Here's how you'd do a server redirection...
//		cout << "Location: http://localhost/~ted/lynx-warning.html\n\n";
//		...but you'd also have to comment out the "cout << HTML" above,
//		as a server redirection can't coexist with a Content-type header.
		exit(1);
	}

	//  Get userid and pwd from stdin:
	Html_argument_array args;
//	cout << name_pwd["name"] << '\n';
	strncpy( user.name, args["name"].chars(), NAMELEN);
	strncpy(user.pwd, args["pwd"].chars(), NAMELEN);

//	cout << "CGInamepwd* : args[name] = " << args["name"]
//		 << ", args[pwd] = " << args["pwd"] << '\n';

    if( !nameok( &user, G::names, G::N_players ) )
            error( "Bad user name or password." );

    // warn if server is down 
    if( (handle=server_avail( user.name )) < 0 ) 
        cout << "**The server is unavailable; you can look, \n"
			 << "but you can\'t place orders.***\n";
    else {
        server_ready( handle ); // If omit this, I think the server 
								// dies (althought maybe not right away).
        server_goodbye( handle );
    }

	disp_choices1( user.name, user.pwd );

} // main()










