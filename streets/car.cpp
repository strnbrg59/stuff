#include <stdlib.h>
class M;
#include <misc_util.hpp>
#include "streets.hpp"

Car::Car() {

	posx = posy = 0;
	dirx = diry = speed = accel = 0;
	nodefrom = nodeto = nextnodeto = frontcar = backcar = 0;
	disttonextnode = approachingstop = 0;

} // Car::Car()


Car_array::Car_array( int n_ ) {
	n = n_;
	rep = new (Car*)[n+1];
	
	// Allocate space for cars.  When you have derived types,
	// you'll read the distribution of different vehicles from
	// a response file, and this constructor will look at
	// that file to determine just what to create in the loop
	// below.
	for( int i=1;i<=n;i++ )
		if( i%(G::porsche_to_bus_ratio+1) == 0 )
			rep[i] = new Bus;
		else
			rep[i] = new Porsche;

} // Car_array::Car_array()
//---------------------------

Car& Car_array::operator[]( int i ) {
#ifdef debuggingon
	if( (i<1) || (i>n) )
		error( "Car_array.operator[](%d) out of bounds.\n", i );
#endif   
	return *rep[i];
}
Car& Car_array::operator[]( int i ) const {
#ifdef debuggingon
	if( (i<1) || (i>n) )
		error( "Car_array.operator[](%d) out of bounds.\n", i );
#endif   
	return *rep[i];
}
//---------------------------

Car_array::~Car_array() {
	
	for( int i=1;i<=n;i++ )
		delete rep[i];

	delete[] rep;
} // ~Car_array
//---------------------------

int Car_array::num() const {
	return n;
}
//---------------------------

void Car_array::display() const { 

	// Position of pixel we want to erase:  has to be static, so it can
	//	remember where we were last time.
	static int *oldx, *oldy;
	if(!oldx) oldx = new int[G::numcars+1];
	if(!oldy) oldy = new int[G::numcars+1];

	for( int i=1;i<=n;i++ ) {

		Coordinates old_position(oldx[i],oldy[i]);
		Coordinates new_position
			= operator[](i).display( old_position );

		oldx[i] = new_position.x;
		oldy[i] = new_position.y;
	}
 
} // Car_array::display() 
//---------------------------------------------

Coordinates Porsche::display( Coordinates old_position ) const {

	int nx,ny, // new position
		ox=old_position.x, oy=old_position.y;

	//------------------------
	// Find position of new pixel: has to be just to the
	// right of the street's centerline.
	if( dirx != 0 ) {  // street not exactly vertical 
		nx = int( posx + 0.5 ); 
		ny = int( posy + 2*signum(dirx) + 0.5); 
	} 

	if( diry != 0 ) {  // street not exactly horizontal 
		nx = int( posx - 2*signum(diry) + 0.5); 
		ny = int( posy + 0.5); 
	} 
 	//-------------------------

	if( (nx>=0) && (nx<640) && (ny>=0) && (ny<480) ) {
		putpixel( nx, ny, WHITE ); 
		if( (ox!=nx) || (oy!=ny) ) 
		putpixel( ox, oy, BLACK );  // erases old pixel 
	}

	return Coordinates( nx, ny );

} // Porsche::display()
//-----------------------------------------------

Coordinates Bus::display( Coordinates old_position ) const {
// Same as Porsche::display(), except draws bigger pixels.

	int nx,ny, // new position
		ox=old_position.x, oy=old_position.y;

	//------------------------
	// Find position of new pixel: has to be just to the
	// right of the street's centerline.
	if( dirx != 0 ) {  // street not exactly vertical 
		nx = int( posx + 0.5 ); 
		ny = int( posy + 2*signum(dirx) + 0.5); 
	} 

	if( diry != 0 ) {  // street not exactly horizontal 
		nx = int( posx - 2*signum(diry) + 0.5); 
		ny = int( posy + 0.5); 
	} 
 	//-------------------------

	if( (nx>=0) && (nx<640) && (ny>=0) && (ny<480) ) {
		putbigpixel( nx, ny, WHITE ); 
		if( (ox!=nx) || (oy!=ny) ) 
		putbigpixel( ox, oy, BLACK );  // erases old pixel 
	}

	return Coordinates( nx, ny );

} // Bus::display()
//-----------------------------------------------

