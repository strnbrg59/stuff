#include "cmdline.hpp"
#include "trace.hpp"
#include "network-utils.hpp"
#include <iostream>
#include <unistd.h>
#include <cassert>
#include <vector>
using std::cout; using std::cerr; using std::endl;

int main( int argc, char * argv[] )
{
    Trace t("clientest");
    Cmdline cmdline( argc, argv );
    NetworkUtils netutils;
    int socket;
    try
    {
        socket = netutils.GetClientSocket( cmdline.Port(), "127.0.0.1" );
    }
    catch( ... )
    {
        t.FatalError( "Failure in NetworkUtils.GetClientSocket()" );
    }

    t.Info("socket=%d", socket);

    while(1)
    {
        netutils.ToPeer( socket, "hello world\n" );
        sleep(1);
    }
}
