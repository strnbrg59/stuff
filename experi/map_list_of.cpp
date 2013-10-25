#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <map>
#include <iostream>
using namespace std;

int main()
{
    typedef map<char, int> MapT;
    MapT mymap = boost::assign::map_list_of('a', 1)('b', 2)('c', 3);
    BOOST_FOREACH(MapT::value_type v, mymap) {
        cout << "(" << v.first << ", " << v.second << ")\n";
    }
}
