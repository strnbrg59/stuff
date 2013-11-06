#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>

#include <stdio.h>
#include <string.h>

//#include <m.hpp>
//#include <optr.hpp>
//#include <misc_util.hpp>

#include "happyface.bitmap"
#include "doXgraph.hpp"
//#include "redrawers.hpp"

#define BITMAPDEPTH 1
#define TOO_SMALL 0
#define BIG_ENOUGH 1

/* These are used as arguments to nearly every Xlib routine, so it saves 
 * routine arguments to declare them global.  If there were 
 * additional source files, they would be declared extern there. */
Display *display;
//int screen_num;

void doXgraph( void graphics_engine(/*GX,*/Window,GC,Display* )/*, GX gx*/ )
{
	Window win;
	int x, y; 	// window position
	unsigned int border_width = 4;	// four pixels 
	unsigned int display_width, display_height;
//	unsigned int icon_width, icon_height;
	char *window_name = "win";
	char *icon_name = "win";
	Pixmap icon_pixmap;
	XSizeHints *size_hints;
	XIconSize *size_list;
	XWMHints *wm_hints;
	XClassHint *class_hints;
	XTextProperty windowName, iconName;
	int count;
	XEvent report;
	GC gc;
	XFontStruct *font_info;
	char *display_name = NULL;
	int window_size = BIG_ENOUGH;	// or TOO_SMALL to display contents 

	// TDS declarations
	char dispstr[100];
	dispstr[0]=0;
	//--- end TDS declarations ---  

	if (!(size_hints = XAllocSizeHints())) {
		fprintf(stderr, "Failure allocating memory\n");
        exit(0);
    }
	if (!(wm_hints = XAllocWMHints())) {
		fprintf(stderr, "Failure allocating memory\n");
        exit(0);
    }
	if (!(class_hints = XAllocClassHint())) {
		fprintf(stderr, "Failure allocating memory\n" );
        exit(0);
    }

	// connect to X server
	if ( (display=XOpenDisplay(display_name)) == NULL )
	{
		(void) fprintf( stderr, "Cannot connect to X server %s\n", 
				XDisplayName(display_name));
		exit( -1 );
	}

	// get screen size from display structure macro 
	GX::screen_num = DefaultScreen(display);
	display_width = DisplayWidth(display, GX::screen_num);
	display_height = DisplayHeight(display, GX::screen_num);

	// Note that in a real application, x and y would default to 0
	// but would be settable from the command line or resource database.  
	//
	x = y = 0;

	// create opaque window 
	win = XCreateSimpleWindow(display, RootWindow(display,GX::screen_num), 
			x, y, GX::window_width, GX::window_height, border_width, BlackPixel(display,
			GX::screen_num), WhitePixel(display,GX::screen_num));

	// Get available icon sizes from Window manager 

	if (XGetIconSizes(display, RootWindow(display,GX::screen_num), 
			&size_list, &count) == 0) ;
//		(void) fprintf( stderr, "Window manager didn't set icon sizes - using default.\n");
	else {
		;
		// A real application would search through size_list
		// here to find an acceptable icon size, and then
		// create a pixmap of that size.  This requires
		// that the application have data for several sizes
		// of icons. 
	}

	// Create pixmap of depth 1 (bitmap) for icon.
	icon_pixmap = XCreateBitmapFromData(display, win, (const char*)happyface_bits, 
			happyface_width, happyface_height);

	// Set size hints for window manager.  The window manager may
	// override these settings.  Note that in a real
	// application if size or position were set by the user
	// the flags would be UPosition and USize, and these would
	// override the window manager's preferences for this window. 

	// x, y, width, and height hints are now taken from
	// the actual settings of the window when mapped. Note
	// that PPosition and PSize must be specified anyway. 

	size_hints->flags = PPosition | PSize | PMinSize;
	size_hints->min_width = 300;
	size_hints->min_height = 200;

	// These calls store window_name and icon_name into
	// XTextProperty structures and set their other 
	// fields properly. 
	if (XStringListToTextProperty(&window_name, 1, &windowName) == 0) {
		(void) fprintf( stderr, "Structure allocation for windowName failed.\n");
				
		exit(-1);
	}
		
	if (XStringListToTextProperty(&icon_name, 1, &iconName) == 0) {
		(void) fprintf( stderr, "Structure allocation for iconName failed.\n");
		exit(-1);
	}

	wm_hints->initial_state = NormalState;
	wm_hints->input = True;
	wm_hints->icon_pixmap = icon_pixmap;
	wm_hints->flags = StateHint | IconPixmapHint | InputHint;

	class_hints->res_name = " ";
	class_hints->res_class = "win";

	XSetWMProperties(display, win, &windowName, &iconName, 
			NULL, 0, size_hints, wm_hints, 
			class_hints);

	// Select event types wanted 
	XSelectInput(display, win, ExposureMask | KeyPressMask | 
			ButtonPressMask | StructureNotifyMask | PointerMotionHintMask );

	load_font(&font_info);

	// create GC for text and drawing 
	getGC(win, &gc, font_info);

	// Display window 
	XMapWindow(display, win);

	// get events, use first to display text and graphics 
	while (1)  {
		XNextEvent(display, &report);

		switch  (report.type) {
		case Expose:
			// unless this is the last contiguous expose,
			// don't draw the window 
			if (report.xexpose.count != 0)
				break;

			// if window too small to use 
			if (window_size == TOO_SMALL)
				TooSmall(win, gc, font_info);
			else {
				// place text in window 
				// draw_text(win, gc, font_info, GX::window_width, GX::window_height);

				// place graphics in window, 
				graphics_engine( /*gx,*/ win, gc, display );
			}
			break;
		case ConfigureNotify:
			// window has been resized, change width and
			// height to send to draw_text and draw_graphics
			// in next Expose 
			GX::window_width = report.xconfigure.width;
			GX::window_height = report.xconfigure.height;
			if ((GX::window_width < unsigned(size_hints->min_width)) || 
				(GX::window_height < unsigned(size_hints->min_height)))
				window_size = TOO_SMALL;
			else
				window_size = BIG_ENOUGH;
			break;
		default:
			// all events selected by StructureNotifyMask
			// except ConfigureNotify are thrown away here,
			// since nothing is done with them.
			break;
		} // switch 
	} // while

} // doXgraph()

