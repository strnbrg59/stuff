#ifndef INCLUDED_MYREGEXP_HPP
#define INCLUDED_MYREGEXP_HPP

#include <string>
using std::string;

class MyRegexp
{
public:
    MyRegexp( string pattern );
    bool matches( string haystack, bool caseInsensitive=false );
private:
    string m_pattern;
};

#endif // INCLUDED_MYREGEXP_HPP
