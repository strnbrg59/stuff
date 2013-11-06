//
// Simple mail server
//

#include "argmap.hpp"
#include "cmdline.hpp"
#include "exceptions.hpp"
#include "mailserver.hpp"
#include "myregexp.hpp"
#include "trace.hpp"
#include "utils.hpp"

#include <assert.h>
#include <errno.h>
#include <fstream>
#include <limits.h>  // INT_MAX
#include <map>
#include <netdb.h>   // gethostbyaddr
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>  // flock()
#include <sys/socket.h>
#include <sys/stat.h>  // stat()
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
using std::cout; using std::cerr; using std::endl;

MailServer::MailServer( int socket, Cmdline const & cmdline,
                        sockaddr_in const & client_sockaddr )
  : m_socket( socket ),
    m_cmdline( cmdline ),
    m_client_sockaddr( client_sockaddr )
{
    // Block all signals.  (The parent process doesn't block any signals; it
    // can die any time as its only duty is to fork off grandchildren to do the
    // real work.  The grandchildren, on the other hand, must not be interrupted
    // until they (soon enough) exit on their own.
    sigset_t sig_mask;
    sigfillset( &sig_mask );
    sigprocmask( SIG_BLOCK, &sig_mask, 0 );

    gethostname( m_host_name, sizeof(m_host_name) );
    getdomainname( m_domain_name, sizeof(m_domain_name) );
}

/** Carry on a conversation with the mail client */
void
MailServer::Converse()
{
    Trace t( "MailServer::Converse()" );

    Greeting();

    string cmd;

    try
    {
        while( 1 )  // break out on exceptions.
        {
            SMTPCommand cmd = m_network_utils.ReadLine(
                m_socket, m_cmdline.SecondsTimeout());
            m_smtp_commands.push_back( cmd.key + " " + cmd.value );
            t.Info( "New SMTPCommand: %s %s\n",
                    cmd.key.c_str(), cmd.value.c_str() );
            RespondToClient( cmd );
        }
    }

    // Exceptions that might come from ReadLine().
    catch( EofException & eof )
    {
        t.Info( "Received EOF.  Normal, after QUIT.\n" );
    }
    catch( ReadErrorException & re )
    {
        std::ostringstream o;
        o  << re << endl;
        t.Warning( o.str().c_str() );
    }
    catch( TimeoutException & toe )
    {
        std::ostringstream o;
        o  << toe << endl;
        t.Warning( o.str().c_str() );
    }
/*
    catch( DataModeExceptionException & dme )
        // while responding to a DATA command.
    {
        cerr << dme << endl;
    }
*/
    catch( ... )
    {
        assert( 1 );
    }
}

/** Goes through all the recipients and calls RecordPerRCPT() for each one. */
void
MailServer::Record() const
{
    Trace t( "MailServer::Record()" );

    for( vector<string>::const_iterator userid =  m_RCPT_userids.begin();
                                        userid != m_RCPT_userids.end();
                                        ++userid )
    {
       t.Info( "Record( %s )\n", userid->c_str() );
       RecordPerRCPT( *userid );
    }
}

