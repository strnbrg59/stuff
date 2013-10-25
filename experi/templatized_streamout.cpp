#include <set>
#include <iostream>
#include <boost/foreach.hpp>
using namespace std;
typedef unsigned uint32;

template<typename T>
ostream& operator<<(ostream& out, std::set<T> const& v)
{
    out << "[";
    BOOST_FOREACH(T const& t, v) {
        out << t << ',';
    }
    out << "]";
    return out;
}

int main()
{
    std::set<int> si; si.insert(1);
    std::set<double> sd; sd.insert(3.14);
    cout << si << '\n';
    cout << sd << '\n';
}