void getGC( Window win, GC *gc, XFontStruct *font_info) {
	unsigned long valuemask = 0; // ignore XGCvalues and use defaults 
	XGCValues values;
	unsigned int line_width = 1;
	int line_style = LineSolid;
	int cap_style = CapRound;
	int join_style = JoinRound;
	int dash_offset = 0;
	static char dash_list[] = {12, 24};
	int list_length = 2;

	// Create default Graphics Context
	*gc = XCreateGC(display, win, valuemask, &values);

	// specify font 
	XSetFont(display, *gc, font_info->fid);

	// specify black foreground since default window background is 
	// white and default foreground is undefined. 
	XSetForeground(display, *gc, BlackPixel(display,GX::screen_num));

	// set line attributes 
	XSetLineAttributes(display, *gc, line_width, line_style, 
			cap_style, join_style);

	// set dashes
	XSetDashes(display, *gc, dash_offset, dash_list, list_length);
}


void load_font( XFontStruct **font_info )
{
	char *fontname = "9x15";

	/* Load font and get font information structure. */
	if ((*font_info = XLoadQueryFont(display,fontname)) == NULL)
	{
		(void) fprintf( stderr, "Cannot open 9x15 font\n" );
		exit( -1 );
	}
}

void draw_text(
Window win,
GC gc,
XFontStruct *font_info,
unsigned int win_width, unsigned int win_height )
{
	char string1[30];
	int len1;
	int width1;
//	char cd_height[50], cd_width[50], cd_depth[50];
	int font_height;
	int initial_y_offset, x_offset;


	sprintf( string1, "winwidth=%d, winheight=%d", win_width, win_height );

	/* need length for both XTextWidth and XDrawString */
	len1 = strlen(string1);

	/* get string widths for centering */
	width1 = XTextWidth(font_info, string1, len1);

	font_height = font_info->ascent + font_info->descent;

	/* output text, centered on each line */
	XDrawString(display, win, gc, (win_width - width1)/2, 
			font_height,
			string1, len1);

	/* To center strings vertically, we place the first string
	 * so that the top of it is two font_heights above the center
	 * of the window.  Since the baseline of the string is what we
	 * need to locate for XDrawString, and the baseline is one
	 * font_info->ascent below the top of the character,
	 * the final offset of the origin up from the center of the 
	 * window is one font_height + one descent. */

	initial_y_offset = win_height/2 - font_height - font_info->descent;
	x_offset = (int) win_width/4;

}


void TooSmall( Window win, GC gc, XFontStruct *font_info)
{
	char *string1 = "Too Small";
	int y_offset, x_offset;

	y_offset = font_info->ascent + 2;
	x_offset = 2;

	/* output text, centered on each line */
	XDrawString(display, win, gc, x_offset, y_offset, string1, 
			strlen(string1));
}
//------------------------

unsigned int GX::window_width  = 600;
unsigned int GX::window_height = 440;
int GX::CTRL_key_depressed = 0;
int GX::curr_keycode = 0;
int GX::screen_num=0;
