#include <stdlib.h> 
class M;
#include <misc_util.hpp>
#include "streets.hpp" 
 
/**** FUNCTION DECLARATIONS ****/ 
static int gridlock( Node* node, int conto, Car_array& cars ); 
//static double intercar_dist( Car frontcar, Car car, Node_array& nodes ); 
 
/**** FUNCTION DEFINITIONS ****/ 
void ctrlvars( Node_array& nodes, Car_array& cars ) { 
 
// A car's basic desire is to proceed on its course (which may include 
//   a turn), at the maximum legal speed. 
//   Variable idealacc is the acceleration that accomplishes that goal. 
//   The only reason we won't set accel to idealacc is if that would 
//   create an unsafe following distance vis-a-vis the frontcar.  Therefore 
//   we need to calculate where the frontcar will be if it continues as 
//   it is. 
//   We also need to figure out which car will be in front of us, if we 
//   are about to pass through a node. 

	// Declare newcars[], which holds the control variable values that we 
	// set in this function.  We don't touch cars[] itself until the end, 
	// so as to preserve the simultaneous nature of the simulation. 
	struct Newcars { 
		double accel; 
		int   nextnodeto; 
	};
	static Newcars* newcars;
	if( !newcars ) newcars = new Newcars[G::numcars+1];

	static Car //pred_icar, // icar as predicted next interval,ctrlvars unchanged
		*pred_icar2; // icar next interval, assuming newcars[icar].accel 
//		pred_frontcar; // car that will be in front after G::timeint, taking 
                              // all turns into account. Its state variables 
                              // are set to those predicted for next interval. 
	int confrom, 
		conto, 
		nodeto, 
		onefront, twofront,	// boolean
		facinglight; 
	double legalspeed,  	// speed limit 
		idealspeed, 
		idealacc, 
		maxsafeacc, 
		separation, 
		will_stop_at, z,s,a_min; // to check stopping distance for red light 
 
	for( int icar=1;icar<=cars.num();icar++ ) { 
 
		//---------------------------------------------
		// Find idealacc, which would bring us to max legal speed, regardless 
		// of traffic conditions. 
		legalspeed = nodes[ cars[icar].nodeto ].legalspeed; 
		idealspeed = min( cars[icar].maxspeed(), legalspeed ); 
		idealacc = max( cars[icar].minaccel(), 
						min(cars[icar].maxaccel(), 
						(idealspeed-cars[icar].speed)/G::timeint ) ); 
 		//---------------------------------------------

		//---------------------------------------------
		// Find separation from frontcar if we keep going at current speed. 
		// We may be on different streets, so it's trickier than in traffic.pas. 
		//	separation = intercar_dist( &pred_frontcar, &pred_icar, nodes ); 
		//	This is the fancy, slower way to do it, that may be unavoidable when 
		//	we go to multi-lane traffic. 
		if(( cars[icar].frontcar != 0 )
		&& ( cars[icar].nodeto == cars[ cars[icar].frontcar ].nodeto ) )
			separation = cars[icar].disttonextnode 
			- cars[ cars[icar].frontcar ].disttonextnode; 
		else 
			separation = 10000; 
		separation = round( separation, 0.000001 ); // prevents underflow
		//---------------------------------------------

		//---------------------------------------------
		// Find maxsafeacc, the maximum acceleration that will preserve a safe 
		// distance behind the car that will be in front of us. 
		// Safedistance and maxsafeacc are the solutions to the following 
		//   simultaneous system: 
		// 
		//   safedistance = 
		//     	2 + 2*(cars[icar].speed + maxsafeacc*G::timeint)
		//			*G::timeint; 
		//	that's conservative; icar has a 1 timeint reaction time.  The 2+ 
		//		is to maintain one car length in between, even when stopped. 
		//   -maxsafeacc = 2*( separation - safedistance )
		//					/(G::timeint*G::timeint) 
		//               + pred_icar.accel; 
		//   The solution for maxsafeacc is... 
		cars[icar].speed = round( cars[icar].speed, 0.000001 ); 
		maxsafeacc = ( /*2**/(separation - 2 - 2*cars[icar].speed)*G::timeint*G::timeint 
			+ cars[icar].accel ) 
			/( 4.0*G::timeint*G::timeint*G::timeint /* -1*/ ); 
		// (But there was still a gridlock problem, so I've empirically
		// determined that it's best to hold the acceleration down a little:
		// hence the terms inside /* */ in the expression above for
		// maxsafeacc.)

		newcars[icar].accel = 
			max(cars[icar].minaccel(), min( maxsafeacc, idealacc ) ); 
		//---------------------------------------------

		//---------------------------------------------
		// Decide which way to turn.  A car may decide anytime it's more 
		// than Car::annctime ("announce time") seconds away from nodeto, and that 
		// includes the possibility of changing one's mind while stopped 
		// at a light. 

		if( cars[icar].disttonextnode > cars[icar].speed*cars[icar].annctime() ) 
			newcars[icar].nextnodeto = 
			cars[icar].nextnodeto = cars[icar].decide_turn( nodes ); 
		else 
			newcars[icar].nextnodeto = cars[icar].nextnodeto; 
  
		if( cars[icar].nodeto == newcars[icar].nextnodeto ) 
			error( "nodeto == nextnodeto\n" );
		//---------------------------------------------

		//----------------------------------------------------
		// Deal with stop lights, gridlock, and right-of-way. 
		// If there are >1 cars in front, then ignore the light; frontcar will 
		//   worry about it and icar will follow at a safe distance. 
		//   But if there's exactly 1 car in front, we can't let icar ignore 
		//   the light, or lots of cars will follow lemming-like through red 
		//   lights and stop signs. 
		// Otherwise, consider stopping if the light is red or there's 
		//   gridlock, or there's a car coming with a higher right-of-way 
		//   level. 

		// Some convenient definitions for the coming complex condition...
		confrom = G::edgekey[ cars[icar].nodeto ][ cars[icar].nodefrom ]; 
		conto   = G::edgekey[ cars[icar].nodeto ][ cars[icar].nextnodeto ]; 
		// Unfortunately, nextnodeto hasn't been reset since the
		// car initialization, when it was "temporarily" set to 
		// equal nodeto.  Thus, conto often gets set to some kind
		// of garbage.  That's why I've taken the code that invokes
		// decide_turn(), and moved it above here.
		nodeto = cars[icar].nodeto; 
		onefront = ( cars[icar].frontcar != 0 )  // exactly 1 car in front 
			&& ( cars[ cars[icar].frontcar ].frontcar == 0 ); 
		twofront = ( cars[icar].frontcar != 0 )  // >1 car in front 
			&& ( cars[ cars[icar].frontcar ].frontcar != 0 ); 
		facinglight = nodes[nodeto].light.color[confrom][conto]; 
 
		if( twofront 
		||  ( ( facinglight==GREEN ) 
			&&( !gridlock( &nodes[nodeto], conto, cars ) 
				|| onefront ) ) ) 
			cars[icar].approachingstop = 0; 
		else { 
			if( cars[icar].approachingstop == 1 ) 
				newcars[icar].accel = cars[icar].approach_stop();
			else { 
				// There's no car in front, so see if you'd be able to stop in time 
				//   if you decelerated at minaccel/Car::gentle_brake one 
			   	//	G::timeint from now. 
               	// This is where the decision to slow down is initiated (after 
               	//   that approach_stop() takes over). 
				
				if( cars[icar].mytype() == "Porsche" )
	               	pred_icar2 = new Porsche;
				else
					pred_icar2 = new Bus;
				*pred_icar2 = cars[icar];

               	pred_icar2->accel = newcars[icar].accel; 
               	pred_icar2->newstate( nodes ); 
               	// pred_icar2 assumes car accelerates at newcars[icar].accel; 
               	// pred_icar assumes current acceleration continues. 
 
               	z = pred_icar2->disttonextnode; 
               	s = pred_icar2->speed; 
               	a_min = pred_icar2->minaccel()/pred_icar2->gentle_brake(); 
               	will_stop_at = z + s*s/(2*a_min); 
               	// that's how far from the node you'd be 
 
				if( ( ( will_stop_at < G::stoplightclearance ) 
                   ||( pred_icar2->nodeto != cars[icar].nodeto ) ) 
               	&&  ( cars[icar].can_stop_now( nodes ) ) ) 
			   	{ 
                   cars[icar].approachingstop = 1; 
                   newcars[icar].accel = cars[icar].approach_stop();
			   	} 

				delete pred_icar2;
			} 
		} 
		//--------- done dealing with stoplights & gridlock-------------------
 
	} // for icar 
 
 
	// Copy from newcars[] to cars[] 
	for( icar=1;icar<=cars.num();icar++ ) { 
		cars[icar].accel = newcars[icar].accel; 
		cars[icar].nextnodeto = newcars[icar].nextnodeto; 
	} 
 
} // ctrlvars()
//------------------------------------------

