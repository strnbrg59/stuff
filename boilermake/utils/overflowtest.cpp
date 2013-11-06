#include "network-utils.hpp"
#include "trace.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
using std::cout; using std::cerr; using std::endl;

int main( int argc, char * * argv)
{
    Trace t("main()");

    if( argc != 3 )
    {
        t.FatalError( "Usage: %s port ipaddress", argv[0] );
    }

    NetworkUtils netutils;
    short port( atoi( argv[1] ) );
    char * ipaddress = argv[2];
    int sock = netutils.GetClientSocket( port, ipaddress );
    
    int const bigbufsize( 1000000 );
    char * bigbuf = new char[bigbufsize];
    for( int i=0;i<bigbufsize;++i )
    {
        bigbuf[i] = char(88);
    }

    netutils.ToPeer( sock, string(bigbuf) );

    delete [] bigbuf;    
    return 0;
}
