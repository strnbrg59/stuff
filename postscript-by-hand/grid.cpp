// Plot a grid across a page, so we can better know what coordinates to 
// give things, when we try to type captions ex post facto.

#include <fstream.h>
#include "/users/ted/src/rpnprog/m.hpp"
#include "/users/ted/src/rpnprog/ps_util.hpp"

main() {

	cout << "/smallfont { /TeX-charter findfont 7 scalefont setfont } def smallfont\n";
	cout << "/inch { 72 mul } def\n";
	cout << " 11.5 inch 0.5 inch translate 90 rotate\n";


	// Horizontal lines:
	for( int i=0;i<900;i++ )

		if( i%10 == 0 )	{

			if( i%50 == 0 )
				cout << "0 " << i << " moveto (" << i << ") show\n";
			cout << "0 " << i << " moveto " << "900 " << i << " lineto stroke\n";
		}


	// Vertical lines:
	for( i=0;i<900;i++ )

		if( i%10 == 0 )	{

			if( i%50 == 0 )
				cout << i << " 400" << " moveto (" << i << ") show ";

			cout << i << " 0" << " moveto " << i << " 900 lineto stroke\n";
		}

	cout << "showpage quit\n";

}