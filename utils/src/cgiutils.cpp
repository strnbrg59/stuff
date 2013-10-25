#include "cgiutils.hpp"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <boost/tokenizer.hpp>

using namespace std;

/** Trim off leading and trailing control characters.
 *  Convert '+' to space.
 *  Convert instances of "%XY" to a single character for which XY
 *      is the hexadecimal ASCII code.
*/
void QueryStringCleanup(string & egrep_iv)
{
    unsigned long pos;
    pos = egrep_iv.find_first_not_of( "\t\r\n" );
    egrep_iv.erase( 0, pos );

    pos = egrep_iv.find_last_not_of( "\t\r\n" );
    egrep_iv.erase( pos+1 );

    if( egrep_iv == "+" )
    {
        egrep_iv = "";
    } else
    {
        pos = egrep_iv.find("+");
        while( pos != egrep_iv.npos )
        {
            egrep_iv.replace( pos,1," " );
            pos = egrep_iv.find("+");
        }

        pos = egrep_iv.find("%");
        while( pos != egrep_iv.npos )
        {
            string hexa( egrep_iv, pos+1, 2 );
            char ascii[2];
            sprintf(ascii, "%c", strtol(hexa.c_str(),0,16));
            egrep_iv.replace( pos,3,ascii );
            pos = egrep_iv.find("%");
        }
    }
}


/** Stuff QUERY_STRING into a convenient map. */
map<string, string>
QueryStringParse(string qstr)
{
    QueryStringCleanup(qstr);
    map<string, string> result;
    typedef boost::tokenizer<boost::char_separator<char> > CharTokenizer;
    CharTokenizer amperToker(qstr, boost::char_separator<char>("&"));

    for(CharTokenizer::const_iterator i=amperToker.begin();
        i!=amperToker.end(); ++i)
    {
        CharTokenizer equalsToker(*i, boost::char_separator<char>("="));
        CharTokenizer::iterator j = equalsToker.begin();
        string key = *j;
        ++j;
        string val = j == equalsToker.end() ? "" : *j;
        result.insert(make_pair(key, val));
    }

    return result;
}

        
        

