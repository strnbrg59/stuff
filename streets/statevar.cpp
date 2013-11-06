#include <math.h> 	// for ceil() 
#include <stdlib.h> // for qsort()
class M;
#include <misc_util.hpp> // for max template
#include "streets.hpp" 
 
// LOCAL FUNCTION DECLARATIONS 
// light color control 
static int compare 
    ( const void *dist1, const void *dist2 );  // for qsort() 
static void make_lists 
    ( Node_array& nodes, Car_array& ); 
static void sortcars( Node_array& nodes, Car_array& cars );
static void sort_list 
    ( int inode, int con, Node_array& nodes, Car_array& ); 
static void start_at( int percent, int nodefrom, int nodeto, 
               Node_array& nodes, Car_array& ); // car initialization 
 
// FUNCTION DEFINITIONS 
void start_cars( Node_array& nodes, Car_array& cars ) { 
 
  // use start_at( int percent, int nodefrom, int nodeto, 
  //               Node_array& nodes, int numcars, Car_array& cars ) 
  // to place percent% of the cars at nodefrom, heading toward 
  // nodeto. 
/* 
  start_at( 10, 1, 2, nodes, cars ); 
  start_at( 10, 5, 4, nodes, cars ); 
  start_at( 5, 2, 6, nodes, cars ); 
  start_at( 10, 1, 22, nodes, cars ); 
  start_at( 5, 6, 8, nodes, cars ); 
  start_at( 10, 22,21, nodes, cars ); 
  start_at( 5, 19,20, nodes, cars ); 
  start_at( 10, 16,8, nodes, cars ); 
  start_at( 5, 16,15, nodes, cars ); 
  start_at( 10, 16,17, nodes, cars ); 
  start_at( 5, 12,23, nodes, cars ); 
  start_at( 10, 13,14, nodes, cars ); 
  start_at( 5, 15,16, nodes, cars ); 
*/
  start_at( 100,1,2, nodes, cars );

  sortcars( nodes, cars ); 
 
} // start_cars() 
//------------------------------------------------
 
void statevars(  Node_array& nodes, Car_array& cars ) 
{ // Assumes car doesn't pass through >1 intersection within one TIMEINT. 
	int icar;
 
	// Find each car's new position and speed.
	for( icar=1;icar<=cars.num();icar++ ) { 
		cars[icar].newstate( nodes ); 
	} 
 
	sortcars( nodes,cars ); 
 
} // statevars() 
//-------------------------------------------
 
static void sortcars( Node_array& nodes, Car_array& cars ) 
{ // Reset nodes' endcar fields, reset cars' frontcar and backcar fields. 
  // 
  // Algorithm is to go through all the cars, attaching them to linked lists 
  //   headed by each node's .endcar.to field. 
  //   Then go through all pairs of nodes, sorting the cars on their linked 
  //   lists. 
  // Eventually, we should be able to skip over streets with no entries or 
  //   exits since the last iteration. 

  int inode, con,
	  numnodes = nodes.num();
 
  // Set all nodes' .endcar fields to zero.
  for( inode=1;inode<=numnodes;inode++ ) 
  for( con=1;con<=nodes[inode].numconnections;con++ ) { 
      nodes[inode].connections[con].endcar.to = 0; 
      nodes[inode].connections[con].endcar.from = 0; 
  } 
 
  // Assign the cars to linked lists headed by each node's .endcar.to field. 
  make_lists( nodes, cars ); 
 
  // Sort each linked list. 
  for( inode=1;inode<=numnodes;inode++ ) 
  for( con=1;con<=nodes[inode].numconnections;con++ ) { 
      nodes[inode].connections[con].ncars = 0; 
      if( nodes[inode].connections[con].endcar.to != 0 ) 
          // don't do anything if the list is empty 
          sort_list( inode, con, nodes, cars ); 
  } 
 
} // sortcars() 
//----------------------------------------------------
 
static void make_lists( Node_array& nodes, Car_array& cars ) 
{ // Called by sortcars(), creates linked lists attached to nodes.connections. 
  //   endcar.to, which are then sorted in sort_lists(). 

  int icar, 
      nodeto, nodefrom, con, endcar_to;  // shorthand 
 
  for( icar=1;icar<=cars.num();icar++ ) { 
 
      // a few convenient definitions 
      nodeto = cars[ icar ].nodeto; 
      nodefrom = cars[ icar ].nodefrom; 
      con = G::edgekey[ nodeto ][ nodefrom ]; 
      endcar_to = nodes[ nodeto ].connections[ con ].endcar.to; 
 
      // push icar onto linked list 
      if( endcar_to == 0 ) { 
          // list is empty 
          nodes[ nodeto ].connections[ con ].endcar.to = icar; 
          cars[ icar ].backcar = 0; 
      } 
      else { 
          // list not empty 
          cars[ endcar_to ].frontcar 
              = icar; 
          cars[ icar ].backcar = endcar_to; 
          nodes[ nodeto ].connections[ con ].endcar.to = icar; 
      } 
  } // for icar 
 
} // make_lists() 
//---------------------------------------------------
 