/** Save the mail to a file.  To ensure only one child process writes to the
 *  file at a time we use flock().
 *  If the file's size is over Cmdline::m_max_mailbox_size, then save just the
 *  first few lines of the current message.
 *  This function gets called once for each RCPT command received.
*/
void
MailServer::RecordPerRCPT( string rcpt_userid ) const
{
    Trace t("MailServer::Record()");

    FILE * mailbox;
    string mailbox_filename;
    struct stat mystat;

    //
    // Check that the intended recipient is in the m_cmdline.Users() list, and
    // that he has a valid Mailbox file.  If anything goes wrong, use the
    // mailbox of m_cmdline.DefaultUser().
    //
    char * users_buf = new char[ m_cmdline.Users().size() + 1 ];
    strcpy( users_buf, m_cmdline.Users().c_str() );
    char * lasts;
    char * tok = strtok_r( users_buf, ",\0", &lasts );
    while( tok && ( tolower(string(tok)) != tolower(rcpt_userid) ) )
    {
        tok = strtok_r( 0, ",\0", &lasts );
    }
    if( ! tok ) // means we didn't find rcpt_userid among m_cmdline.Users()
    {
        t.Warning( "RCPT userid %s is not among this server's recognized users"
                   " (see \"users\" parameter in mailserver.rc).\n",
                   rcpt_userid.c_str() );
        mailbox_filename = "/home/" + m_cmdline.DefaultUser() + "/"
                         + m_cmdline.Mailbox();
    } else // rcpt_userid is OK, check that he has a mailbox file.
    {
        mailbox_filename = "/home/" + rcpt_userid + "/" + m_cmdline.Mailbox();
        int stat_retval = stat( mailbox_filename.c_str(), &mystat );
        if( stat_retval != 0 )
        {
            t.SystemError( "Mailbox file %s does not exist\n",
                           mailbox_filename.c_str() );
            mailbox_filename =
              "/home/" + m_cmdline.DefaultUser() + "/" + m_cmdline.Mailbox();
        }
    }

    mailbox = fopen( mailbox_filename.c_str(), "a" );
    if( ! mailbox ) t.FatalSystemError( "fopen() failed\n" );
    // Only reason this could have failed is if we're not running mailserver as
    // root, and mailbox isn't writeable by us.

    //
    // If got here, we have a valid mailbox to write to.  Lock it and write the
    // message.
    //
    if( flock( fileno(mailbox), LOCK_EX ) )
    {
        t.FatalSystemError( "flock() failed\n" );
    }

    // If we've reached the maximum mailbox size, start truncating the messages.
    fstat( fileno(mailbox), &mystat );
    int max_lines_to_write = INT_MAX;
    if( mystat.st_size > m_cmdline.MaxMailboxSize()*1024 )
    {
        max_lines_to_write = 40;
    }
    
    time_t tt;
    time( &tt );
    fprintf( mailbox, "\nFrom MAILER-DAEMON %s", ctime(&tt) );

    // Get sender's true inet address.
    char * dotted = inet_ntoa( m_client_sockaddr.sin_addr );
    string peer_name;
    struct hostent * host_ent =
        gethostbyaddr( (char *)&m_client_sockaddr.sin_addr,
                       sizeof(m_client_sockaddr.sin_addr), 
                                                AF_INET );
    if( ! host_ent )
    {
       t.Warning( "gethostbyaddr() failed" );
       peer_name = "(unknown host name)";
    } else
    {
       peer_name = host_ent->h_name;
    }
    fprintf( mailbox, "Received: from %s [%s]\n", peer_name.c_str(), dotted );

    int i_line = 0;
    for( vector<string>::const_iterator i = m_data_lines.begin();
         (i != m_data_lines.end()) && (i_line < max_lines_to_write);
         ++ i, ++ i_line )
    {
        int j=0;
        while( (*i)[j] == '\t' ) j++;
        fprintf( mailbox, "%s", i->c_str() + j );
    }

    for( vector<string>::const_iterator i = m_smtp_commands.begin();
         i != m_smtp_commands.end();
         ++ i )
    {
        fprintf( mailbox, "%s\n", i->c_str() );
    }

    if( mystat.st_size > m_cmdline.MaxMailboxSize()*1024 )
    {
        fprintf( mailbox, "***WARNING: mailbox over max size (%dKB), "
                 " this message truncated\n", m_cmdline.MaxMailboxSize() );
    }

    if( flock(fileno(mailbox), LOCK_UN ))
        t.FatalSystemError( "flock() failed to unlock.\n" );

    fclose( mailbox );
}

