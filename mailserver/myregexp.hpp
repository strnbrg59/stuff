#ifndef INCLUDED_MYREGEXP_HPP
#define INCLUDED_MYREGEXP_HPP

class MyRegexp
{
public:
    MyRegexp( string pattern );
    bool matches( string haystack );
private:
    string m_pattern;
};

#endif // INCLUDED_MYREGEXP_HPP
