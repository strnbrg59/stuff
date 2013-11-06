#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <chibitmap.hpp>
class M;
#include <misc_util.hpp>
#include "streets.hpp"

Node::Node() {
	connections = new Connection[G::maxconnections];
}

Node::~Node() {
	delete[] connections;
}

Node_array::Node_array() {

	loadnetwork();

	set_distndir(); 
	draw_streets(); 

	initlights();

} // Node_array::Node_array()
//---------------------------

Node& Node_array::operator[]( int i ) {

#ifdef debuggingon
	if( (i<1) || (i>n) )
		error( "Node_array.operator[](%d) out of bounds.\n", i );
#endif

	return rep[i];
} // Node_array::operator[]
//---------------------------

Node& Node_array::operator[]( int i ) const {

#ifdef debuggingon
	if( (i<1) || (i>n) )
		error( "Node_array.operator[](%d) out of bounds.\n", i );
#endif

	return rep[i];
} // Node_array::operator[] const
//---------------------------

Node_array::~Node_array() {
	delete[] rep;
} // !Node_array()
//---------------------------

int Node_array::num() const {
	return n;
}
//---------------------------

void Node_array::loadnetwork() { 
// Loads network data from file, then turns us over to the other functions, 
//	enabling us to add to the network. 
//
// This function does most of the work of the Node_array constructor.

	//-----------------------------------------
	// Figure out how many nodes there are: just count the number
	//	of lines in the node file.
	n=0;
	{ // separate block, to ensure reclosing of infile.
	ifstream infile( G::netdatfile );
	if( !infile ) error( "loadnetwork: can\'t find %s.\n", 
		G::netdatfile.chars() );
	const int len=80;
	char buf[len+1];
	while( !infile.eof() ) {
		infile.getline( buf, len );
		n ++;
	}
	}
	//---------- done counting nodes ----------

	rep = new Node[n+1];

	int idnum=0, // node id number 
		numconn,othernode; 
	 
	//---------------------------------------------------------
	// Reopen network data file and load connections information.
	ifstream infile(G::netdatfile);
	if( !infile ) 
		error( "Can\'t find %s.\n", G::netdatfile.chars() );	
 
	int i=0, len=80;
	char wholeline[len+1];

	while ( !infile.eof() ) { 
 
		infile.getline( wholeline, len );

		idnum = atoi( strtok( wholeline, " \t" ));
		operator[](idnum).x = atoi( strtok( NULL, " \t" ));
		operator[](idnum).y = atoi( strtok( NULL, " \t" ));

		operator[](idnum).numconnections = 0; 
		operator[](idnum).idnum = idnum; 
//		outtextxy(operator[](idnum).x,operator[](idnum).y, itoa(idnum,buffer,10)); 

		othernode = atoi( strtok(NULL, " \t"));

		while (othernode!=0) { 
			// 0 indicates end of othernode list */ 
 
			operator[](idnum).numconnections++; 
			numconn = operator[](idnum).numconnections; 
			operator[](idnum).connections[numconn].idnum = othernode; 
			othernode = atoi( strtok(NULL, " \t"));

		} // while othernode!=0 

		i++;
		cout << "Read " << i << " lines of network.dat.\n";

	} // !infile.eof()
	//---------------------------------------------------------

	if( i != n )
		error( "Recheck %s; make sure there's no blank last line.\n", 
			G::netdatfile.chars() );

	//---------------------------------------------------------
	// Check the network for integrity.  Make sure if all connections
	// are two-way; if node i is connected to node j, then j has to
	// be connected to i.

	for( i=1;i<=n;i++ ) {
		for( int j=1;j<=operator[](i).numconnections;j++ ) {
			int c = operator[](i).connections[j].idnum;

			int reverse_exists=0;
			int k=1;
			while( !reverse_exists && ( k<=operator[](c).numconnections ) ) {
				if( operator[](c).connections[k++].idnum == i )
					reverse_exists=1;
			}
			
			if( !reverse_exists )
				error( "Connection between %d and %d is only one-way!\n",
					i,c );
		}
	}
	//-----------------------------------------------------------
 
} // Node_array::loadnetwork() 
//--------------------------------------
 
