#include "trace.hpp"
#include "network-utils.hpp"
#include "mailserver.hpp"

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

int main( int argc, char * argv[] )
{
    Trace t( "main()" );
    Cmdline cfg( argc, argv );

    NetworkUtils nu;
    int s = nu.GetServerSocket( cfg.SmtpPort() );
    // GetServerSocket() calls FatalError() if anything goes wrong.  So here we
    // can assume s is a valid socket.

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
            cfg.Reload();

            int pid = fork();
            if( pid < 0 ) t.FatalSystemError( "fork() failed" );
            if( pid > 0 ) // parent
            {   
                waitpid( pid, 0, 0 ); // we'll soon kill this pid.
                continue;
            }

            // Fork again; we want to do the work in a grandchild that's an orphan,
            // so we aren't left with a zombie.
            int pid2 = fork();
            if( pid2 < 0 ) t.FatalSystemError( "fork() failed" );
            if( pid2 > 0 ) exit(0);

            // grandchild from here on.
            MailServer m( s1, cfg, client_addr );
            m.Converse();
            close( s1 );

            m.Record();

            exit(0);
        }
    }

    return 0;
}
