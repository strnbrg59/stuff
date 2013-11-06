//
// Simple web server
//

#include "exceptions.hpp"
#include "webserver.hpp"
#include "trace.hpp"
#include "utils.hpp"

#include <errno.h>
#include <netdb.h>   // gethostbyaddr
#include <pwd.h>
#include <cstdio>
#include <cassert>
#include <string>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/file.h>  // O_RDONLY, open()
#include <sys/socket.h>
#include <sys/stat.h>  // stat()
#include <sys/types.h>
#include <sys/wait.h>


WebServer::WebServer( int socket, Cmdline const & config,
                        sockaddr_in const & client_sockaddr )
  : m_socket( socket ),
    m_config( config ),
    m_client_sockaddr( client_sockaddr )
{
    gethostname( m_host_name, sizeof(m_host_name) );
    getdomainname( m_domain_name, sizeof(m_domain_name) );
}

/** Carry on a conversation with the web client.  But for now, since we ignore
 *  the  keep-alive directive, the conversation consists of just one request
 *  and one response.
*/
void
WebServer::Converse()
{
    Trace t( "WebServer::Converse()" );

    time_t tloc, tdummy;
    char buf[48];
    tloc = time( &tdummy );
    ctime_r( &tloc, buf );
    buf[ strlen(buf)-1 ] = 0;
    t.Info("------- begin (%s) ---------", buf );

    // Identify the peer.  Return value from inet_ntoa is in a static buffer
    // that soon gets overwritten.
    char * volatile_client_ip = inet_ntoa( m_client_sockaddr.sin_addr );
    if( volatile_client_ip )
    {    
        m_client_ip = std::string( volatile_client_ip );
    } else
    {
        t.Error( "inet_ntoa() returned NULL" );
        m_client_ip = "0.0.0.0"; // FIXME: Exclude 0.0.0.0 at all times.
    }
    std::string peer_name;
    struct hostent * host_ent =
        gethostbyaddr( (char *)&m_client_sockaddr.sin_addr,
                       sizeof(m_client_sockaddr.sin_addr), AF_INET );
    if( ! host_ent )
    {
       t.Warning( "gethostbyaddr() failed" );
       peer_name = "(unknown host name)";
    } else
    {
       peer_name = host_ent->h_name;
    }
    t.Info( "Peer: from %s [%s]\n", peer_name.c_str(), m_client_ip.c_str() );

    std::string cmd;

    try
    {
        HTTPCommand cmd = m_network_utils.ReadLine( m_socket, 
                                                    m_config.SecondsTimeout());
        t.Info( "New HTTPCommand: %s %s\n",
                cmd.key.c_str(), cmd.value.c_str() );
        RespondToClient( cmd );
    }

    // Exceptions that might come from ReadLine().
    catch( EofException & eof )
    {
        t.Info( "Received EOF.  Normal, after QUIT." );
    }
    catch( ReadErrorException & re )
    {
        std::ostringstream o;
        o  << re;
        t.Warning( o.str().c_str() );
    }
    catch( TimeoutException & toe )
    {
        std::ostringstream o;
        o  << toe;
        t.Warning( o.str().c_str() );
    }
    catch( ... )
    {
        assert( 1 );
    }

    t.Info("------------------------- end --------------------");
}

void
WebServer::RespondToClient( HTTPCommand cmd )
{
    Trace t( "WebServer::RespondToClient()" );

    // Set up table of pointers to functions that handle the various commands
    // we might get from the client.
    typedef void (WebServer::*HTTPCommandHandler)( HTTPCommand );
    std::map< std::string, HTTPCommandHandler > command_handler_map;
    command_handler_map[ "GET" ] = &WebServer::HandleGET;
    command_handler_map[ "PUT" ] = &WebServer::HandleUNIMPLEMENTED;
    command_handler_map[ "POST" ] = &WebServer::HandlePOST;
                                  

    std::map<std::string, HTTPCommandHandler>::iterator i
      = command_handler_map.find( toupper(cmd.key) );
    if( i == command_handler_map.end() )
    {
        t.Error( "Unknown HTTP command: %s", cmd.key.c_str() );
        HandleUNIMPLEMENTED( cmd );
    } else
    {
        (this->*(i->second))( cmd ); // calls HandleXXX function.
    }
}

