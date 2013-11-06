#include <String.h>
#include "streets.hpp"

int G::numcars 			= 100;
int G::maxnodes 		= 60;
int G::maxconnections	= 10;
int G::porsche_to_bus_ratio = 0;
String G::netdatfile	= "network.dat";

int G::lanewidth		= 4;
int G::bigdist			= 10000; 

double Porsche::maxspeed_ 		= 5;
double Porsche::maxaccel_		= 0.5;
double Porsche::minaccel_		= -2;
double Porsche::gentle_brake_	= 2; 
double Porsche::annctime_		= 2; 

double Bus::maxspeed_ 		= 5;
double Bus::maxaccel_		= 0.1;
double Bus::minaccel_		= -2;
double Bus::gentle_brake_	= 1; 
double Bus::annctime_		= 2; 

int G::stoplightclearance = 15; 

int G::timeint			= 1;  // Program not ready for other than 1!
int G::usleep			= 100000; //100000;

int **G::edgekey; 



