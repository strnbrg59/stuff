#include <algorithm>

#include "punutils.hpp"
using std::string;

/** Trim off leading and trailing whitespace.
 *  Capitalize.
 *  Remove anything left, except capital letters, blanks and apostrophes.
*/
void StrNormalize( string & str )
{
    // Capitalize.
    std::transform( str.begin(), str.end(), str.begin(), toupper );

    // Convert any non-letters-not-blanks-non-apostrophe to spaces.
    static string lettersBlankAndApostrophe( " ABCDEFGHIJKLMNOPQRSTUVWXYZ\'" );

    unsigned long pos = str.find_first_not_of( lettersBlankAndApostrophe );
    while( pos != str.npos )
    {
        str.replace( pos,1," " );
        pos = str.find_first_not_of( lettersBlankAndApostrophe );
    }

    // Remove multiple blanks
    pos = str.find("  ");
    while( pos != str.npos )
    {
        str.erase( pos,1 );
        pos = str.find("  ");
    }

    // Remove apostrophes that aren't sandwiched between two letters --
    // those are not apostrophes but single quotes, and we don't want them.
    pos = str.find( "'" );
    while( pos != str.npos )
    {
        if( (pos==0) || (pos==str.size()-1)
        ||  (str[pos-1]==' ') || (str[pos+1]==' ') )
        {
            str.erase( pos, 1 );
            pos = str.find( "'", pos );
        } else
        {
            pos = str.find( "'", pos+1 );
        }
    }

    // trim leading and trailing white space.
    StrTrim( str );
}


/** Trim off leading and trailing whitespace. */
void StrTrim( string & str )
{
    unsigned pos;
    pos = str.find_first_not_of( " \t\r\n" );
    str.erase( 0, pos );

    pos = str.find_last_not_of( " \t\r\n" );
    str.erase( pos+1 );
}