void Car::newstate( const Node_array& nodes ) {
// Calculate car's new state variables, except frontcar & backcar. 
// Used in statevar() as that function's main act, and in ctrlvar() 
//   to predict the position of a given car. 

	double 	timetostop, 
			distmoved, 
			extradist;    // Distance moved past next node
	int   	con; 

	// Find how far car moved. 
	timetostop = -speed/(0.00001+accel); 
		// If accel<0, might stop before end of TIMEINT, and we 
		// don't want the car backing up 
	if( ( timetostop < G::timeint ) && (timetostop>0) ) 
		distmoved = 0.5 * speed * timetostop; // ...and then it stopped
	else { 
		// car didn't stop during G::timeint
		distmoved = (speed + accel*G::timeint/2) 
				  * G::timeint; 
	} 
 
	// Find car's new position; it might be on a different street now.
	if( distmoved <= disttonextnode ) { // stayed on same street 
		posx += distmoved*dirx; 
		posy += distmoved*diry; 
		disttonextnode -= distmoved; 
	} 
	else { // Moved onto another street 
		extradist = distmoved - disttonextnode; 
		nodefrom = nodeto; 
		nodeto = nextnodeto; 
 
		// Find connection number for nextnodeto
		con = G::edgekey[ nodefrom ][ nodeto ]; 
 
		dirx = nodes[nodefrom].connections[con].dirx;
		diry = nodes[ nodefrom ].connections[con].diry; 
		// nextnodeto will be reset in ctrlvars()
 
		// Find car's new coordinates 
		posx = nodes[ nodefrom ].x + dirx * extradist; 
		posy = nodes[ nodefrom ].y + diry * extradist; 
 
		disttonextnode = nodes[ nodefrom ].connections[con].distance - extradist; 
	} // car moved to another street 
 
	// Find car's new speed.
	speed = max( 0, speed + accel*G::timeint ); 
} // Car::newstate()
//-------------------------------------
 
int Car::can_stop_now( const Node_array& nodes ) const {
// returns 1 if car can stop within G::stoplightclearance.

	int result; 

	// We create a temporary copy of *this, so we can call its
	// newstate() member, to see where it would be if it hit
	// the brakes right now. 
	Car* tempcar;
	if( mytype() == "Porsche" )
		tempcar = new Porsche;
	else
		tempcar = new Bus;

	*tempcar = *this;
	
	tempcar->accel = minaccel()/gentle_brake(); 
	(*tempcar).newstate( nodes ); 
 
	if( ( tempcar->disttonextnode < 0.5*G::stoplightclearance ) 
	||  ( tempcar->nodeto != nodeto ) ) // expects to be beyond node 
		result = 0; 
	else 
		result = 1; 
 
	delete tempcar;

	return result; 
} // Car::can_stop_now()
//----------------------------------------------------
 
double Car::approach_stop() {
// Come to a smooth stop as close as possible to G::stoplightclearance. 
// Returns the deceleration that brings this about. 

	double result; 
 
	if( disttonextnode > G::stoplightclearance+0.00001 ) 
		result = max( speed*speed
			/(2*( 0.00001 + G::stoplightclearance - disttonextnode )), 
			minaccel() 
					); 
	else 
		result = minaccel();  // the best car can do now 
 
	if ( speed == 0 ) 
		result = 0; 
 
	return result; 
} // Car::approach_stop() 
//------------------------------------------------
 
int Car::decide_turn( const Node_array& nodes ) {
// Return nextnodeto.

	int numcons, currnodeto, 
		result; 
 
	if	( ( nodeto == nextnodeto ) 
	||	( nodes.car_density( nodeto, nextnodeto ) > 0.3 ) )
	{ 
		// Just went through node; give it a plan for when it gets to nodeto.
		numcons = nodes[ nodeto ].numconnections; 
		currnodeto = nodeto; 

		// Find how many cars are headed toward every possible new node.
		double congestion[nodes[nodeto].numconnections];
		int most_congested=1;
		int least_congested=1;

		for( int k=1;k<=nodes[nodeto].numconnections;k++ ) {

			int conn_k = nodes[nodeto].connections[k].idnum;
			congestion[k] = nodes.car_density( nodeto, conn_k );

			if( ( congestion[k] > congestion[most_congested] )
			&&	( least_congested != k ) ) // otherwise, infinite loop!
				most_congested = k;
			if( ( congestion[k] < congestion[least_congested] )
			&&	( most_congested != k ) ) 
				least_congested = k;

		}
		// Defense against quirks that lead to infinite loops:
		if( most_congested == least_congested )
			least_congested = 1 + 
				(most_congested+1)%nodes[nodeto].numconnections;

		// Go to random node, except u-turn, or node already headed for.
		// Don't head onto a street that's too congested.  But that part
		//	of the code isn't working; it's making things worse, not better.
		//	I suspect it's that the code is getting mixed up about
		//	street names.
		result = nodefrom;
		int ok=0; // Set to 1 when you've found an appropriate turn.
		while( ok==0 ) {

			if( ( nodes[nodeto].connections[least_congested].idnum
				 != nodefrom )
			&&	( nodes[nodeto].connections[least_congested].idnum
				 != nodeto ) )
				result = nodes[nodeto].connections[least_congested].idnum;
			else
				result = nodes[currnodeto].
					connections[(random()%numcons) + 1].idnum; 
	
			ok=1;

			if( result == nodefrom ) ok=0;
			if( result == nodeto ) 	 ok=0;
			if( ( nodes.car_density( nodeto, result ) > 0.3 )
			&&	( result == 
					nodes[nodeto].connections[most_congested].idnum ) 
			&&  ( nodes[currnodeto].numconnections>2 ) )
				ok=0;
		}
	} 
	else // Already decided; no need to change plan.
		result = nextnodeto; 
 
	return result; 
} // Car::decide_turn()
//----------------------------------------------------

String Porsche::mytype() const {
	return "Porsche";
}
String Bus::mytype() const {
	return "Bus";
}
