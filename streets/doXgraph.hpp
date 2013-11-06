// Special version for streets project.

class Axis_label_array;
class String;

struct GX { // global X stuff (TDS)
	static unsigned int window_width;
	static unsigned int window_height;
	static int screen_num;

	static int CTRL_key_depressed; // for communication with scatterplot_redraw().
		// Set to 1 if CTRL has been depressed.
	static int curr_keycode; // most recent key depressed 
	
	// From here down, it's not really global stuff at all.  Rather, it's just
	//	all sorts of parameters that various graphics functions need, and which
	//	need to be passed down through doXgraph().  Thus, we bundle them in struct
	//	GX, so doXgraph() doesn't have to have millions of arguments.
	int slowdown;
	int drawaxes;
	Axis_label_array* custom_axis_labels;
	String* pointlabels;

}; // struct GX

void doXgraph( void redrawer(/*GX gX,*/ Window,GC, Display*)/*, GX gX*/ );

void load_font( XFontStruct **font_info );

void draw_text(
Window win,
GC gc,
XFontStruct *font_info,
unsigned int win_width, unsigned int win_height);

void getGC( Window win, GC *gc, XFontStruct *font_info);
void TooSmall( Window win, GC gc, XFontStruct *font_info);

// TDS-written functions
void handle_buttonpress( Window win, GC gc, Display* display, 
	XEvent report, char* dispstr, int& old_buttonx, int& old_buttony );
int  handle_keypress   ( Window win, GC gc, Display* display, 
	XEvent report, char* dispstr, int& old_buttonx, int& old_buttony );

#define CTRL_keycode 37
#define ESC_keycode   9