/* 
double intercar_dist( Car frontcar, Car car, Node_array& nodes ) 
{ // Returns distance between frontcar and car. 
  // If they're not on the same 
  //   edge, we assume frontcar is on the edge car is going to turn onto. 
  //   If this assumption proves to be wrong, we return G::bigdist, which is 
  //   big enough to let car know the coast is ctly clear. 

  int con; 
  double result, 
        internodedist; 
 
 
  if( frontcar.nodeto == car.nodeto ) 
      // They're on the same edge.
      result = car.disttonextnode - frontcar.disttonextnode; 
  else { 
      if( frontcar.nodeto == car.nextnodeto ) { 
          // Frontcar is on the edge car is going to turn onto.
          con = G::edgekey[ frontcar.nodeto ][ frontcar.nodefrom ]; 
          internodedist = nodes[ frontcar.nodeto ].connections[con].distance; 
          result = car.disttonextnode 
                 + internodedist - frontcar.disttonextnode; 
      } 
      else 
          result = G::bigdist; 
  } 
 
  return result; 
} // intercar_dist() 
*/
//--------------------------------------------------
  
int gridlock( Node* node, int conto, Car_array& cars ) 
{ // Checks for gridlock on node->connections[conto]. 
  //	 endcar is the endcar.from on the edge we're checking for gridlock. 

  int endcar, 
      result; 
  double endcardist;  // endcar distance from node it's coming from.
 
  endcar = node->connections[conto].endcar.from; 
  if( endcar==0 ) 
      result = 0; 
  else { 
      endcardist = node->connections[conto].distance 
                 - cars[endcar].disttonextnode; 
 
      if( endcardist + cars[endcar].speed * G::timeint 
	  < G::stoplightclearance ) 
          result = 1; 
      else 
          result = 0; 
  } 
 
  return result; 
} // gridlock() 

