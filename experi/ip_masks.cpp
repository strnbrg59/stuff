#include <string>
#include <iostream>
#include <boost/tokenizer.hpp>
using namespace std;

typedef boost::tokenizer<boost::char_separator<char> > TokerT;

int main()
{
    string ip1 = "1.2.3.4";
    string mask = "255.255.255.0";
    string ip2 = "1.2.3.6";

    
    