static void sort_list( int inode, int con, 
	Node_array& nodes, Car_array& cars ) 
{ 
// The objective is to reset nodes' endcar fields, reset cars' frontcar 
//   and backcar fields. 
// This function works on just one linked list. 
// 
// Algorithm is to encode each car's idnum and disttonextnode in one 
//   element of car_n_dist[], and then to submit car_n_dist to C's 
//   qsort(). 
 
  int ncars=0,  // Number of cars on the list (not nec. same as numcars.
      currcar, 
      icar, 
      othernode, 
      inodeinv; 
  long car_n_dist[ cars.num()+2 ]; 
 
  // Count the number of cars on the list.  This function isn't called if 
  //   the number is zero. 
  // As you count, fill car_n_dist[]. 

  currcar = nodes[inode].connections[con].endcar.to; 
  while( currcar != 0 ) { 
      ncars++; 
      car_n_dist[ncars] = currcar + 
          32768L * (int)(cars[currcar].disttonextnode); 
          // This encodes both the distance and the car number, for qsort().
 
      currcar = cars[currcar].backcar; 
  } 
 
  nodes[inode].connections[con].ncars = ncars; // For summary stats 
 
  // Sort car_n_dist[] 
  qsort( (void *)(car_n_dist+1), ncars, sizeof(long), compare ); 
 
  // Set nodes' endcar .to and .from fields.
  nodes[inode].connections[con].endcar.to = car_n_dist[1] % 32768L; 
    // % 32768L is not the same as % 32768L, for reasons of 2's complement 
    // encoding, so beware.  It does run faster with the bit-operation, so 
    // you should try to go to that; but 2's complementation will make it 
    // tricky.  See bitwise.c for an example of how to make it work. 

  othernode = nodes[inode].connections[con].idnum; 
  inodeinv = G::edgekey[othernode][inode]; 
  nodes[ othernode ].connections[inodeinv].endcar.from = 
      car_n_dist[ncars] % 32768L; 
 
  // Set the cars' frontcar and backcar fields.
  car_n_dist[0] = car_n_dist[ncars+1] = 0; 
  for( icar=1;icar<=ncars;icar++ ) { 
     currcar = car_n_dist[ icar ] % 32768L; 
     cars[currcar].frontcar = car_n_dist[icar-1] % 32768L; 
     cars[currcar].backcar = car_n_dist[icar+1] % 32768L; 
 
     // front_back( cars+currcar );
     // for debugging
  } 
  
} // sort_list() 
//---------------------------------------------
 
int compare( const void *dist1, const void *dist2 ) {
// for qsort() 
	if( *(long *)dist1 < *(long *)dist2 ) 
		return -1; 
	else 
		if( *(long *)dist1 > *(long *)dist2 ) 
			return 1; 
		else 
			return 0; 
 
} // compare() 
//--------------------------------------
 
 
void start_at( int percent, int nodefrom, int nodeto, 
               Node_array& nodes, Car_array& cars ) 
{ // Places percent of the cars at nodefrom, heading to nodeto.  Cars start 
  //   stationary, separated by 2 pixels. 
  // Knows where previous start_at() left off by looking at cars.nodefrom 
  //   field, which is initialized to zero.
  //
  // Better check that there really is a connection between nodefrom to
  //	nodeto!
 
  int cars_already_placed, icar, 
      firstcar, con  ; 
 
  if( percent==0 ) return; 
 
  //  Find first car not yet placed.
  cars_already_placed = 0; 
  while( cars[ ++cars_already_placed ].nodefrom != 0 ); 
  cars_already_placed--; 
 
  if( cars_already_placed == cars.num() ) return; 
 
  // Place the first car not yet placed.
  firstcar = cars_already_placed + 1; 
  con = G::edgekey[ nodefrom ][ nodeto ]; 
  if( con == 0 ) 
	error( "There's no connection between node %d and node %d.\n",
			nodefrom, nodeto );
 
  cars[firstcar].dirx = nodes[nodefrom].connections[con].dirx; 
  cars[firstcar].diry = nodes[nodefrom].connections[con].diry; 
  cars[firstcar].posx = nodes[nodefrom].x; 
  cars[firstcar].posy = nodes[nodefrom].y; 
  cars[firstcar].speed = 0; 
  cars[firstcar].accel = 0; 
  cars[firstcar].nodefrom = nodefrom; 
  cars[firstcar].nodeto   = nodeto; 
  cars[firstcar].nextnodeto = nodeto; // Will be reset in decide_turn()
  cars[firstcar].disttonextnode = 
    nodes[ nodefrom ].connections[con].distance; 
  cars[firstcar].approachingstop = 0; 
 
  //  Initialize and place the next percent*numcars cars.
  for( icar=cars_already_placed + 2; 
       icar <= min( cars.num(),
       cars_already_placed + (int)ceil((double)0.01*percent*cars.num()) 
                  ); 
       icar++ ) 
  { 
	  cars[icar].posx = cars[icar-1].posx - 2*cars[icar-1].dirx; 
      cars[icar].posy = cars[icar-1].posy - 2*cars[icar-1].diry; 
      cars[icar].dirx = cars[icar-1].dirx; 
      cars[icar].diry = cars[icar-1].diry; 
      cars[icar].speed = cars[icar-1].speed; 
      cars[icar].accel = cars[icar-1].accel; 
      cars[icar].nodefrom = cars[icar-1].nodefrom; 
      cars[icar].nodeto = cars[icar-1].nodeto; 
      cars[icar].nextnodeto = cars[icar-1].nextnodeto; 
      cars[icar].disttonextnode = cars[icar-1].disttonextnode + 2; 
      cars[icar].approachingstop = 0; 
  } // for icar
 
} // start_at()
//--------------------------------------------
