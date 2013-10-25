#include <string>
#include <iostream>
using namespace std;

unsigned countChar(char c, string str)
{
    unsigned result = 0;
    string::size_type iter = str.find(c);
    while(iter < str.size())
    {
        ++result;
        iter = str.find(c, iter+1);
    }
    return result;
}

int main()
{
    string str("axyxx");
    cout << countChar('x', str) << '\n';
    cout << countChar('y', str) << '\n';
}