/** Reads env(CONTENT_LENGTH) bytes from stdin. */
void
WebServer::HandlePOST( HTTPCommand cmd )
{
   // Not ready for prime time.  First, make it work.  Then, make sure it
   // doesn't compromise security.
   return;

   Trace t( "WebServer::HandlePOST()" );
   unsigned const MAXLEN=80;
   char input[MAXLEN], data[MAXLEN];
   char* lenstr = getenv("CONTENT_LENGTH");
   long len;
   if( (!lenstr) || (sscanf(lenstr,"%ld",&len)!=1) || (len>MAXLEN))
   {
      t.Error( "No CONTENT_LENGTH env var, or other error in invocation." );
      return;
   }

   fgets(input, len+1, stdin);
   FILE* outfile = fopen("/tmp/postin.txt","w");
   fputs(input,outfile);
   fclose(outfile);
//   puts(input);   
}


/** Reads options up to a line that matches ^\r\n$ */
void
WebServer::HandleGET( HTTPCommand cmd )
{
    Trace t( "WebServer::HandleGET()" );
 
    // Read up to a line that matches ^\r\n$, throwing it all away.
    try
    {
        std::string one_line = "";
        while( one_line != "\r\n" )
        {
            one_line = m_network_utils.ReadLine(
                            m_socket, m_config.SecondsTimeout() );
            t.Info( one_line.c_str() );
        }
    }
    // Exceptions that might come from ReadLine().  Since they all imply we
    // won't be able to write to this socket, we should return from this
    // function.
    catch( EofException & eof )
    {
        t.Error( "Received EOF." );
        return;
    }
    catch( ReadErrorException & re )
    {
        std::ostringstream o;
        o  << re;
        t.Error( o.str().c_str() );
        return;
    }
    catch( TimeoutException & toe )
    {
        std::ostringstream o;
        o  << toe;
        t.Error( o.str().c_str() );
        return;
    }

    // Respond to the GET.  Its format is "GET <path> HTML/<version>".
    // So grab <path>.
    // If the next thing is cgi-bin, then we try to execute the thing after
    // that.

    char * lasts_1;
    char get_args[1024];
    // FIXME: It's standard to limit this, but what's the standard?
    get_args[ sizeof(get_args)-1 ] = 0;
    strncpy( get_args, cmd.value.c_str(), sizeof(get_args)-1 );
    char * url =  // the part before "HTTP/1.1"
        strtok_r( get_args, " \t\r\n\0", &lasts_1 );
    if( ! url )
    {
        t.Error( "Bad GET command: %s %s", cmd.key.c_str(), cmd.value.c_str() );
        ClientError( "Bad URL format" );
        return;
    }
    if( strstr(url, "..") )
    {   // This would be a huge security hole.
        // FIXME: It still is, if a user made a link, under his public_html,
        // to, say, "/".
        t.Warning( "URL contains '..' -- rejected!" );
        ClientError( "Sorry, you can't have that file.  Bye." );
        return;
    }

    std::string full_path;
    char * lasts_2;

    std::string url_str( url );
    t.Info("*** url_str=|%s|\n", url_str.c_str() );
    char * first_tok;
    if( url[0] == '/' )
    {
        first_tok = strtok_r( url+1, "/ \t\r\n", &lasts_2 );
    } else
    {
        first_tok = strtok_r( url, "/ \t\r\n", &lasts_2 );
    }
    assert( first_tok ); // hard to see how it could be null, if url was not.

    if( first_tok[0] == '~')
    {   // URL is "absolute", and first_tok+1 should be a userid.
        char * userid = first_tok + 1;
        if( ! userid )
        {
            t.Error( "Expected to find a userid after ~, found null." );
            ClientError( "Bad URL format" );
            return;
        }
        struct passwd * pwinfo = getpwnam( userid );
        if( ! pwinfo )
        {
            t.Error( "getpwnam() returned null on alleged username|%s|",userid);
            ClientError( std::string(userid) + std::string(" is not a valid username."));
            return;
        }
        full_path = std::string( pwinfo->pw_dir)
                  + std::string( "/" ) + m_config.PublicHtml()
                  + std::string( "/" );

        if( lasts_2 && lasts_2[0] )  // if there's something beyond ~userid.
        {
            full_path += std::string( lasts_2 );
        }
    } 

    // Deal with CGI.
    if( strstr( full_path.c_str(), "cgi-bin" ) )
    {
        // Grab the part before the '?'.  Put the part after the '?' into the
        // environment variable QUERY_STRING (after converting all non-alnum
        // chars to pseudo-hex).
        char full_path_buf[1024];
        full_path_buf[sizeof(full_path_buf)-1] = 0;
        strncpy( full_path_buf, full_path.c_str(), sizeof(full_path_buf)-1 );
        char * lasts;
        char * progname = strtok_r( full_path_buf, "?\0", &lasts );
        char * query_string = lasts;

        int pid = fork();
        if( pid < 0 ) t.FatalSystemError( "Fork() failed." );
        if( pid == 0 ) // child
        {
            if( query_string )
            {
                setenv( "QUERY_STRING", query_string, 1 );
                setenv( "CLIENT_IP", m_client_ip.c_str(), 1 );
                setenv( "LD_LIBRARY_PATH", "/home/strnbrg/usr/local/lib", 1 );
            }
            if( dup2( m_socket, STDOUT_FILENO ) == -1 )
            {
                t.FatalSystemError( "dup2() failed on STDOUT_FILENO." );
            }
            if( dup2( m_socket, STDERR_FILENO ) == -1 )
            {
                t.FatalSystemError( "dup2() failed on STDERR_FILENO." );
            }
            execl( progname, 0 );
            char * perror_msg = strerror( errno );
            ClientError( perror_msg );
            t.FatalError( "execl(%s) failed, perror()=%s.",
                          progname, perror_msg );
        }

        // parent
        waitpid( pid,0,0 );
    }
    else
    {
        // What kind of file is this?  If it's a directory, look for index.html.
        struct stat my_stat;
        if( stat( full_path.c_str(), &my_stat ) != 0 )
        {
            t.SystemError( "stat() returned nonzero on path--%s--",
                           full_path.c_str() );
            ClientError( std::string("bad path: " ) + full_path );
            return;
        }
    
        if( my_stat.st_mode & S_IFDIR )
        {
            full_path += std::string("/index.html");
        }
    
        // Check for <ALLOW> or <DENY> tags in the <HEAD> section.
        if( AccessDenied( full_path ) )
        {
            ClientError( "Permission to view page is denied.  Sorry." );
            return;
        }

        int fd = open( full_path.c_str(), O_RDONLY );
        if( fd == -1 )
        {
            t.SystemError( "open() failed on file %s\n", full_path.c_str() );
            ClientError( "File Not Found" );
            return;
        }
    
        // Everything looks good.  Return the requested web page then.
        int rc, wc;
        char buf[64];
        while( 0 != (rc = read( fd, buf, sizeof(buf)) ) )
        {
            if( rc < 0 )
            {
                t.SystemError( "read() returned %d", rc );
                ClientError( "Read error" );
                close( fd );
                return;
            }
            wc = write( m_socket, buf, rc );
            if( wc < 0 )
            {
                t.SystemError( "write() returned %d", wc );
                ClientError( "Write error" );
                close( fd );
                return;
            }
        }
        close( fd );
    }

    // If got to here, then normal end of connection.
    t.Info( "Normal return from this function" );
}

