#include <sys/time.h>
#include <iostream.h>
int gettimeofday( struct timeval*, struct timezone* );

main() {

	struct timeval tv;
	gettimeofday( &tv, NULL );
	cout << tv.tv_sec << "." << tv.tv_usec << "\n";

}