void Node_array::set_distndir() 
{ // Set distances and directions between all pairs of nodes. 
  // Fills G::edgekey[][], where G::edgekey[i][j] gives the connection 
  //   number of j, within i's .connections field. 
  // This is called after all node coordinates have been read. 

  int ownid,otherid, 
      con; // Connection number (in nodes.connections field) 
 
  G::edgekey = imatrix(1,n,1,n); 
 
  for( ownid=1;ownid<=n;ownid++ ) 
  for( con=1;con<=operator[](ownid).numconnections;con++ ) 
  {  // loop over all pairs of nodes 
 
      otherid = operator[](ownid).connections[con].idnum; 
      G::edgekey[ownid][otherid] = con; 
 
      // set distance 
      operator[](ownid).connections[con].distance 
            = sqrt( 
                pow(operator[](ownid).x - operator[](otherid).x, 2 ) 
               +pow(operator[](ownid).y - operator[](otherid).y, 2) 
                  ); 

	if( operator[](ownid).connections[con].distance 
		< 2*Bus::annctime_*Porsche::maxspeed_ ) 
	{
		cerr << "Nodes " << ownid << " and " << otherid 
			 << " are too close together. ";
		error( "Decrease Bus:annctime or Porsche::maxspeed.\n");
	}
 
	// set directions for own node
	operator[](ownid).connections[con].dirx 
		= ( operator[](otherid).x - operator[](ownid).x ) 
		/ operator[](ownid).connections[con].distance; 
	operator[](ownid).connections[con].diry 
		= ( operator[](otherid).y - operator[](ownid).y ) 
		/ operator[](ownid).connections[con].distance; 
 
	} // for ownid, for con 
 
} // Node_array::set_distndir()
//----------------------------------------------------
 
void Node_array::draw_streets() const {
	int inode, nodeto, con; 
	double dirx, diry, perpx, perpy; 
 
#	ifdef vgagraphics
	cwfont chifont( "/users/ted/src/rpnprog/standard.eft" );
#endif

	for( inode=1; inode<=n; inode++ ) {
		Node& from 	= operator[](inode);

#		ifdef vgagraphics
#		ifdef debuggingon
		char buf[80];
		sprintf(buf,"%d",inode);
		chifont.printxy( buf, from.x, from.y );
#		endif
#		endif

		for( con=1; con<=from.numconnections; con++ ) { 
 
			dirx = from.connections[con].dirx; 
			diry = from.connections[con].diry; 

			// find perpendicular 
			if( dirx==0 ) 
				perpy = 0; 
			else 
				perpy = 1/sqrt( diry*diry/(dirx*dirx) + 1 ); 
			perpx = sqrt( 1 - perpy*perpy ); 
 
			nodeto = from.connections[con].idnum; 
			Node& to = operator[](nodeto);
 
			vga_drawline( 
				int(from.x - G::lanewidth*dirx + G::lanewidth*perpx + 0.5),
				int(from.y - G::lanewidth*diry + G::lanewidth*perpy + 0.5),
				int(to.x - G::lanewidth*dirx + G::lanewidth*perpx + 0.5), 
				int(to.y -G::lanewidth*diry + G::lanewidth*perpy + 0.5) ); 

			vga_drawline( 
				int(from.x + G::lanewidth*dirx - G::lanewidth*perpx + 0.5), 
				int(from.y + G::lanewidth*diry - G::lanewidth*perpy + 0.5),
				int(to.x + G::lanewidth*dirx - G::lanewidth*perpx + 0.5), 
				int(to.y + G::lanewidth*diry - G::lanewidth*perpy + 0.5) ); 
		}
 
	}
 
	vga_setcolor(WHITE); 
} // Node_array::draw_streets() 
//-------------------------------

void Node_array::initlights() {
// allocate light.colors field, and set other parameters 
// initialize node legal speeds. 
 
	int inode, con1, con2, 
		numcons;
 
	// Allocate light.color matrix. Initialize all lights to RED.
	for( inode=1;inode<=n;inode++ ) { 
		numcons = operator[](inode).numconnections; 
		operator[](inode).light.color = imatrix(1,numcons,1,numcons); 
		for( con1=1;con1<=numcons;con1++ ) 
			for( con2=1;con2<=numcons;con2++ ) 
				operator[](inode).light.color[con1][con2] = RED; 
	} 
 
	// Set one color[][] element per node to green; all others remain RED.
	for( inode=1;inode<=n;inode++ ) 
		operator[](inode).light.color[2][1] = GREEN; 
 
	// Set interval and lag.
	for( inode=1;inode<=n;inode++ ) { 
		operator[](inode).light.interval = 60; 
		operator[](inode).light.lag      =  inode*20; 
	} 
 
	// set node 9 to stop light mode 
	// nodes[9].light.interval = 2; 
 
	// for Monterey monster traffic jam (bigjam.dat): set lights 2-4 to 
	// perpetual green, light 1 to a normal interval 
	//   nodes[2].light.interval = nodes[3].light.interval 
	//  = nodes[4].light.interval = 32766; 
	//  nodes[1].light.interval = 20; 
 
	// set node legal speeds 
	for( inode=1;inode<=n;inode++ ) { 
		operator[](inode).legalspeed = Porsche::maxspeed_; 
	} 

} // Node_array::initlights()  
//----------------------------------------------------
 
