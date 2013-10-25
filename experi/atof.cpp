#include <cmath>
#include <iostream>
using namespace std;

double myatof(char const* str)
{
    double result = 0;
    int m=0;
    int decpt=99;
    for(char const* p = str; *p; ++p)
    {
        int digit;
        if(*p == '.')
        {
            decpt = m;
        } else
        {
            digit = *p - '0';
            result += digit*pow(10.0,double(m));
            --m;
        }
    }
    if(decpt==99) decpt=m;
    return result*pow(10.0,-double(decpt+1));
}

int main(int argc, char** argv)
{
    cout << myatof(argv[1]) << '\n';
}
