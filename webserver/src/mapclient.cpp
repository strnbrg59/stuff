#include "exceptions.hpp"
#include "cmdline.hpp"
#include "webclient.hpp"
#include "network-utils.hpp"
#include "trace.hpp"
#include "utils.hpp"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <unistd.h>

int
main( int argc, char * argv[] )
{
    Trace t( "main()" );
    Cmdline cfg( argc, argv );
    std::string myGoogleMapsKey("ABQIAAAA_UDgKGWyYuE0YHl4H0WTHhT4gZ6bPW9nPZnWiuOURMVI7YQk4xTl22ZUpiHksJnJe-8xrfEO1aXE9g");
    std::string query("q=33205+Lark+Way,+Fremont,+CA");
    std::string request( std::string("GET /maps/geo?")
                       + query
                       + std::string("&output=csv")
                       + std::string("&key=")
                       + myGoogleMapsKey );

    std::string googleMapsAddr("216.239.53.104");
    WebClient client( 80, googleMapsAddr.c_str(), cfg );
    client.Request( request.c_str() );

    try
    {            
        client.Send( 80, googleMapsAddr.c_str() );
    }
    catch( Exception & e )
    {
        std::ostringstream o;
        o << e << '\n';
        t.FatalError( o.str().c_str() );
    }
return 0;
}

WebClient::WebClient( short port, string IPAddr, Cmdline const & config )
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

    string msg = m_request + string("\r\n");

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
