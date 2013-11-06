#include <fstream.h>
#include <math.h>
#include <stdlib.h>

void coutdefs();		
void movetopoint(double x, double y);
void plotpoint( double x, double y );

main() {
	coutdefs();

	// Random walk.
	cout << "newpath\n";
	double walk=0;
	for( int i=0;i<=60;i++ ) {
		double randy = 10*((rand()%100)/99.0 - .5);
		walk += randy;
		if( i==0 ) movetopoint(i,walk);
		plotpoint(i,walk);
	}

	cout<< "stroke\n" 
		<< "showpage\n";

} // main()
//------------------

void plotpoint( double x, double y ) {
	cout << "gsave newpath " << x << " " << y << " " << "moveto 1 box grestore\n";
	cout << x << " " << y << " " << "lineto\n";
}
//------------------

void movetopoint(double x, double y) {
	cout << x << " " << y << " moveto\n";
}
//------------------

void coutdefs() {
// Handy Postscript definitions.

cout 
	<< "/inch { 72 mul } def\n"
	<< "/box{\n"		// stack: boxsize
	<< "	/boxsize exch def\n"
	<< "	.5 setgray\n"
	<< "	boxsize 2 div -1 mul boxsize 2 div -1 mul rmoveto\n"
	<< " 	boxsize 0 rlineto\n"
	<< "	0 boxsize rlineto\n"
	<< "	-1 boxsize mul 0 rlineto\n"
	<< "	closepath fill} def\n"

	<< "4 inch 4 inch translate\n"
	<< ".1 .1 setlinewidth\n"
	<< "90 rotate\n"
	<< "15 15 scale\n\n";

} // coutdefs()


