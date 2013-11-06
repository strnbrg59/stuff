#ifndef INCLUDED_WEBSERVER_HPP
#define INCLUDED_WEBSERVER_HPP

#include "cmdline.hpp"
#include "network-utils.hpp"
#include "status-codes.hpp"
#include <cstdio>
#include <vector>
#include <string>
using std::vector;

char *
StrtokXML(char **lasts,
          char* last_ch,
          bool* is_within_angle_brackets );

class FILEWrapper
{
  public:
    FILEWrapper( string filename, string mode );
    ~FILEWrapper();
    Status GetStatus() const { return m_status; }
    FILE* GetRep() const { return m_rep; }
  private:
    FILE* m_rep;
    Status m_status;
};

struct HTTPCommand
{
    HTTPCommand( string one_line );
    string key;    // e.g. GET, PUT, POST, etc
    string value;  // the rest.
};

/** A pattern can be xxx or xxx.yyy or xxx.yyy.zzz or xxx.yyy.zzz.www.
 *  Maybe we'll support wildcards in the future...
*/
class IPPattern
{
  public:
    IPPattern( char const * pattern );
    bool Matches( string full_ip ) const;
    static bool IsValidPattern( char const * );
  private:
    vector<int> m_pattern; // components strtok'd at the '.'s.
};

class HEADParser
{
  public:
    HEADParser( string filename, string client_IP );
    bool AccessDenied() const;
  private:
    vector<IPPattern> CollectIPPatterns( string allow_or_deny ) const;

    string m_filename;  // The html file the client has requested
    string m_client_ip; // in dotted decimal form
};

class WebServer
{
  public:
    WebServer( int socket, Cmdline const &, struct sockaddr_in const & );
    void Converse();

  private:
    int m_socket;
    Cmdline const & m_config;

    void RespondToClient( HTTPCommand cmd );

    // Handlers for various HTTP commands.
    void HandleGET( HTTPCommand );
    void HandlePOST( HTTPCommand );
    void HandleUNIMPLEMENTED( HTTPCommand );

    void ClientError( string msg );

    bool AccessDenied( string filename );

    enum { m_max_name_length = 256 };
    char m_host_name[ m_max_name_length + 1 ];
    char m_domain_name[ m_max_name_length + 1 ];
    sockaddr_in const & m_client_sockaddr;
    string m_client_ip;

    vector<string> m_data_lines;
    vector<string> m_http_commands;

    NetworkUtils m_network_utils;
};

#endif // INCLUDED_WEBSERVER_HPP
