#include "network-utils.hpp"
#include "trace.hpp"
#include "cmdline.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
using std::cout;

int
main( int argc, char * argv[] )
{
    Trace t("server");
    Cmdline cmdline(argc, argv);
    NetworkUtils netutils;
    int sock = netutils.GetServerSocket( cmdline.Port() );
    while( 1 )
    {
        sockaddr_in client_addr;
        socklen_t sizeof_client_addr = sizeof( client_addr );
        int sock1 =
            accept( sock, (sockaddr*)&client_addr, &sizeof_client_addr);
        t.Info("server: sock1=%d", sock1);
        if( sock1 < 0 )
        {
            t.FatalError( "accept() failed" );
        }

        t.Info("read from client: %s", netutils.ReadLine(sock1, 2).c_str());
        exit(0);
    }
}
