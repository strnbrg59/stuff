#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>
using namespace boost;
using namespace std;

// Doesn't work.
void strip(string& str)
{
    str.erase(remove(str.begin(), str.end(), ' '));    
    str.erase(remove(str.begin(), str.end(), ' '));    
}

int main()
{
    string str("  foo ");
    strip(str);
    cout << "|" << str << "|\n";
}

