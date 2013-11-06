#include "exceptions.hpp"
#include "config.hpp"
#include "webclient.hpp"
#include "network-utils.hpp"
#include "trace.hpp"
#include "utils.hpp"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <strstream>
#include <unistd.h>

int
main( int argc, char * argv[] )
{
    Trace t( "main()" );
    Config cfg( argc, argv );

    WebClient client( cfg.HttpPort(), "127.0.0.1", cfg );
    client.Request( "GET /ngrams/ngrams.html" );

    try
    {            
        client.Send( cfg.HttpPort(), "127.0.0.1" );
    }
    catch( Exception & e )
    {
        ostrstream o;
        o << e << endl;
        t.FatalError( o.str() );
    }
return 0;
}

WebClient::WebClient( short port, string IPAddr, Config const & config )
  : m_config( config )
{
    m_socket = m_network_utils.GetClientSocket( port, IPAddr.c_str() );
      // GetClientSocket() calls FatalError() if anything goes wrong.  So we can 
      // assume m_socket is a valid socket.
}

WebClient::~WebClient()
{
    close( m_socket );
}

void
WebClient::Request( string str )
{
    m_request = str;
}

/** Sends the request to its destination.
 *  Postpends "HTTP/1.0" and maybe more stuff.
*/
void
WebClient::Send( short port, string hostIP )
{
    Trace t( "WebClient::Send()" );

    string msg = m_request 
               + string(" HTTP/1.0\r\n" )
               + string("Connection: Keep-Alive\r\n")
               + string("User-Agent: Mozilla/4.73 [en] (X11; U; Linux 2.2.16 i686)\r\n")
               + string("Host: localhost:8080\r\n")
               + string("Accept: image/gif, image/x-xbitmap, image/jpeg,"
                        " image/pjpeg, image/png, */*\r\n")
               + string("Accept-Encoding: gzip\r\n")
               + string("Accept-Language: en\r\n")
               + string("Accept-Charset: iso-8859-1,*,utf-8\r\n")
               + string("\r\n");

    m_network_utils.ToPeer( m_socket, msg );

    char read_buf[64];
    int rc = read( m_socket, read_buf, sizeof(read_buf) );
    while( rc && !strstr(tolower(string(read_buf,0,rc)).c_str(), "</html>") )
    {
        write( 2, read_buf, rc );
        rc = read( m_socket, read_buf, sizeof(read_buf) );
    }
    close( m_socket );
}
