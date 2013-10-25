#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <boost/bind.hpp>
using namespace std;

typedef int (*classifierT)(int);

bool contains_class(string str, classifierT f)
{
    return find_if(str.begin(), str.end(), f) != str.end();
}

int not_alnum(int c) { return !isalnum(c); }

bool strong_enough(string pwd)
{
    int n_classes = 0;
    n_classes += contains_class(pwd, islower);
    n_classes += contains_class(pwd, isupper);
    n_classes += contains_class(pwd, isdigit);
    n_classes += contains_class(pwd, not_alnum);
    cout << pwd << " contains " << n_classes << " classes.\n";
    
    bool enough_complexity =
           ((n_classes <= 2) && (pwd.size() >= 8))
        || ((n_classes == 3) && (pwd.size() >= 7))
        || ((n_classes == 4) && (pwd.size() >= 6));

    bool forbidden =
        strcasestr(pwd.c_str(), "admin") || strcasestr(pwd.c_str(), "root");

    return enough_complexity && !forbidden;
}

int main()
{
    cout << strong_enough("abc 123") << '\n';
    cout << strong_enough("aBc123") << '\n';
    cout << strong_enough("abc123@") << '\n';
    cout << strong_enough("aB123@") << '\n';
    cout << strong_enough("aadminB123@") << '\n';
}