void
WebServer::HandleUNIMPLEMENTED( HTTPCommand cmd )
{
    Trace t( "WebServer::HandleUNIMPLEMENTED()" );
    ClientError( std::string("Unimplemented method: ") + cmd.key );
    t.Warning( "Received unimplemented command %s %s",
      cmd.key.c_str(), cmd.value.c_str() );
}

/** Present the message, inside <HTML></HTML> */
void
WebServer::ClientError( std::string msg )
{
    m_network_utils.ToPeer( m_socket, "<HTML><H1>" + msg + "</H1></HTML>\r\n" );
}

/** Check if this client has access to this web page.
 *  Look for a <HEAD> block in the web page.  If it contains an <ALLOW> section,
 *  then allow access only to clients connecting from IP addresses with the
 *  indicated  prefixes.  For example,
 *  <ALLOW> 128.3.5 </ALLOW>
 *  <ALLOW> 209.24 </ALLOW>
 *  means, allow access only to clients from 128.3.5.* or 209.24.*.*.
 *  If there's a <DENY> section, then deny access according to analogous
 *  principles.
 *  If there are <ALLOW> sections as well as <DENY> sections, that makes sense
 *  if what's in the <DENY> section(s) is a subset of what's in the <ALLOW>
 *  sections.
 *
 *  This is the first time we even check that filename exists and is readable.
 *  If it's not, then return "false" anyway.  The next line of HandleGET() will
 *  check for readability and issue a ClientError if there's a problem.
*/
bool
WebServer::AccessDenied( std::string filename )
{
    HEADParser head_parser( filename, m_client_ip );
    return head_parser.AccessDenied();
}

