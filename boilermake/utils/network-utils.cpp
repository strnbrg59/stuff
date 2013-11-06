#include "exceptions.hpp"
#include "network-utils.hpp"
#include "trace.hpp"

#include <cassert>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

sig_atomic_t g_alarmSignalHandled(0);

NetworkUtils::NetworkUtils()
  : m_readline_count(0)
{
    memset( m_readline_buf, 0, sizeof(m_readline_buf) );
}


/** On error throws MiscException.
 *  Arg addr must be in numbers-and-dots notation, e.g. "127.0.0.1".
 */
int
NetworkUtils::GetClientSocket( short port, char const * addr ) const
{
    Trace t( "GetClientSocket()" );

    sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons( port );
    peer.sin_addr.s_addr = inet_addr( addr );

    int s = socket( AF_INET, SOCK_STREAM, 0 );
    if( s < 0 )
    {
        throw MiscException( "socket() failed in GetClientSocket(), addr=%s",
                             addr );
    }


    // Try to connect, first setting an alarm timer to abandon the effort
    // if the connect blocks for more than a couple seconds.
    signal( SIGALRM, AlarmSignalHandler );
    alarm( 2 );
    int ret = connect( s, (sockaddr *)&peer, sizeof( peer ));
    if( g_alarmSignalHandled == 1 )
    {
        MiscException me( "connect() timed out -- alarm went off." );
        throw me;
    }
    signal( SIGALRM, SIG_DFL );
    alarm( 0 );
    if( ret < 0 )
    {
        MiscException me( "connect failed" );
        throw me;
    }

    return s;
}

int
NetworkUtils::GetServerSocket( short port ) const
{
    Trace t( "GetServerSocket()" );

    sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons( port );
    local.sin_addr.s_addr = htonl( INADDR_ANY );

    int s = socket( AF_INET, SOCK_STREAM, 0 );
    if( s < 0 ) t.FatalError( "socket() failed" );

    const int on = 1;

    if( setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof( on )))
    {
        t.FatalError( "setsockopt() failed" );
    }

    int rc = bind( s, (struct sockaddr *)&local, sizeof( local ) );
    if( rc < 0 ) t.FatalError( "bind() failed" );

    rc = listen( s, 5 );
    if( rc ) t.FatalError( "listen() failed" );

    return s;
}

/** Semantics like recv(), except... 
 *  If times out, returns -1 and sets arg timed_out.
 *  Other negative return values indicate an error either from select() or from
 *  recv(); use perror() or strerror() to find out which it is.  (So don't do any
 *  errno inquiries if the return value is -1 but timed_out is true.)
 *  Returns 0 if EOF received (just as with recv()).
*/
int
NetworkUtils::TimedRead( int s, char * buf, size_t len, 
                         int seconds_timeout, bool * timed_out ) const
{
    fd_set readmask;
    FD_ZERO( &readmask );
    FD_SET( s, &readmask );
    timeval tv;
    tv.tv_sec = seconds_timeout;
    tv.tv_usec = 0;

    int rc = select( s+1, &readmask, 0,0, &tv );
    if( rc < 0 )
    {
        *timed_out = false;
        return rc;
    }
    if( rc == 0 )
    {
        *timed_out = true;
        return -1;
    }
    if( FD_ISSET( s, &readmask ) )
    {
        rc = recv( s, buf, sizeof(buf ), 0 );
        return rc;
    }

    assert( 1 );
    return -1; // Should never get here.
}

