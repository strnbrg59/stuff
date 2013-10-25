#ifndef _CGICLIENT_HPP_
#define _CGICLIENT_HPP_

#include <string>
class Cmdline;


void CgiClient( Cmdline const & cmdline );
bool ServerIsUp( short port );
void DoCGI( short port );
void ProcessQueryString( std::string * clue, std::string * egrep_iv,
                         int * badness, bool * multispan );
void QueryStringCleanup( std::string & egrep_iv );

#endif // _CGICLIENT_HPP_