/** Break up one line into the 4-letter command (e.g. GET, PUT, POST) and
 *  the rest.
*/
HTTPCommand::HTTPCommand( std::string one_line )
{
    Trace t( "HTTPCommand:HTTPCommand()" );

    char * str = new char[ one_line.size() + 1 ];
    strcpy( str, one_line.c_str() );
    char * lasts;
    
    char * tok = strtok_r( str, " \r\n\t\0", &lasts );
    if( ! tok )
    {
        t.Error("Tried to construct HTTPCommand object from nothing: %s",
                one_line.c_str());
        key = "";
        value = "";
    } else
    {
        key = std::string( tok );
        tok = strtok_r( 0, "\n\r\0", &lasts );
        if( ! tok )  // may be ok; just a key but no extra info.
        {
            value = "";
        } else
        {
            value = std::string( tok );
        }
    }
}


HEADParser::HEADParser( std::string filename, std::string client_IP )
  : m_filename( filename ),
    m_client_ip( client_IP )
{
}

/** Check if this client has access to this web page.
 *  Look for a <HEAD> block in the web page.  If it contains an <ALLOW> section,
 *  then allow access only to clients connecting from IP addresses with the
 *  indicated  prefixes.  For example,
 *  <ALLOW> 128.3.5 </ALLOW>
 *  <ALLOW> 209.24 </ALLOW>
 *  means, allow access only to clients from 128.3.5.* or 209.24.*.*.
 *  If there's a <DENY> section, then deny access according to analogous
 *  principles.  If there are <ALLOW> sections as well as <DENY> sections, that
 *  makes sense if what's in the <DENY> section(s) is a subset of what's in the
 *  <ALLOW> sections.
 *
 *  This is the first time we even check that filename exists and is readable.
 *  If it's not, then return "false" anyway.  The next line of HandleGET() will
 *  check for readability and issue a ClientError if there's a problem.
*/
bool
HEADParser::AccessDenied() const
{
    vector<IPPattern> allows( CollectIPPatterns("ALLOW") );
    vector<IPPattern> denies( CollectIPPatterns("DENY" ) );
    bool result = false;

    if( allows.size() > 0 )
    {
        result = true; // Unless we find a match.
        for( vector<IPPattern>::const_iterator i = allows.begin();
             i != allows.end();
             ++i )
        {
            if( i->Matches( m_client_ip ) )
            {
                result = false;
                break;
            }
        }
    }

    if( denies.size() > 0 )
    {   // Going into this, result is false if allows.size()==0, or if the
        // client IP matched one of the ALLOWs.
        for( vector<IPPattern>::const_iterator i = denies.begin();
             i != denies.end();
             ++i )
        {
            if( i->Matches( m_client_ip ) )
            {
                result = true;
                break;
            }
        }
    }

    return result;
}        
                