/** Reads and returns one "\r\n"-delimited line of text from the socket.
 *  If anything goes wrong, throws an exception:
 *  EofException if the recv() inside TimedRead() reads EOF.
 *  ReadErrorException if the recv() returns a negative number;
 *  TimeoutException if TimedRead() times out.
*/
string
NetworkUtils::ReadLine( int socket, int seconds_timeout )
{
    Trace t( "NetworkUtils::ReadLine()" );

    int const max_line_length = 256; // FIXME: don't limit to 256
    char * bufptr = new char[ max_line_length ];  // where we assemble the line.
    char * const bufx = bufptr;      // placeholder for beginning of bufptr.
    int len = max_line_length;
    char c;
    bool timed_out;

    while( --len )
    {
        if( --m_readline_count <= 0 )
        {  // We've gone through entire m_readline_buf; time to read in
           // some more.
            m_readline_count =
                TimedRead( socket, m_readline_buf, sizeof(m_readline_buf),
                           seconds_timeout, &timed_out );
            if( m_readline_count < 0 )
            {
                if( errno == EINTR )
                {
                    len++; // the while will decrement
                    continue;
                } else
                if( timed_out )
                {
                    throw TimeoutException(
                        "TimedRead() timed out (after %d seconds)",
                        seconds_timeout );
                } else
                {
                    throw ReadErrorException(
                        "recv() returned -1 in TimedRead and not EINTR" );
                }
            } else
            if( m_readline_count == 0 )
            {
               throw EofException( "recv() read EOF" );
            } else
            {   // This is the "normal" outcome; 
                m_readline_buf_position = m_readline_buf;
            }
        }

        c = *m_readline_buf_position++;
        *bufptr++ = c;
        if( c == '\n' )
        {
            *bufptr = '\0';
            string result( bufx );
            delete[] bufx;
            return result;
        }
    }

    // If reached here, means we've hit a line longer than max_line_length.
    // FIXME: we're truncating provisionally, but eventually you should
    // reallocate another bufptr buffer and keep reading until you hit '\n'
    // while at the same time not falling for denial-of-service attacks.
    t.Error( "Found a line longer than %d.  Truncating it...", max_line_length);
    *bufptr = '\0';
    string result( bufx );
    delete[] bufx; // corresponds to "bufptr = new..."
    return result;
}

void
NetworkUtils::ToPeer( int socket, string message )
{
    Trace t( "NetworkUtils::ToPeer()" );

    if( -1 == send( socket, message.c_str(), message.size(), 0 ) )
    {
        // Note message.size(), and not message.size()+1.  The latter sends a
        // '\0' byte which confuses the peer.
        //
        // It's ok to send an arbitrarily large message.  Even if the peer
        // crashes while it has several more IP packets to go, there won't be
        // any SIGPIPE; you get SIGPIPE if you call send() twice after FIN;
        // once you've called send() (here) it's the kernel's business.
        t.FatalSystemError( "send returned -1" );
    }
}

/** Give up controlling terminal, go into background, chdir("/"), etc.
 *  See http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC15, under
 *  "How do I get my program to act like a daemon?".
 *
 *  Redirect stdout and stderr to the log file.
*/
void
NetworkUtils::BecomeDaemon( string log_filename )
{
    Trace t( "NetworkUtils::BecomeDaemon()" );

    // Fork, so ensure child is not a process group leader (and can therefore
    // call setsid).
    int pid = fork();
    if( pid < 0 ) t.FatalSystemError( "First fork() failed." );
    if( pid > 0 ) exit(0);

    if( setsid() == -1 ) t.FatalSystemError( "setsid() failed." );
    pid = fork();
    if( pid < 0 ) t.FatalSystemError( "Second fork() failed." );    
    if( pid > 0 ) exit(0);

    // The only running process now is guaranteed to be neither a process group
    // leader, nor to have a controlling terminal.

    // chdir to '/' so there won't be any problems umounting current filesystem.
    if( chdir( "/" ) == -1 ) t.FatalSystemError( "chdir() failed." );

    umask(0);

    // Close stdin, stdout and stderr.  Then reopen them to what we really want.
    close(0); close(1); close(2);
    if( open( "/dev/null", O_RDONLY ) != 0 ) exit(1);                // stdin
    if( open( log_filename.c_str(), O_WRONLY | O_CREAT | O_APPEND,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ) != 1 ) exit(2);  // stdout
    if( open( log_filename.c_str(), O_WRONLY | O_APPEND, 0 ) != 2 ) exit(3);
         // stderr
    if( fchown( 1, 99, 99 ) == -1 ) exit(4);  // user nobody, group nogroup
    if( fchown( 2, 99, 99 ) == -1 ) exit(5);
}


void
NetworkUtils::ForkGrandchild( int socket )
{
    Trace t("NetworkUtils::ForkGrandchild()");
    int pid = fork();
    if( pid < 0 ) t.FatalSystemError( "fork() failed" );
    if( pid > 0 ) // parent
    {
        close( socket );
        waitpid( pid, 0, 0 ); // we'll soon kill this pid.
        return;
    }
    
    // Fork again; we want to do the work in a grandchild that's
    // an orphan, so we aren't left with a zombie.
    int pid2 = fork();
    if( pid2 < 0 ) t.FatalSystemError( "fork() failed" );
    if( pid2 > 0 ) exit(0);
    
    if( setuid( 99 ) || setgid(99) )  // user nobody, no group
    {                                 // fails if current user isn't root
        t.FatalError( "setuid() or setgid() failed.  Must be root to do that.");
    }
}


void AlarmSignalHandler( int sig )
{
    g_alarmSignalHandled = 1;
}
