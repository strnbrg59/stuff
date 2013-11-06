// Print out all the environment variables, in a neat format.  Call
// system("env") and trap the result in a pseudo-terminal.  Use the
// expect library.

#include <stdlib.h>
#include <expect.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>
#include <string.h>
#include <String.h>

char* Tcl_ErrnoMsg(int);

main() {

	FILE* fp = exp_popen( "/usr/bin/env" );

	const linelen=1200;
	char incoming_line[linelen+1];

	char* rtrn = fgets( incoming_line, linelen, fp );
	while( rtrn ) {
		cout << incoming_line << "<BR>\n";
		rtrn = fgets( incoming_line, linelen, fp );
	}

} // main()
//------------------------


