#ifndef INCLUDED_WEBCLIENT_HPP
#define INCLUDED_WEBCLIENT_HPP

#include "network-utils.hpp"
#include <string>

class WebClient
{
  public:

    WebClient( short port, string IPAddr, Cmdline const & );
    ~WebClient();

    void Request( string str );
    void Send( short port, string hostIP );

  private:

    Cmdline const & m_config;
    int m_socket;

    void CheckResponse( string expected_SMTP_code );

    string m_request;

    NetworkUtils m_network_utils;
};

#endif // INCLUDED_WEBCLIENT_HPP