/** Scan through the first 20000 or so bytes of the HTML file, looking for
 *  <ALLOW> or <DENY> tags and accumulating the IP patterns that follow these
 *  tags into a collection -- the return value.
 *
 *  The only valid arguments are "ALLOW" or "DENY".
*/
vector<IPPattern>
HEADParser::CollectIPPatterns( std::string allow_or_deny ) const
{    
    Trace t( "HEADParser::CollectIPPatterns()" );

    vector<IPPattern> result;

    // Serious programming error, if this is called with an arg other than
    // "ALLOW" or "DENY".
    if( (allow_or_deny != "ALLOW")
    &&  (allow_or_deny != "DENY" ) )
    {
        t.Error( "CollectIPPatterns() arg %s is invalid!  Must be \"ALLOW\" or "
                 "\"DENY\"", allow_or_deny.c_str() );
        return result;
    }

    FILEWrapper infile( m_filename, "r" );
    if( infile.GetStatus() != STATUS_OK )
    {
        t.Error( "Can't open %s !?", m_filename.c_str() );
        return result;
    }

    // We'll assume all the <ALLOW> and <DENY> items are in the first buf_size
    // bytes of infile.
    int const buf_size = 20000;
    char buf[buf_size];
    int rc;
    if( (rc = fread( buf, sizeof(char), buf_size-1, infile.GetRep() )) <= 0 )
    {
        t.Error( "fread() on %s returned %d", m_filename.c_str(), rc );
        return result;
    }
    buf[rc*sizeof(char)] = 0;

    char * lasts = buf;
    char last_ch;
    bool is_within_angle_brackets;
    char * tok = StrtokXML( &lasts, &last_ch, &is_within_angle_brackets );
    char * prev_tok;
    while( tok )
    {
        // If we've come upon <ALLOW> or <DENY>, strtok the next item and push
        // it onto the return value.
        if( is_within_angle_brackets
        &&  std::string(tok) == allow_or_deny )
        {
            prev_tok = tok;
            tok = StrtokXML( &lasts, &last_ch, &is_within_angle_brackets );
            if( !tok )
            {
                t.Error( "Incomplete %s block; %s was the last HTML token "
                         "found", allow_or_deny.c_str(), prev_tok );
                return result;
            }
            if( is_within_angle_brackets )
            {
                t.Error( "%s was immediately followed by %s.", prev_tok, tok );
                return result;
            }

            // If we're here, we might have a valid IP pattern.  Test it first:
            if( ! IPPattern::IsValidPattern( tok ) )
            {
                t.Error( "%s is not a valid IP pattern.", tok );
                return result;
            }

            // It's valid.  Construct the IPPattern and push it onto the return
            // value.
            result.push_back( IPPattern( tok ) );
        }
        prev_tok = tok;
        tok = StrtokXML( &lasts, &last_ch, &is_within_angle_brackets );
    }

    return result;
}

/** Tokenizes at delimiters "><".  Indicates if token was found surrounded
 *  by angle brackets.  Throws away comments.  Doesn't understand escaped
 *  characters, so don't use '<' or '>' anywhere unless they really enclose
 *  a tag.
 *
 *  MT-safe.
 *
 *  @param lasts As in strtok_r (position to start from on next call).
 *         On first call, lasts should point to the beginning of the
 *         string you want to tokenize.
 *  @param last_ch Delimiter at which previous call ended.
 *  @param is_within_angle_brackets Indicates XML element.
*/
char *
StrtokXML(char **lasts,
          char* last_ch,
          bool* is_within_angle_brackets )
{
    Trace t("StrtokXML()");

    bool found_left_angle = false;
    bool found_right_angle = false;
    char const * delim = "<>\0";

    // If starting at a delimiter, move past that.
    char * c = *lasts;

    c += strspn(c, delim);
    if(*c == '\0') return 0;
    if( (( c > *lasts ) && (*(c-1) == '<') )
    ||  (*last_ch == '<') )
    {
        found_left_angle = true;
    }

    // Find next delimiter and move just past that.
    // FIXME: We are going to fail if there's a <> inside a comment.
    char * d  = c + strcspn(c, delim);
    if(*d != '\0')
    {
        if( *d == '>' )
        {
            found_right_angle = true;
        }

        if( ( found_left_angle && !found_right_angle )
        ||  ( !found_left_angle && found_right_angle ) )
        {
            char aLittleContext[20];
            aLittleContext[0] = *last_ch;
            aLittleContext[19] = 0;
            strncpy( aLittleContext+1, c, 18 );
            t.Warning() << "Bad format -- unmatched angle brackets at|"
                 << aLittleContext <<  "|\n";
            return 0;
        }

        *last_ch = *d;
        *d++ = '\0';
    }

    *lasts = d;
    *is_within_angle_brackets = found_left_angle && found_right_angle;

    if( c && *is_within_angle_brackets )
    {
        // Convert to uppercase
        if( c )
        {
            for( char * i = c; *i; i++ )
            {
                *i = toupper(*i);
            }
        }
    }

    return c;
}

/** Open the FILE */
FILEWrapper::FILEWrapper( std::string filename, std::string mode )
{
    Trace t( "FILEWrapper::FILEWrapper()" );

    m_rep = fopen( filename.c_str(), mode.c_str() );
    if( ! m_rep )
    {
        m_status = STATUS_ERROR;
    } else
    {
        m_status = STATUS_OK;
    }
/*
    int buf_size=1024;
    char buf[buf_size];
    int rc;
    if( (rc = fread( buf, sizeof(char), buf_size-1, m_rep )) <= 0 )
    {
        t.FatalError( "fread returned %d", rc );
    } else
    {
        t.Info( "fread returned|%s|", buf );
    }
*/
}