void
MailServer::Greeting()
{
    m_network_utils.ToPeer( m_socket,
      string( string("220 ") + m_host_name + string(".") + m_domain_name + 
              string(" mailserver ready\r\n" )));
}


void
MailServer::RespondToClient( SMTPCommand cmd )
{
    Trace t( "MailServer::RespondToClient()" );

    // Set up table of pointers to functions that handle the various commands
    // we might get from the client.
    typedef void (MailServer::*SMTPCommandHandler)( SMTPCommand );
    std::map< string, SMTPCommandHandler > command_handler_map;
    command_handler_map[ "HELO" ] = &MailServer::HandleHELO;
    command_handler_map[ "MAIL" ] = &MailServer::HandleMAIL;
    command_handler_map[ "RCPT" ] = &MailServer::HandleRCPT;
    command_handler_map[ "DATA" ] = &MailServer::HandleDATA;
    command_handler_map[ "QUIT" ] = &MailServer::HandleQUIT;
    command_handler_map[ "NOOP" ] = &MailServer::HandleNOOP;

    command_handler_map[ "SEND" ] 
      = command_handler_map[ "EHLO" ]
      = command_handler_map[ "SOML" ]
      = command_handler_map[ "SAML" ]
      = command_handler_map[ "VRFY" ]
      = command_handler_map[ "EXPN" ]
      = command_handler_map[ "HELP" ]
      = command_handler_map[ "TURN" ]
      = command_handler_map[ "RSET" ]
                                  = &MailServer::HandleUNIMPLEMENTED;

    std::map<string, SMTPCommandHandler>::iterator i
      = command_handler_map.find( toupper(cmd.key) );
    if( i == command_handler_map.end() )
    {
        t.Error( "Unknown SMTP command: %s", cmd.key.c_str() );
        HandleUNIMPLEMENTED( cmd );
    } else
    {
        (this->*(i->second))( cmd ); // calls HandleXXX function.
    }
}

void
MailServer::HandleHELO( SMTPCommand cmd )
{
    Trace t( "MailServer::HandleHELO()" );
    m_network_utils.ToPeer( m_socket, "250 OK (HELO)\r\n" );
}

void
MailServer::HandleMAIL( SMTPCommand cmd )
{
    Trace t( "MailServer::HandleMAIL()" );
    m_network_utils.ToPeer( m_socket, "250 OK (MAIL)\r\n" );
}

/** Handles any number of RCPT lines. */
void
MailServer::HandleRCPT( SMTPCommand cmd )
{
    Trace t( "MailServer::HandleRCPT()" );
    m_network_utils.ToPeer( m_socket, "250 OK (RCPT)\r\n" );


    // Parse out the intended recipient's userid and place on m_RCPT_userids.
    // The format of cmd.value should be "To:<user@domain>".
    // If the format isn't what we expect (so we can't figure out who the
    // intended recipient is), then we use m_cmdline.DefaultUser().
    // Later, in MailServer::Record(), we may find that the indended recipient
    // doesn't have a Mailbox, or he's not on the m_cmdline.Users() list.  In
    // that case, once again the mail goes to m_cmdline.DefaultUser()'s Mailbox.
    char * cmd_copy = new char[ cmd.value.size()+1 ];
    strcpy( cmd_copy, cmd.value.c_str() );
    char * lasts;
    char * tok = strtok_r( cmd_copy, "<", &lasts );
    if( ! tok )
    {
        t.Warning( "Invalid format in RCPT command: %s\n", cmd.value.c_str() );
        m_RCPT_userids.push_back( m_cmdline.DefaultUser() );
    } else
    {
        tok = strtok_r( 0, "@", &lasts );
        if( ! tok )
        {
            t.Warning( "Invalid format in RCPT command: %s\n",
                       cmd.value.c_str() );
            m_RCPT_userids.push_back( m_cmdline.DefaultUser() );
        } else
        {
            m_RCPT_userids.push_back( string(tok) );
        }
    }
    delete[] cmd_copy;
}

