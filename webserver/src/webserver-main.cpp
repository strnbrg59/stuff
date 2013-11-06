#include "trace.hpp"
#include "network-utils.hpp"
#include "webserver.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

int
main( int argc, char * argv[] )
{
    // This one line...
    Cmdline cfg( argc, argv );
    // ...somehow causes webserver to die, when it's started by init!?!

    NetworkUtils nu;

    if( cfg.BecomeDaemon() )
    {
        nu.BecomeDaemon( cfg.LogFile() );
    }

    int s = nu.GetServerSocket( cfg.HttpPort() );
    // GetServerSocket() calls FatalError() if anything goes wrong.  So here we
    // can assume s is a valid socket.

    Trace t( "main()" ); // Don't call this before BecomeDaemon(); it'll print
      // to stdout and stderr and the process might have no controlling
      // terminal.

    while(1)
    {
        sockaddr_in client_addr;
        socklen_t sizeof_client_addr = sizeof( client_addr );
        int s1 = accept( s, (sockaddr*)&client_addr, &sizeof_client_addr );
        if( s1 < 0 )
        {
            t.FatalSystemError( "accept() failed" );
        } else
        {
            // FIXME: Support with keepalive connections.

            cfg.Reload();

            // We can run in no-forking mode, for easier debugging.
            if( cfg.BecomeDaemon() )
            {
                int pid = fork();
                if( pid < 0 ) t.FatalSystemError( "fork() failed" );
                if( pid > 0 ) // parent
                {   
                    close( s1 );
                    waitpid( pid, 0, 0 ); // we'll soon kill this pid.
                    continue;
                }

                // Fork again; we want to do the work in a grandchild that's
                // an orphan, so we aren't left with a zombie.
                int pid2 = fork();
                if( pid2 < 0 ) t.FatalSystemError( "fork() failed" );
                if( pid2 > 0 ) exit(0);

                setuid( 99 );  // user nobody
                setgid( 99 );  // nogroup
            }

            // grandchild from here on.
            WebServer w( s1, cfg, client_addr );
            w.Converse();
            close( s1 );
            if( cfg.BecomeDaemon() )
            {
                exit(0);
            }
        }
    }
return 0;
}