/** Close the file */
FILEWrapper::~FILEWrapper()
{
    if( m_rep )
    {
        fclose( m_rep );
    }
}

/** Stores an IP pattern found in an <ALLOW> or a <DENY> block. */
IPPattern::IPPattern( char const * pattern )
{
    // Trim away white space.
    char * pattern_copy = new char[ strlen( pattern ) + 1 ];
    strcpy( pattern_copy, pattern );
    char * lasts;
    char * tok = strtok_r( pattern_copy, ".", &lasts );
    if( ! tok )  // Should never happen, cuz we should have called
                //  IsValidPattern() first.
    {
        for( int i=0;i<4;i++ )
        {
            m_pattern.push_back(0);
        }
    } 
    else
    {   
        while( tok )
        {
            m_pattern.push_back( atoi(tok) );
            tok = strtok_r( 0, ".", &lasts );
        }
    }

    delete[] pattern_copy;
}

/** It's valid if, tokenized at the dots, it's made up of from 1 to 4 numbers 
 *  between and including 0 and 255.
*/
bool
IPPattern::IsValidPattern( char const * pattern )
{
    Trace t( "IPPattern::IsValidPattern()" );

    if( ! pattern )
    {
        t.Error( "Argument is NULL." );
        return false;
    }

    // First, trim away white space.
    char * pattern_copy = new char[ strlen( pattern ) + 1 ];
    strcpy( pattern_copy, pattern );
    char * lasts;
    char * tok = strtok_r( pattern_copy, "\t\n\r ", &lasts );
    if( ! tok )
    {
        t.Warning( "Nothing but whitespace in argument" );
        return false;
    } else
    {
        strcpy( pattern_copy, tok );
    }

    // Tokenize at the periods.
    tok = strtok_r( pattern_copy, ".", &lasts );
    int num_fields = 0;
    int val;
    char * endptr;
    while( tok )
    {
        ++ num_fields;
        if( num_fields > 4 )
        {
            t.Warning( "Too many fields in alleged IP address %s", pattern );
            return false;
        }

        val = strtol( tok, &endptr, 10 );
        if( *endptr != 0 )
        {
            t.Warning( "Invalid integer in alleged IP address %s\n", pattern );
            return false;
        } 

        if( val < 0  ||  val > 255 )
        {
                t.Warning( "Out-of-range integer in alleged IP address %s\n",
                           pattern );
                return false;
        }

        // So far so good.
        tok = strtok_r( 0, ".", &lasts );
    }

    return true;
}

/** Arg full_ip is the IP address of a client, e.g. xxx.yyy.zzz.www  .
 *  This object holds a pattern, e.g. aaa.bbb.ccc  .  We define there to be
 *  a match if xxx==aaa, yyy==bbb and zzz==ccc.  Or if this object is just
 *  aaa.bbb, we have a match if xxx==aaa and yyy==bbb.  Analogously if this
 *  object is just aaa.
 *
 *  Arg other_ip is a fully IP address.  *This, however, may be just something
 *  like 209. or 209.88. or 209.88.102. (always ending with a period).
*/
bool
IPPattern::Matches( std::string other_ip ) const
{
    Trace t( "IPPattern::Matches()" );

    if( ! IPPattern::IsValidPattern( other_ip.c_str() ) )
    {
        t.Error( "Client IP -- %s -- is weird.", other_ip.c_str() );
        return false;
    }

    vector<int> other_ip_components;

    // Parse other_ip and m_pattern.
    char * lasts;
    char * buf = new char[ other_ip.size() + 1 ];
    strcpy( buf, other_ip.c_str() );
    char * tok = strtok_r( buf, ".", &lasts );
    while( tok )
    {
        other_ip_components.push_back( atoi(tok) );
        tok = strtok_r( 0, ".", &lasts );
    }

    if( other_ip_components.size() != 4 )
    {
        t.Error( "Client IP address -- %s -- is incomplete!",
                 other_ip.c_str() );
        delete[] buf;
        return false;
    }

    bool result = true;
    for( unsigned int i=0;i< m_pattern.size(); i++ )
    {
        if( m_pattern[i] != other_ip_components[i] )
        {
            result = false;
        }
    }

    delete[] buf;
    return result;
}