void
MailServer::HandleNOOP( SMTPCommand cmd )
{
}

/** Receives what comes after ^DATA and up to ^\..  
 *  Shuts off after Cmdline::MaxMessageSize() kilobytes.
*/
void
MailServer::HandleDATA( SMTPCommand cmd )
{
    Trace t( "MailServer::HandleDATA()" );
    t.Info( "\n" );

    m_network_utils.ToPeer( m_socket,
        "354 Start mail input; end with <CRLF>.<CRLF>\r\n" );
    string one_line;
    long bytes_read = 0;
    try
    {
        int i=0;
        while( ((one_line[0] != '.') || (isalnum(one_line[1])))
        &&     ( bytes_read/1024 < m_cmdline.MaxMessageSize() ))
        {
            one_line = m_network_utils.ReadLine( m_socket,
                                                 m_cmdline.SecondsTimeout());
            m_data_lines.push_back( one_line );
            if( ++i < 20 ) t.Info( one_line.c_str() );
            if( i == 20 ) t.Info( "-- subsequent lines suppressed --\n" );
            bytes_read += one_line.size();
        }
        if( bytes_read/1024 >= m_cmdline.MaxMessageSize() )
        {
            t.Error( "Maximum message size (%d kilobytes) exceeded",
                     m_cmdline.MaxMessageSize() );
            m_data_lines.push_back(
                "***ERROR*** Maximum message size exceeded\r\n" );
            m_network_utils.ToPeer( m_socket,
                                    "552 exceeded maximum message size\r\n" );
        }
        else
        {
            m_data_lines.pop_back(); // discard the '^\.' line
            m_network_utils.ToPeer( m_socket,
                                    "250 Message accepted for delivery\r\n" );
        }
    }
    // Exceptions that might come from ReadLine().
    catch( EofException & eof )
    {
        std::ostringstream o;
        o << eof;
        t.Error( o.str().c_str() );
    }
    catch( ReadErrorException & re )
    {
        std::ostringstream o;
        o << re;
        t.Error( o.str().c_str() );
    }
    catch( TimeoutException & tmt )
    {
        std::ostringstream o;
        o << tmt;
        t.Error( o.str().c_str() );
    }
}

void
MailServer::HandleQUIT( SMTPCommand cmd )
{
    Trace t( "MailServer::HandleQUIT()" );
    t.Info( "\n" );

    m_network_utils.ToPeer( 
      m_socket, string( string("221 ") + m_host_name + string(".")
                        + m_domain_name + 
                        string(" Service closing transmission channel\r\n" )));
}

void
MailServer::HandleUNIMPLEMENTED( SMTPCommand cmd )
{
    Trace t( "MailServer::HandleUNIMPLEMENTED()" );
    m_network_utils.ToPeer( m_socket, string("530 " + cmd.key
                                             + " unimplemented\r\n") );
    t.Warning( "Received unimplemented command %s %s\n",
                cmd.key.c_str(), cmd.value.c_str() );
}

/** Break up one line into the 4-letter command (e.g. MAIL, RCPT, HELO) and
 *  the rest.
*/
SMTPCommand::SMTPCommand( string one_line )
{
    Trace t( "SMTPCommand:SMTPCommand()" );

    char * str = new char[ one_line.size() + 1 ];
    strcpy( str, one_line.c_str() );
    char * lasts;
    
    char * tok = strtok_r( str, " \r\n\t\0", &lasts );
    if( ! tok )
    {
        t.Error("Tried to construct SMTPCommand object from nothing: %s",
                one_line.c_str());
        key = "";
        value = "";
    } else
    {
        key = string( tok );
        tok = strtok_r( 0, " \t\n\r\0", &lasts );
        if( ! tok )  // may be ok; just a key but no extra info.
        {
            value = "";
        } else
        {
            value = string( tok );
        }
    }
}
