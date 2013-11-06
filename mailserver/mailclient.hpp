#ifndef INCLUDED_MAILCLIENT_HPP
#define INCLUDED_MAILCLIENT_HPP

#include "network-utils.hpp"
#include <multimap.h>
#include <string>
#include <vector>

class MailClient
{
  public:

    MailClient( short port, string IPAddr, Config const & );
    ~MailClient();

    void MAIL( string mail_from );
    void RCPT( string rcpt_to );
    void DATA( string data_line );

    void Send();

  private:

    Config const & m_config;
    int m_socket;

    void CheckResponse( string expected_SMTP_code );

    static void ParseUseridAndDomain(
        string addr, string * userid, string * domain );
    string m_sender_userid;
    string m_sender_domain;
    multimap<string,string> m_destinations;

    string DATA() const;
    vector<string> m_data;

    NetworkUtils m_network_utils;
};


static string ParseHostName( string mail_addr );
static string GetHostQuadAddr( string to_email );

#endif // INCLUDED_MAILCLIENT_HPP

