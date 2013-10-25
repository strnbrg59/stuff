#include <errno.h>
#include <cassert>
#include <iostream>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
using namespace std;

typedef unsigned long long uint64;

int
lc_str_to_uint64(const char *s, uint64 *ret_val)
{
    int err = 0;
    char *end_char = NULL;
    uint64 result = 0;
    bool bad_string = false;

    if (ret_val) {
        *ret_val = 0;
    }

    errno = 0;
    result = strtoll(s, &end_char, 10);

    /* strtoll sets end_char to s if no valid characters were found */
    if (end_char == s) {
    bad_string = true;
    }

    /* consume trailing whitespace */
    while (isspace(*end_char)) {
    ++end_char;
    }

    /* bail on overflow */
    assert(errno != ERANGE);

    /* didn't make it to the end of the string or we set bad_string above */
    assert(*end_char == '\0' && !bad_string);

    if (ret_val) {
        *ret_val = result;
    }

    return(err);
}


string format_nicely(const string& raw)
{
    typedef boost::tokenizer<boost::char_separator<char> > TokerT;
    TokerT mytokenizer(raw, boost::char_separator<char>(" "));
    vector<string> tokens(mytokenizer.begin(), mytokenizer.end());
/*    
    cout << "raw=" << raw << '\n';
    cout << "tokens.size()=" << tokens.size() << '\n';
    for (vector<string>::iterator i=tokens.begin(); i!=tokens.end(); ++i) {
        cout << *i << endl;
    }
*/
    string result;

    assert(tokens.size() == 4);

    // megabytes
    uint64 bytes;
    int err = lc_str_to_uint64(tokens[0].c_str(), &bytes);
    assert(!err);
    double megabytes = bytes/(1E6);
    result = (boost::format("%.1fMB ") % megabytes).str();

    // percent complete
    assert(tokens[1].find('%') != string::npos);
    result += (boost::format("(%s), ") % tokens[1]).str();

    // megabytes per second
    assert(tokens[2].find("MB/s") != string::npos);
    result += tokens[2] + ", ";

    // time remaining
    string time_with_colons = tokens[3];
    mytokenizer = TokerT(time_with_colons, boost::char_separator<char>(":"));
    vector<string> time_tokens(mytokenizer.begin(), mytokenizer.end());
    assert(time_tokens.size() == 3);
    string nice_time;
    uint64 units[3];
    string unit_names[3] = {"h", "m", "s"};
    for (int i=0; i<3; ++i) {
        assert(!lc_str_to_uint64(time_tokens[i].c_str(), &units[i]));
        if ((units[i] > 0) || (i==2)) {
            nice_time += (boost::format("%d%s ")
                % units[i] % unit_names[i]).str();
        }
    }

    result += nice_time + "remaining";
    return result;
}


int main()
{
    cout << format_nicely("116948992 56% 6.88MB/s 0:00:12") << '\n';
    cout << format_nicely("11 0% 0.04MB/s 21:00:12") << '\n';
    cout << format_nicely("1234516948992 56% 556.88MB/s 0:02:00") << '\n';
    cout << format_nicely("116948992 56% 6.88MB/s 1:02:13") << '\n';
}
