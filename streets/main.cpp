#include <iostream.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#ifdef vgagraphics
#	include <chibitmap.hpp>
#endif

#include "streets.hpp"

/**** MAIN ****/ 

// For X graphics, the main() is in Xwrapper.cpp.
#ifdef Xgraphics
void main_streets() { 
#else
main() {
#endif

//  randomize();
# 	ifdef vgagraphics
		grafinit(); 
#	endif

	Node_array nodes;

	Car_array cars( G::numcars );
	start_cars( nodes, cars );  // Places cars in starting positions.

	int clocktick=0;
	while(1) { 
		clocktick++; 

		nodes.light_state( clocktick );
		statevars( nodes, cars ); 
		ctrlvars( nodes, cars ); 
		cars.display();

		usleep(G::usleep);

#		ifdef nographics
		if( clocktick%1000 == 0 ) cerr << clocktick << "iterations\n";
#		endif

		sumstats( nodes, cars, clocktick ); 
	} 
 
	sumstats( nodes, cars, CLOSE );  // close the stats file 
	vga_setmode( TEXT );

} // main_streets()


