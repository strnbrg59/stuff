#include "exceptions.hpp"
#include "config.hpp"
#include "mailclient.hpp"
#include "network-utils.hpp"
#include "trace.hpp"
#include "utils.hpp"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <netdb.h>

/** Usage:
 *  mailclient from=<from> to=<to> subject=<subject> msgfile=<filename>
 *  ...and it sends the contents of the file to the addressee.
*/
int main( int argc, char * argv[] )
{
    Trace t( "main()" );
    Config cfg( argc, argv );

    t.Info( "From %s, to %s, subject %s, filename %s.",
            cfg.From().c_str(), cfg.To().c_str(), cfg.Subject().c_str(),
            cfg.MsgFile().c_str() );

    string host_quad_addr( GetHostQuadAddr( cfg.To() ) );

    try
    {
        MailClient client1( cfg.SmtpPort(), host_quad_addr, cfg );
        
        client1.MAIL( cfg.From() );
        client1.RCPT( cfg.To() );
        client1.DATA( string("SUBJECT: ") + cfg.Subject() );

        ifstream message_file( cfg.MsgFile().c_str() );
        if( ! message_file )
        {
            throw ReadErrorException(
                "File not found:|%s|", cfg.MsgFile().c_str() );
        }

        char line_buf[1024];
        while( ! message_file.eof() )
        {
            message_file.getline( line_buf, 1023 );            
            client1.DATA( line_buf );
        }
        client1.DATA( "." );

        client1.Send();
    }
    catch( Exception & e )
    {
        ostringstream o;
        o << e;
        t.FatalError( o.str().c_str() );
    }

    return 0;
}

MailClient::MailClient( short port, string IPAddr, Config const & config )
  : m_config( config )
{
    m_socket = m_network_utils.GetClientSocket( port, IPAddr.c_str(),
                                                config.ConnectTimeOut() );
    // GetClientSocket() throws MiscException if anything goes wrong.
}

MailClient::~MailClient()
{
    close( m_socket );
}

/** Sends the email to its destination.  Checks for correct responses from
 *  server.
*/
void
MailClient::Send()
{
    Trace t( "MailClient::Send()" );

    try
    {
        CheckResponse( "220" ); 
            // First thing that should have come back, after connect().

        m_network_utils.ToPeer( m_socket, 
          string( string("HELO ") + m_sender_domain + string("\r\n") ));
        t.Info( "<<< HELO\n" );
        CheckResponse( "250" );

        m_network_utils.ToPeer( m_socket,
          string( string("MAIL From:<") + m_sender_userid + string("@") + 
                  m_sender_domain + string(">\r\n") ));
        t.Info( "<<< MAIL From\n" );
        CheckResponse( "250" );

        for( multimap<string,string>::const_iterator
            i = m_destinations.begin();
            i != m_destinations.end();
            ++i )
        {
            string rcpt_str = string(string("RCPT To:<") + string(i->first)
                            + string("@") + string(i->second)
                            + string(">\r\n") );
            m_network_utils.ToPeer( m_socket, rcpt_str );

            t.Info( "<<< %s\n", rcpt_str.c_str() );
            CheckResponse( "250" );
        }

        m_network_utils.ToPeer( m_socket, "DATA\r\n" );
        t.Info( "<<< DATA\n" );
        CheckResponse( "354" );

        m_network_utils.ToPeer( m_socket, DATA() );
        t.Info( "<<< %s\n", DATA().c_str() );
        CheckResponse( "250" );

        m_network_utils.ToPeer( m_socket, "QUIT\r\n" );
        t.Info( "<<< QUIT\n" );
        CheckResponse( "221" );
    }
    catch( UnexpectedServerBehaviorException & usbe )
    {
        ostringstream o;
        o << usbe << endl;
        t.FatalError( o.str().c_str() );
    }
}

//
// MAIL(), RCPT(), DATA() : configuring the MailClient object
//

/** Parse out m_sender_userid and m_sender_domain. */
void
MailClient::MAIL( string mail_from )
{
    ParseUseridAndDomain( mail_from, &m_sender_userid, &m_sender_domain );
}


/** Parse out m_dest_userid and m_dest_domain. */
void
MailClient::RCPT( string rcpt_to )
{
    string dest_userid, dest_domain;
    ParseUseridAndDomain( rcpt_to, &dest_userid, &dest_domain );
    m_destinations.insert( pair<string,string>( dest_userid, dest_domain ));
}


