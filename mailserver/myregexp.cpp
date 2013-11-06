/* Recognize regular expressions.  Print "success" or "failure". */

#include <stdlib.h>
#include <regex.h> // doesn't compile unless preceded by #include <stdlib.h>
#include <iostream>
#include <string>
#include "myregexp.hpp"


MyRegexp::MyRegexp( string pattern ) : m_pattern(pattern) {}

bool MyRegexp::matches( string haystack )
{
    regex_t compiled;
    int status;
    char regerrorBuf[128];
    if( (status=regcomp( &compiled, m_pattern.c_str(), REG_NOSUB )) != 0 )
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

/*
main( int argc, char* argv[] )
{
    if( argc != 3 )
    {
        cout << "Usage: " << argv[0] << " <pattern> <haystack>\n";
        exit(1);
    }
    const char* haystack = argv[2];

    MyRegexp pattern( argv[1] );
//  MyRegexp pattern( "^\\." );

    if( pattern.matches(haystack) )
        cout << "success" << '\n';
    else
        cout << "failure" << '\n';
}
*/
