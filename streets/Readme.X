It works in X, but could use some enhancements.

1. Respond to events, e.g. redraw after an Expose event.
We can do this by putting, in main_streets(), a test like

		XEvent report;
		XNextEvent( GXstreets::display, &report);
		if( report.type == Expose )
			return;

Unfortunately, that also generates a button event, and the result 
is, so far, that the redrawing doesn't happen.  

2. Draw a bigger window, and make the scale of the drawing depend on the 
size of the window.