/** From a string of the form foo@bar.com, set arg userid to foo and domain
 *  to bar.com.
*/
void
MailClient::ParseUseridAndDomain(
    string addr, string * userid, string * domain )
{
    Trace t( "MailClient::ParseUseridAndDomain()" );

    char * buf = new char[ addr.size() + 1 ];
    strcpy( buf, addr.c_str() );

    char * lasts;
    char * tok = strtok_r( buf, "@", &lasts );
    if( ! tok ) t.FatalError( "Bad format in addr:%s", addr.c_str() );
    *userid = string(tok);

    tok = strtok_r( 0, "\r\n\0", &lasts );
    if( ! tok ) t.FatalError( "Bad format in addr:%s", addr.c_str() );
    *domain = string(tok);

    delete[] buf;
}


/** This function can be called repeatedly; each call adds one element to the
 *  m_data vector.  But it's fine to have \r\n inside the argument too.
*/
void
MailClient::DATA( string line )
{
    m_data.push_back( line );
}


/** Returns all the DATA lines, each ending in \r\n.
 *  It's user's responsibility to make sure the last line is a ^\.
*/
string
MailClient::DATA() const
{
    string retval;

    for( vector<string>::const_iterator i = m_data.begin();
         i != m_data.end();
         ++i )
    {
        retval += *i + "\r\n";
    }

    return retval;
}        


/** Read on the socket and check that message starts with arg
 *  expected_SMTP_code.
 *  If not, throw UnexpectedServerBehaviorException.
*/
void
MailClient::CheckResponse( string expected_SMTP_code )
{
    Trace t( "MailClient::CheckResponse()" );

    string response;
    try
    {
        response =
            m_network_utils.ReadLine( m_socket, m_config.SecondsTimeout());
    }
    catch( EofException & eof )
    {
        ostringstream o;
        o  << eof << endl;
        t.FatalError( o.str().c_str() );
    }
    catch( ReadErrorException & re )
    {
        ostringstream o;
        o  << re << endl;
        t.FatalError( o.str().c_str() );
    }
    catch( TimeoutException & toe )
    {
        ostringstream o;
        o  << toe << endl;
        t.FatalError( o.str().c_str() );
    }

    t.Info( ">>> %s", response.c_str() );
    if( strstr( response.c_str(), expected_SMTP_code.c_str() )
        != response.c_str() )
    {
        throw UnexpectedServerBehaviorException( 
          "Received SMTP response %s.  Was expecting %s.", 
          response.c_str(), expected_SMTP_code.c_str() );
    }
}


/** Arg mail_addr looks like foo@bar.com.  Returns the bar.com part. */
string ParseHostName( string mail_addr )
{
    Trace t("ParseHostName()");
    t.Info( "mail_addr=|%s|", mail_addr.c_str() ); 
    char * buf = new char[mail_addr.size() + 1];
    strcpy( buf, mail_addr.c_str() );
    char * tok = strtok( buf, "@" );
    if( ! tok )
    {
        throw MiscException( "Bad email address -- doesn't contain '@'" );
    }
        
    tok = strtok( 0, "\t\n " );
    assert( tok );
    return string( tok );
}


/** Looks up the address of davis.lbl.gov and returns "128.3.5.43" */
string GetHostQuadAddr( string to_email )
{
    Trace t("GetHostQuadAddr()");

    string host_name;
    try
    {
        host_name = ParseHostName( to_email );
    }
    catch( Exception & e )
    {
        t.FatalError( "%s", e.str() );
    }

    struct hostent * host_ent = gethostbyname( host_name.c_str() );

    if( ! host_ent )
    {
        switch( h_errno )
        {
            case NETDB_INTERNAL : {
                t.Error( "gethostbyname() failed: NETDB_INTERNAL" ); break;
                }
            case HOST_NOT_FOUND : {
                t.Error( "gethostbyname() failed: HOST_NOT_FOUND" ); break;
                }
            case TRY_AGAIN : {
                t.Error( "gethostbyname() failed: TRY_AGAIN" ); break;
                }
            case NO_RECOVERY : {
                t.Error( "gethostbyname() failed: NO_RECOVERY" ); break;
                }
            case NO_DATA : {
                t.Error( "gethostbyname() failed: NO_DATA" ); break;
                }
            default:
                t.Error( "gethostbyname() failed, h_errno=%d", h_errno );
        }
        exit(1);
    }

    short quad[4];
    for( int i=0;i<4;++i )
    {
        quad[i] = (host_ent->h_addr_list)[0][i];
        if( quad[i] < 0 )
        {
            quad[i] += 256;
        }
    }

    char host_addr[16];
    memset( host_addr, 0, 16 );
    sprintf( host_addr, "%d.%d.%d.%d", quad[0], quad[1], quad[2], quad[3] );
    string result( host_addr );
    return result;
}
