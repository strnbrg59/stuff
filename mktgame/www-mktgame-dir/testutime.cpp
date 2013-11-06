#include <iostream.h>

main() {
	for( int i=0;i<10;i++ ) {
		usleep(500000);
		cout << i << '\n';
	}
	
}