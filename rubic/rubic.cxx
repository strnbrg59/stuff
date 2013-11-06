#include "rubic.h"
#include <cctype>
using namespace std;

ostream& operator<<(ostream& out, Printable const& p)
{
    return p.print(out);
}

string invert_turnstring(string const& turnstring)
{
    string result;
    for (string::const_reverse_iterator i = turnstring.rbegin();
         i != turnstring.rend(); ++i)
    {
        if (isupper(*i)) {
            result += tolower(*i);
        } else
        if (islower(*i)) {
            result += toupper(*i);
        } else
        {
            cerr << "Illegal turn symbol: " << *i << '\n';
            assert(0);
        }
    }
    return result;
}
