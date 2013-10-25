#include <boost/format.hpp>
#include <iostream>
using namespace std;

int main()
{
    int i=17, j=23;
    string str_fmt = "i = %d, j = %d";
    boost::format fmt = boost::format(str_fmt.c_str()) % i % j;
    cout << fmt.str() << '\n';
}
