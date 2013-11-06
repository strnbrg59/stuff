#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqc.hpp"

// FUNCTION DEFINITIONS

void disp_choices1( char* username, char* pwd ) {
	int i;

	cout << "<h1> Main menu</h1>\n";

	cout << 
	" <form method=\"POST\" \
	ACTION=\"http://" << LOCALHOST << CGIBINDIR << "CGImenu1\">";

	// Pass user's name and pwd as hidden variables.
	cout << "<p>"
	"<INPUT TYPE=\"hidden\" NAME=\"username\" VALUE=" <<
	"\"" << username << "\">\n";
	cout << "<p>"
	"<INPUT TYPE=\"hidden\" NAME=\"pwd\" VALUE=" <<
	"\"" << pwd << "\">\n";

	// List the securities by name, as input buttons.
	cout << "<p>\n";
	cout << "<ul>\n";
	for( i=0; i<G::N_sec; i++ )
		cout << "<li><INPUT TYPE=SUBMIT VALUE= \""
			 <<	G::securitydata[i]->menu_fullname << "\" "
			 << " NAME=\"choice\""
			 << ">\n";

	// List other choices, as input buttons.
	cout << 
	"<li><INPUT TYPE=SUBMIT VALUE= " << "\"finger\""
			 << " NAME=\"choice\">\n" ;
	cout << 
	"</ul>\n";

	cout << 
	"</form>\n";

/*
	// Display some environment variables.
	cout 
	<< "<ul>\n"
	<< "<li> SERVER_NAME: " << getenv( "SERVER_NAME" ) << '\n'
	<< "<li>REQUEST_METHOD: " << getenv( "REQUEST_METHOD" ) << '\n'
	<< "<li>HTTP_ACCEPT: " << getenv( "HTTP_ACCEPT" ) << '\n'
	<< "<li>HTTP_USER_AGENT: " << getenv( "HTTP_USER_AGENT" ) << '\n'
	<< "<li>CONTENT_LENGTH: " << getenv( "CONTENT_LENGTH" ) << '\n'
//	<< "<li>everything: "<< system("/users/ted/src/mktgame/www-mktgame-dir/envy") << '\n'
	<< "</ul>\n";
*/
} // disp_choices1() 
//----------------------

void disp_choices2( int sec_num, String username, String pwd ) {
// Pass both sec_num and username as HIDDEN inputs in the form.

//	cout << HTML ;
	cout << "<h1> " <<
		 G::securitydata[sec_num]->name << " market </h1>\n";

	cout << "<FORM METHOD=\"POST\"\
	ACTION=\"http://" << LOCALHOST << CGIBINDIR << "CGImenu2\">";

	// Pass user's name as a hidden variable.
	cout << 
	"<INPUT TYPE=\"hidden\" NAME=\"username\" VALUE=" <<
	"\"" << username << "\">\n" <<
	"<INPUT TYPE=\"hidden\" NAME=\"pwd\" VALUE=" <<
	"\"" << pwd << "\">\n" <<
	"<INPUT TYPE=\"hidden\" NAME=\"sec_num\" VALUE=" <<
	"\"" << sec_num << "\">\n";

	cout << "<p>\n";
	cout << "<ul>\n";
	cout <<
		"<li>" <<
//		 "<img src=\"/~ted/blank.gif\"> " <<
		 "<INPUT TYPE=SUBMIT VALUE= \"b.\"" <<
		" NAME=\"choice\">   check limit order Book\n" <<

		"<li><INPUT TYPE=SUBMIT VALUE= \"o.\"" <<
		" NAME=\"choice\">   place an Order\n" <<

		"<li><INPUT TYPE=SUBMIT VALUE= \"c.\"" <<
		" NAME=\"choice\">   Cancel an order\n" <<

		"<li><INPUT TYPE=SUBMIT VALUE= \"a.\"" <<
		" NAME=\"choice\">   Account activity\n" ;

    if( ismoney( G::securitydata[sec_num] ) == YES )
		cout <<
		"<li><INPUT TYPE=SUBMIT VALUE= \"h.\""
		<<	" NAME=\"choice\">   cash Holdings in this currency\n" ;

	cout <<
		"<li><INPUT TYPE=SUBMIT VALUE= \"t.\"" 
		<<	" NAME=\"choice\">   Ticker\n" ;

    if( strcmp( G::securitydata[sec_num]->underlying, "NULL" ))
		cout <<
		"<li><INPUT TYPE=SUBMIT VALUE= \"e.\"" 
		<<	" NAME=\"choice\">   Exercise options\n" ;

	cout <<
		"<li><INPUT TYPE=SUBMIT VALUE= \"p.\""
		<<  " NAME=\"choice\">   Prospectus\n" ;

	cout << "</FORM>\n";
} // disp_choices2()
//----------------------

int money_only( Security *sec ) {
  /* If sec is the last currency-type security in Securitydata, then
   *   we won't have a market for it; it will serve as money only,
   *   and when selected from the main menu it will just give an
   *   account of holdings in this currency.
   * Returns YES if sec is in fact the last currency-type security.
  */

  int result=NO, i;

  if( ismoney(sec) == NO )
      result = NO;
  else
      for( i=0; i<G::N_sec; i++ ) {
          if( (result==YES) && (ismoney(G::securitydata[i])==YES) )
              result = NO;
          if( sec == G::securitydata[i] )
              result = YES;
      }

  return result;
} /* end of money_only() */
