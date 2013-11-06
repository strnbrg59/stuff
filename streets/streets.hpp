#ifdef vgagraphics
#	include <vga.h>
#else
#	include "nographics.hpp"
#endif

#include <String.h>

//---- DEFINES ----
#define BLACK 0
#define WHITE 1
#define RED 1		// for b/w monitor
#define GREEN 0		// for b/w monitor
#define CLOSE -1   // for sending the close file command to sumstats
#define max(a,b) ((a) >= (b) ? (a) : (b)) 
#define min(a,b) ((a) <  (b) ? (a) : (b)) 

/*
#ifdef graphicsoff
	struct cwfont {
		cwfont( const char* );
	};
#endif
*/

struct G {
	static int numcars;
	static int maxnodes;
	static int maxconnections;
	static String netdatfile;
	static int lanewidth;
	static int porsche_to_bus_ratio;

	static int bigdist; // used in ctrlvars.c and intercar_dist() to indicate  
						// a distance to frontcar that's so great, icar doesn't
						// need to think about frontcar.                       
	static int stoplightclearance; // how far from a light cars stop.
	static int timeint; // Don't set to other than 1.
	static int usleep;

	static int **edgekey;
}; // struct G

#include <X11/Xlib.h> // for GC, Display, Window structures
struct GXstreets {
// This is structure with gc, display and win as static
// members, and then you won't have to keep passing those
// to all the functions you call, when you use X.
	static GC& gc;
	static Display* display;
	static Window& win;
}; // struct GXstreets
 
struct Coordinates {
	int x;
	int y;
	Coordinates( int, int );
};

//----------- Node and nested structures ---------------
struct Light { 
    int **color; // color[i][j] governs cars coming from connections[i] and 
                 // heading for connections[j]. 
                 
    int   interval;  // change color every interval iterations, starting... 
    int   lag;       // at the lag-th iteration.                            
};

struct Endcar { 
      int to; 
      int from; 
}; // idnum's of car on this connection, nearest to our node.

struct Connection { 
    int idnum; // id number of connected node 
    double distance; 
    double dirx, 
          diry; // direction to connected node 
	Endcar endcar;
    int ncars; // number of cars on this connection (used for statistics) 
};

class Node { 
  public:
	int idnum;  // node id number 
	int x, y;   // its coordinates in the graphics plane 
	double legalspeed; 
	Light light;
	int numconnections; 
	Connection* connections;

	Node();
	~Node();

	// Light timing rules:
	void rotate_green();
	void random_green();
	void all_green();
	void green_on_demand();

	int light_time( int clocktick ) const;
	void display_lights() const;

}; // class Node

class Node_array {
	Node* rep;
	int n;
	void loadnetwork();  // called by constructor
	void set_distndir(); // called by constructor
	void draw_streets() const; // called by constructor
	void initlights();	 // called by constructor

  public:
	Node_array();
	~Node_array();
	Node& operator[]( int );
	Node& operator[]( int ) const;

	int num() const;

	void light_state( int clocktick );
	int cars_on_lane( int from, int to ) const;
	double car_density( int from, int to ) const;
}; // class Node_array
//---------------- end Node and nested structures ---------------

//---------------- Car and nested structures --------------------

class Car { 
  public:

	//------- State variables ----------
	double posx,posy; 
	// Position in the vga graphics coordinates. We're 
	// using double instead of int to avoid rounding error which sets the 
	// car going off course .
	double 	dirx,diry,  // direction 
			speed, accel; 
	int nodefrom, 
		nodeto,   // idnum's of nodes 
		nextnodeto, // nodeto after passing through nodeto 
		frontcar,  // idnum of car in front, in same lane
		backcar;   // idnum of car in back, in same lane 
	double disttonextnode; // has to stay below 2**15, for sort_list() to work
	int approachingstop; 
	//----------------------------------

	void newstate( const Node_array& );
	virtual String mytype() const = 0;

	virtual Coordinates display( Coordinates ) const = 0;

	double approach_stop();
	int decide_turn( const Node_array& );
	int can_stop_now( const Node_array& ) const;

	virtual double maxspeed() const = 0;
	virtual double maxaccel() const = 0;
	virtual double minaccel() const = 0;
	virtual double gentle_brake() const = 0;
	virtual double annctime() const = 0;

	Car();

}; // Class Car
//----------------------

class Porsche : public Car {
  public:

	// Performance characteristics
	static double maxspeed_;
	static double maxaccel_;
	static double minaccel_; // braking
	static double gentle_brake_;
	static double annctime_; // minimum amount of lead-time a car must give before 
							 // changing turn signal.                              
	String mytype() const;

	// We'd make the following bunch of functions static, except they
	// are virtual in class Car and there's no such thing as a virtual
	// static function.
	double maxspeed() const { return maxspeed_; };
	double maxaccel() const { return maxaccel_; };
	double minaccel() const { return minaccel_; };
	double gentle_brake() const { return gentle_brake_; };
	double annctime() const { return annctime_; };

	Coordinates display( Coordinates ) const;

//	Porsche();

}; // class Porsche
//-----------------------

class Bus : public Car {
  public:
	// Performance characteristics
	static double maxspeed_;
	static double maxaccel_;
	static double minaccel_; // braking
	static double gentle_brake_;
	static double annctime_;

	String mytype() const;

	double maxspeed() const { return maxspeed_; };
	double maxaccel() const { return maxaccel_; };
	double minaccel() const { return minaccel_; };
	double gentle_brake() const { return gentle_brake_; };
	double annctime() const { return annctime_; };

	Coordinates display( Coordinates ) const;

//	Bus();
}; // class Bus
//-----------------------

class Car_array {
	Car** rep;	// ptr to array of ptrs, so we can use derived classes.
	int n;
  public:
	Car_array( int );
	~Car_array();
	Car& operator[]( int );
	Car& operator[]( int ) const;
	int num() const;
	
	void display() const;
}; // class Car_array
//------------------------------------


 
//---- FUNCTION DECLARATIONS -----
// in statevar.c 
void start_cars( Node_array& nodes, Car_array& ); 
void statevars( Node_array& nodes, Car_array& ); 
 
// in ctrlvar.c 
void ctrlvars( Node_array& nodes, Car_array& ); 
 
// in util.c 
int **imatrix(int,int,int,int); 
void grafinit(void); 
void manual_control( char cmd ); 
int signum(double); 
void mathstats(void); 
double round(double x, double y); 
void sumstats( const Node_array&, const Car_array&, int iter );  
void putpixel( int x, int y, int color );
void putbigpixel( int x, int y, int color );
