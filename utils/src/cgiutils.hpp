#ifndef _INCLUDED_CGIUTILS_HPP_
#define _INCLUDED_CGIUTILS_HPP_

#include <string>
#include <map>

void QueryStringCleanup(std::string& qstr);
std::map<std::string, std::string> QueryStringParse(std::string qstr);

#endif
