// Crash while client is still sending data.

#include "config.hpp"
#include "network-utils.hpp"
#include "trace.hpp"

#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

main( int argc, char * argv[] )
{
    Trace t( "main()" );
    Config cfg( argc, argv );
    NetworkUtils nu;
    int s = nu.GetServerSocket( 8080 );
    int s1 = accept( s, 0, 0 );

    string one_line;
    while( one_line != "\r\n" )
    {
        one_line = nu.ReadLine( s1, 4 );
        cerr << one_line;
    }

    cerr << "Found CRLF\n";

    nu.ToPeer( s1, "HTTP/1.0 200 OK\r\n" );
    nu.ToPeer( s1, "\r\n" );
    nu.ToPeer( s1, "<HTML>\r\n" );
    nu.ToPeer( s1, "<H1> Hello, world </H1>\r\n" );
    nu.ToPeer( s1, "</HTML>\r\n" );

    close(s1);
}




