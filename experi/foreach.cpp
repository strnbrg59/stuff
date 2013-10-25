#include <boost/foreach.hpp>
#include <string>
#include <iostream>
using namespace std;

int main()
{
    std::string hi("hello world!");
    BOOST_FOREACH(char s, hi) {
        cout << s;
    }
    cout << endl;

    const char* strs[3] = {"foo", "bar", NULL};
    int i=0;
    BOOST_FOREACH(const char* str, strs) {
        cout << (i++) << " : " << str << '\n';
    }
}
