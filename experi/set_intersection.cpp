#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <boost/foreach.hpp>
using std::cout;
using std::set;
using std::ostream;

ostream& operator<<(ostream& out, set<int> const& si) {
    out << "{";
    BOOST_FOREACH(int i, si) {
        out << i << " ";
    }
    out << "}";
    return out;
}

int main()
{
    set<int> s2, s3, intsect;
    for (int i=0;i<20;++i) {
        if (i%2 == 0) s2.insert(i);
        if (i%3 == 0) s3.insert(i);
    }

    std::set_intersection(s2.begin(), s2.end(), s3.begin(), s3.end(),
                          std::inserter(intsect, intsect.begin()));

    cout << "s2 = " << s2 << '\n';
    cout << "s3 = " << s3 << '\n';
    cout << "intersection = " << intsect << '\n';
}
