// main() for the streets project.  The code here creates an X window
// and, from within a loop that takes care of handling window events
// (redraws, moves, etc), calls what was (before we wrapped it all up
// in this X business) the old main() of the streets project.

#include <fstream.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>

#include <m.hpp>
#include <optr.hpp>
#include <rpngraphics.hpp>
#include <misc_util.hpp>

#include "/users/ted/src/rpnprog/libX/doXgraph.hpp"
//#include "redrawers.hpp"
#include "streets.hpp"

void dummy_redrawer( const M& x, const M& y, GX gX,
	Window win, GC gc, Display *display );

GC& GXstreets::gc = (GC *)NULL;
Display* GXstreets::display = (Display *)NULL;
Window& GXstreets::win = (Window *)NULL;

void main_streets();

main() {

	// Assign to global variables--members of GX.
	GX gX;
	doXgraph( dummy_redrawer, M(1),M(1), gX );

} // main()
//---------------------

void dummy_redrawer( const M& x, const M& y, GX gX,
	Window win, GC gc, Display *display )
{

	GXstreets::gc = gc;
	GXstreets::display = display;
	GXstreets::win = win;

/*	You can execute Xlib functions from this environment.  Here's an
	example, and note the Display, Window and GC are part of the
	global structure GXstreets, so you don't have to pass them to
	all your functions.
	for( int i=0;i<100;i++ ) 
		XDrawLine( GXstreets::display,GXstreets::win,GXstreets::gc,
			random()%640,random()%480,random()%640,random()%480);
*/

	main_streets();

} // dummy_redrawer() 
//---------------------------