void Node_array::light_state( int clocktick ) { 
// Called once per clocktick (iteration), from main().  
// Sets new state for lights. 

	static int* i_cycle; // counts number of light-change cycles.
	if( !i_cycle )
		i_cycle = new int[n];
 
	for( int inode=1;inode<=n;inode++ ) { 
 
		if( !(operator[](inode)).light_time( clocktick ) )
			;  // light_time() checks interval and lag 
		else { 
			// operator[](inode).random_green();
			// operator[](inode).all_green();

			// Use green_on_demand 4 out of 5 times, but now and then (
			// 1 out of 5) just rotate them.  Otherwise you get weird
			// bottlenecks.
			if( i_cycle[inode]%10 == 0 )
				operator[](inode).rotate_green(); 
			else
				operator[](inode).green_on_demand();

			i_cycle[inode] ++;
		}
 
		operator[](inode).display_lights();
 	} 
 
} // Node_array::light_state() 
//-----------------------------------------------------------
 
void Node::rotate_green() {
// For a given con1, make it green to all con2's; red for all other con1's 
  int con1, con2, 
      degel=0;  // flag 
 
  for( con1=1;con1<=numconnections;con1++ ) { 
      if( light.color[con1][1] == GREEN ) { 
          for( con2=1;con2<=numconnections;con2++ ) 
              light.color[con1][con2] = RED; 
          degel = 1; 
      } 
      if( degel == 2 ) { 
          degel = 0; 
          // Set all con2's green 
          for( con2=1;con2<=numconnections;con2++ ) 
              light.color[con1][con2] = GREEN; 
      } 
      if( degel == 1 ) 
          degel ++; 
  } 
 
  if( degel == 2 ) // All are red; set con1 to green 
      for( con2=1;con2<=numconnections;con2++ ) 
          light.color[1][con2] = GREEN; 
 
} // Node::rotate_green()
//-----------------------------------------

void Node::green_on_demand() {
// Set to green the light for the connection with the most cars on it.
// Set light to all other connections to red.

	int mostestcars= -1;
	int busiest_con=0;

	// Set all lights to red (you'll turn one to green, before this function
	// returns).
	for( int con1=1;con1<=numconnections;con1++ )
		for( int con2=1;con2<=numconnections;con2++ )
			light.color[con1][con2] = RED;

	// Find busiest connection.
	for( int con=1;con<=numconnections;con++ ) {
		if( connections[con].ncars > mostestcars ) {
			mostestcars = connections[con].ncars;
			busiest_con = con;
		}
	}

	// Set its light to green.
	for( int con2=1;con2<=numconnections;con2++ ) 
		light.color[busiest_con][con2] = GREEN;
} // Node::green_on_demand()
//-----------------------------------------
 
void Node::random_green() {
// Set lights green and red randomly.
  int con1, con2;
 
	for( con1=1;con1<=numconnections;con1++ ) 
	for( con2=1;con2<=numconnections;con2++ ) 
		light.color[con1][con2] = random()%2; 
 
} // Node::random_green() 
//-----------------------------------------
 
void Node::all_green() {
// All lights are green all the time.
  int con1, con2;

	for( con1=1;con1<=numconnections;con1++ ) 
	for( con2=1;con2<=numconnections;con2++ ) 
		light.color[con1][con2] = GREEN; 
} // all_green()
//-----------------------------------------
 
int Node::light_time( int clocktick ) const { 
// Tells if it's time to reset the lights at node. Returns 1 if it is.
  int remainder; 
 
  remainder = ( clocktick - light.lag ) % light.interval; 
 
  if( remainder==0 ) 
      return 1; 
  else 
      return 0; 
} // Node::light_time() 
//------------------------------------------

void Node::display_lights() const {
// Show traffic light state by lighting up green and red lights as appropriate.
  int con1; 
  double dirx, diry, perpx, perpy; 
 
  for( con1=1;con1<=numconnections;con1++ ) { 
 
      dirx = connections[con1].dirx; 
      diry = connections[con1].diry; 

      // find perpendicular 
      if( dirx==0 ) 
          perpy = 0; 
      else 
          perpy = 1/sqrt( diry*diry/(dirx*dirx) + 1 ); 
      perpx = sqrt( 1 - perpy*perpy ); 
 
      if( (light).color[con1][1] == GREEN ) 
          vga_setcolor(GREEN); 
      else 
          vga_setcolor(RED); 
 
	  vga_drawline(
		int(x + G::lanewidth*dirx + G::lanewidth*perpx + 0.5), 
        int(y + G::lanewidth*diry + G::lanewidth*perpy + 0.5),
		int(x + G::lanewidth*dirx - G::lanewidth*perpx + 0.5), 
		int(y + G::lanewidth*diry - G::lanewidth*perpy + 0.5) ); 
 
      vga_setcolor(RED); 
 
  } // for con1 
 
} // Node::display_lights()
//--------------------

//-----------------------

int Node_array::cars_on_lane( int from, int to ) const {
// Returns number of cars currently proceeding from node "from" to node "to".
	int result = operator[](to).connections[G::edgekey[to][from]].ncars;
	return result;
} // Node_array::cars_on_lane()
//------------------------

double Node_array::car_density( int from, int to ) const {
// Cars per length of a street in pixels.
	double result = cars_on_lane(from,to)
				  / operator[](to).connections[G::edgekey[to][from]].distance;
	return result;
} // Node_array::car_density()
//------------------------