#ifndef INCLUDED_MAILSERVER_HPP
#define INCLUDED_MAILSERVER_HPP

#include "cmdline.hpp"
#include "network-utils.hpp"
#include <vector>
#include <string>

struct SMTPCommand
{
    SMTPCommand( string one_line );
    string key;    // e.g. HELO, MAIL, RCPT, QUIT
    string value;  // the rest.
};

class MailServer
{
  public:
    MailServer( int socket, Cmdline const &, struct sockaddr_in const & );
    void Converse();
    void Record() const;

  private:
    int m_socket;
    Cmdline const & m_cmdline;

    void RespondToClient( SMTPCommand cmd );
    void Greeting();

    // Handlers for various SMTP commands.
    void HandleHELO( SMTPCommand );
    void HandleMAIL( SMTPCommand );
    void HandleRCPT( SMTPCommand );
    void HandleDATA( SMTPCommand );
    void HandleQUIT( SMTPCommand );
    void HandleNOOP( SMTPCommand );
    void HandleUNIMPLEMENTED( SMTPCommand );

    void RecordPerRCPT( string rcpt_userid ) const;

    enum { m_max_name_length = 256 };
    char m_host_name[ m_max_name_length + 1 ];
    char m_domain_name[ m_max_name_length + 1 ];
    sockaddr_in const & m_client_sockaddr;
    std::vector<string> m_RCPT_userids;

    std::vector<string> m_data_lines;
    std::vector<string> m_smtp_commands;

    NetworkUtils m_network_utils;
};

#endif // INCLUDED_MAILSERVER_HPP
