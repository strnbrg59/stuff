#include <map>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

typedef map<int, string> MapT;
typedef MapT::iterator MapIteratorT;
typedef MapT::const_iterator MapConstIteratorT;

ostream& operator<<(ostream& out, MapT const& m)
{
    out << "{";
    for(MapConstIteratorT i = m.begin(); i!=m.end(); ++i)
    {
        out << "(" << i->first << ", " << i->second << ")";
    }
    out << "}";
    return out;
}

int main()
{
    MapT mymap;
    mymap[11] = "eleven";
    mymap[12] = "twelve";
    mymap[13] = "thirteen";
    cout << mymap << '\n';

    MapIteratorT i = mymap.begin();
    MapIteratorT temp;
    while(i != mymap.end())
    {
        temp = i;
        ++i;
        mymap.erase(temp);
        cout << mymap << '\n';
    }
}
