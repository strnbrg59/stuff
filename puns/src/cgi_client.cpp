#include "cgi_client.hpp"
#include "punutils.hpp"

#include "exceptions.hpp"
#include "cmdline.hpp"
#include "trace.hpp"
#include "myregexp.hpp"
#include "network-utils.hpp"
#include "cgiutils.hpp"
#include <iostream>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <vector>
using std::cout; using std::cerr; using std::endl;

/**
  The "cgi client" is the cgi program launched by the web server.
  The cgi client parses $QUERY_STRING and communicates with the pun server,
  which holds the many tables in memory, does the search, and sends its results
  back to the cgi client.
*/
int main( int argc, char * argv[] )
{
    Cmdline cmdline( argc, argv );
    cout << "Content-type: text/html\n\n\n";
    if( ! ServerIsUp( cmdline.InetPort() ) )
    {
        cout << "<HTML>\nServer unavailable\n </HTML>\n";
        exit(1);
    }

    DoCGI( cmdline.InetPort() );
}


/** This does most of the work, of the CGI client. */
void DoCGI( short port )
{
    Trace t("DoCGI()");
    cout << "<HTML><TT>" << endl;
    cout << "<BODY BGCOLOR=\"#FFD080\">" << endl;

    NetworkUtils netutils;

    int socket;
    try
    {
        socket = netutils.GetClientSocket( port, "127.0.0.1" );
    }
    catch( ... )
    {
        t.FatalError( "Failure in NetworkUtils.GetClientSocket()" );
    }

    std::string clue, egrep_iv;
    int badness;
    bool multispan;
    if( getenv("QUERY_STRING") )
    {
        ProcessQueryString( &clue, &egrep_iv, &badness, &multispan );
    } else
    {
        clue = "rowing\n";
        egrep_iv = "";
    }

    //
    // Protocol for communication to server: clue[|badness]
    //
    char stol[12];
    sprintf( stol, "%d", badness );
    char sspan[2];
    sprintf( sspan, "%d", multispan );
    netutils.ToPeer( socket, (clue + std::string("|") + std::string(stol)
                            + std::string("|") + std::string(sspan) + "\n").c_str() );
    std::string fromServer;
    int nEliminatedByEgrep(0);
    try
    {
        while( 1 )  // breaks out on exception--normally an EofException
        {
            fromServer = netutils.ReadLine( socket, 10 );

            // Filter out anything that matches the egrep_iv.
            bool printIt( true );
            if( egrep_iv.size() > 0 )
            {
                MyRegexp regexp( egrep_iv );
                std::string normalizedFromServer( fromServer );
                StrNormalize( normalizedFromServer );
                if( regexp.matches( normalizedFromServer, true ) )
                {
                    printIt = false;
                    ++nEliminatedByEgrep;
                }
            }
            if( printIt )
            {
                cout << fromServer << "<BR><BR>" << endl;
            }
        }
    }
    catch( EofException & eof )
    {
        t.Info() << eof << '\n';
    }
    catch( TimeoutException & toe )
    {
        cout << "Timed out\n";
        t.Info() << toe << '\n';
    }
    catch( ReadErrorException & ree )
    {
        t.Info() << ree << '\n';
    }
    catch(...)
    {
        t.FatalError( "Failure in NetworkUtils.ReadLine()" );
    }

    if( nEliminatedByEgrep > 0 )
    {
        cout << "*** " << nEliminatedByEgrep
             << " results eliminated by 'except'. <BR><BR>\n";
    }
    cout << "</TT></HTML>" << endl;
}


bool ServerIsUp( short port )
{
    Trace t("ServerIsUp()");
    int result( false );
    try
    {
        NetworkUtils netutils;
        int socket = netutils.GetClientSocket( port, "127.0.0.1" );
        result = true;
        close( socket );
    }
    catch( ... )
    {
    }
    return result;
}


/** Finds values associated with keywords "clue" and "egrep_iv" in QUERY_STRING
 *  environment variable.
*/
void ProcessQueryString( std::string * clue, std::string * egrep_iv,
                         int * badness, bool * multispan )
{
    // Borrowed this from the ngrams project; it's more general than we need
    // here.
    Trace t("ProcessQueryString()");

    std::string query_string = getenv("QUERY_STRING");
    t.Info( "QUERY_STRING=|%s|", query_string.c_str() );

    char* qs = new char[ query_string.size() + 1 ];
    strcpy( qs, query_string.c_str() );

    // Grab the kv pairs between the & delimiters.
    std::vector<std::string> option_pairs;
    char * lastchar;
    char* tok = strtok_r( qs, "&\0\n", &lastchar );
    while( tok )
    {
        option_pairs.push_back( tok );
        tok = strtok_r( NULL, "&\0\n", &lastchar );
    }

    // Break up the kv pairs.
    std::map<std::string,std::string> options;
    for( std::vector<std::string>::const_iterator i = option_pairs.begin();
         i != option_pairs.end();
         ++ i )
    {
        strcpy( qs, i->c_str() );
        tok = strtok_r( qs, "=", &lastchar );
        assert( tok );
        std::string stok( tok );
        char * val = strtok_r( NULL, "\0\n", &lastchar );
        if( val )
        {
            options[stok] = val;
        } else
        {
            options[stok] = "";
        }
        t.Info( "options[%s]=%s", stok.c_str(), val );
    }
    delete [] qs;

    std::map<std::string,std::string>::const_iterator iter =
        options.find( "clue" );
    assert( iter != options.end() );
    *clue = iter->second;
    QueryStringCleanup(*clue);

    iter = options.find( "egrep_iv" );
    if( iter == options.end() )
    {
        *egrep_iv = "";
    } else
    {
        *egrep_iv = iter->second;
    }
    QueryStringCleanup(*egrep_iv);

    iter = options.find( "badness" );
    assert( iter != options.end() );
    *badness = atoi(iter->second.c_str());

    // Unlike the other widgets, the checkbox doesn't show up in QUERY_STRING
    // at all, unless it's checked.
    *multispan = 0;
    if( options.end() != options.find( "multispan" ) )
    {
        *multispan = 1;
    }
}
