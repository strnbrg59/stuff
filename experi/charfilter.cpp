#include <cassert>
#include <string>
#include <iostream>
using namespace std;

/* Return a copy of str, but with some characters missing.
 * If accept!=0, then retain only characters in accept.
 * If accept==0, accept everything that's not rejected by the reject rule.
 * If reject!=0, then reject any characters in reject.
 * If reject==0, then don't reject anything.
*/
string charfilter(const string& str, const char* accept, const char* reject)
{
    string result;
    for (unsigned i=0;i<str.size();++i) {
        if (reject) {
            if (strchr(reject, str[i])) {
                continue;
            }
        }
        if (accept) {
            if (strchr(accept, str[i])) {
                result += str[i];
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

int main(int argc, char** argv)
{
    const char* letters =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    assert(charfilter("foo", 0, 0) == "foo");
    assert(charfilter("f/oo", letters, 0) == "foo");
    assert(charfilter("@f/oo", 0, "@/") == "foo");
}
