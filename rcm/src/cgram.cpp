#include <math.h>
#include "rcm.h"

/** cross-correlagram of x and y.
 *  maxlag's default is 20.
*/
Md cgram( const Vd& x, const Vd& y, int maxlag ) 
{

	if( maxlag > x.size()/5 )
		maxlag = x.size()/5;

	int lag, i;
	double sx=0,sy=0,ssx=0,ssy=0,sxy=0;
  
	Md result(2*maxlag+1,2);

	// negative lags
	for(lag=-maxlag; lag<=-1; lag++) {
		int m = x.size()+lag;

		for( i=0;i<m; i++) {
			sx += x[i];
			sy += y[i-lag];
			sxy+= x[i] * y[i-lag];
			ssx+= x[i] * x[i];
			ssy+= y[i-lag] * y[i-lag];
		}
	
		if( (ssx-sx*sx/m)*(ssy-sy*sy/m ) == 0 )
			result[maxlag+lag][0] = 0;
		else
			result[maxlag+lag][0] = (sxy - sx*sy/m)
			/ sqrt( (ssx-sx*sx/m)*(ssy-sy*sy/m) );

		result[maxlag+lag][1] = lag;
		sx=sy=sxy=ssx=ssy=0;
	} // for lag 

	
	// positive lags
	for( lag=0; lag<=maxlag; lag++) {
		int m = x.size() - lag;

		for( i=0;i<m; i++) {
			sx += x[i+lag];
			sy += y[i];
			sxy+= x[i+lag] * y[i];
			ssx+= x[i+lag] * x[i+lag];
			ssy+= y[i] * y[i];
		}

		if( (ssx-sx*sx/m)*(ssy-sy*sy/m )  == 0 )
			result[maxlag+lag][0] = 0;
		else
			result[maxlag+lag][0] = (sxy - sx*sy/m )
			/ sqrt( (ssx-sx*sx/m)*(ssy-sy*sy/m ) );
	
		result[maxlag+lag][1] = lag;
		sx=sy=sxy=ssx=ssy=0;
	} // for lag 

	return result;
} // cgram()
