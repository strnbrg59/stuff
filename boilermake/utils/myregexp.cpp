/* Recognize regular expressions.  Print "success" or "failure". */

#include <stdlib.h>
#include <regex.h> // doesn't compile unless preceded by #include <stdlib.h>
#include <iostream>
#include <string>
//#include <unistd.h>
#include <getopt.h>
#include "myregexp.hpp"
using std::string;
using std::cerr;
using std::cout;


MyRegexp::MyRegexp( string pattern ) : m_pattern(pattern) {}

bool MyRegexp::matches( string haystack, bool caseInsensitive/*=false*/ )
{
    regex_t compiled;
    int status;
    char regerrorBuf[128];

    int flags = REG_NOSUB|REG_EXTENDED;
    if( caseInsensitive ) flags |= REG_ICASE;

    if( (status=regcomp( &compiled, m_pattern.c_str(), flags )) != 0 )
    {
        regerror( status, &compiled, regerrorBuf, 128 );
        cerr << regerrorBuf << '\n';
        regfree( &compiled );
        return false;
    }

    if( (status=regexec( &compiled, haystack.c_str(), 0, NULL, 0 )) == 0 )
    {
        regfree( &compiled );
        return true;
    }
    else
    {
        regfree( &compiled );
        return false;
    }
}

#ifdef UNIT_TEST
main( int argc, char* argv[] )
{
    if( argc < 3 )
    {
        cout << "Usage: " << argv[0] << " <pattern> <haystack> [-i|--case-insensitive]\n";
        exit(1);
    }
    const char* haystack = argv[2];

    bool caseInsensitive(false);
    int c = getopt( argc-2, argv+2, "i" );
    if( -1 != c )
    {
        if( c == 'i' )
        {
            caseInsensitive = true;
        }
    }

    MyRegexp pattern( argv[1] );
//  MyRegexp pattern( "^\\." );

    if( pattern.matches(haystack, caseInsensitive) )
        cout << "success" << '\n';
    else
        cout << "failure" << '\n';
}
#endif // UNIT_TEST
