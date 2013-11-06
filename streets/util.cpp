#include <math.h> 
#include <stdarg.h> // for va_list
#include <stdio.h> 
#ifdef vgagraphics
#	include <vga.h>
#endif
#include <stdlib.h> 
#include "streets.hpp" 

//----------------------------------

Coordinates::Coordinates( int xx, int yy ) {
	x = xx;
	y = yy;
}
//-----------------------------------
 
int **imatrix(int nrl,int nrh,int ncl,int nch) 
{ 
	int i; 
	int **m; 
 
	m=(int **) malloc((unsigned) (nrh-nrl+1)*sizeof(int*)); 
	if (!m) printf("allocation failure 1 in cmatrix()"); 
	m -= nrl; 
 
	for(i=nrl;i<=nrh;i++) { 
		m[i]=(int *) malloc((unsigned) (nch-ncl+1)*sizeof(int)); 
		if (!m[i]) printf("allocation failure 2 in cmatrix()"); 
		m[i] -= ncl; 

		for( int j=ncl;j<=nch;j++ )
			m[i][j] = 0;
	} 
	return m; 
} // imatrix() 
//-----------------------------------------
 
int signum( double x ) { 
  int result; 
  if( x<0 ) result = -1; 
  if( x>0 ) result = 1; 
  if( x==0 ) result = 0; 
  return result; 
} // signum() 
//---------------------------------------------
 
void grafinit(void) { 
  vga_init();
  vga_setmode(G640x480x2);
} // grafinit()
//----------------------------------------------
 
/*
void manual_control( char cmd ) {
// called from main() if kbhit().  Doesn't work!
   switch ( cmd ) { 
       case ' ' : while( !kbhit() ); getch(); break; 
       case 'q' : vga_setmode(TEXT); exit(0); 
       default  : ; 
   } 
} // manual_control()
*/
//----------------------------------------------
 
double round( double x, double y ) 
{ // round x to nearest y 
  double result; 
  result = y*floor( 0.5 + x/y ); 
  return result; 
} // round()
//-----------------------------------------------
 
void sumstats( const Node_array& nodes, const Car_array& cars, int i ) { 
  // summary statistics of the situation. 
  // i is the number of iterations to date of the simulation; we might want 
  //   to take our snapshot every 10th or 100th iteration, to save space. 

  int interval=10,  // number of iterations between snapshots 
      ncars, 
	  numnodes = nodes.num(),
	  totcars = cars.num();
	
  double chisqr=0; // measure of car dispersion 
  static FILE *statfile; 
 
  if( (i-1)%interval == 0 ) { 
 
		if( i==1 ) // first time: open statfile.
		statfile = fopen( "statfile.out", "w" ); 
 
		// Find chi-square stat for cars' distribution over edges.
		for( int j=1;j<=numnodes;j++ ) {
			for( int k=1; k<=nodes[j].numconnections; k++ ) { 
				ncars = nodes[j].connections[k].ncars; 
				if( j<2 ) 
					fprintf( statfile, "%d ", ncars ); 
				chisqr += pow( totcars/26.0 - ncars, 2 ); 
			}
		}
		fprintf( statfile, "%f ", chisqr ); 

		// Find mean speed and accel of cars.
		double tot_speed=0;
		double tot_accel=0;
		for( int c=1;c<totcars;c++ ) {
			tot_speed += cars[c].speed;
			tot_accel += cars[c].accel;
		}
		fprintf( statfile, "%f %f ", 
			tot_speed/totcars, tot_accel/totcars );

		fprintf( statfile, "\n" ); 
	} 
 
  if( i==CLOSE ) 
      fclose( statfile ); 
 
} // sumstats() 
//---------------------

void putbigpixel( int x, int y, int color ) {
	vga_setcolor( color );
	vga_drawpixel( x,y );
	vga_drawpixel( x+1,y+1 );
	vga_drawpixel( x+1,y );
	vga_drawpixel( x,y+1 );
}

void putpixel( int x, int y, int color ) {
	vga_setcolor( color );
	vga_drawpixel( x,y );
}
//--------------------

