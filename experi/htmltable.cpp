#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <cmath>
using namespace std;

typedef map<string,double> DataT;
typedef DataT::const_iterator DataIterT;
typedef double (*TRANSF)(double, double);

void print(DataT const& data, TRANSF f)
{
    cout << "<HTML>\n";
    cout << "<TABLE BORDER=1 CELLPADDING=10 CELLSPACING=1>\n";
    cout << "<TR><TD></TD>\n";
    for(DataIterT i = data.begin(); i != data.end(); ++i)
    {
        cout << "<TD> " << string(i->first, 0, 3) << "</TD>\n";
    }
    for(DataIterT i = data.begin(); i != data.end(); ++i)
    {
        cout << "<TR>\n";
        cout << "<TD> " << i->first << "</TD>\n";
        for(DataIterT j = data.begin(); j != data.end(); ++j)
        {
            if(i->first == j->first)
            {
                cout << "<TD></TD>\n";
            } else
            {
                cout << "<TD> " << setprecision(2)
                     << f(i->second, j->second) << "</TD>\n";
            }
        }
        cout << "</TR>\n";
    }
    cout << "</TABLE>\n";
    cout << "</HTML>\n";
}


double expected_spread(double x, double y)
{
    if(x > y)
    {
        return 5*(1-exp(y-x));
    } else
    {
        return -5*(1-exp(x-y));
    }
}


int main()
{
    DataT data;
    data.insert(make_pair("bob", 3.14));
    data.insert(make_pair("ada", 2.72));
    data.insert(make_pair("chuck", 1.41));
    print(data, expected_spread);
}
