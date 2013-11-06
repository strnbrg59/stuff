#ifndef DEFINED_NETWORK_UTILS_HPP
#define DEFINED_NETWORK_UTILS_HPP

#include <string>
using std::string;

class NetworkUtils
{
  public:
    NetworkUtils();

    int GetServerSocket( short port ) const;
    int GetClientSocket( short port, char const * addr ) const;

    string ReadLine( int socket, int seconds_timeout );
    void ToPeer( int socket, string msg );

    void BecomeDaemon( string log_filename );
    void ForkGrandchild( int socket );

  private:
    // ReadLine() and supporting data
    char m_readline_buf[1500];
    char * m_readline_buf_position;
    int  m_readline_count;

    int TimedRead( int socket, char * buf, size_t len, 
                   int seconds_timeout, bool * timed_out ) const;
};

void AlarmSignalHandler( int sig );

#endif // DEFINED_NETWORK_UTILS_HPP
