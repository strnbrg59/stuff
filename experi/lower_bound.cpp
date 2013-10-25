#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <boost/foreach.hpp>
using namespace std;

ostream& operator<<(ostream& out, vector<int> const& v) {
    out << "[";
    BOOST_FOREACH(int i, v) {
        out << i << ", ";
    }
    out << "]";
    return out;
}

int main(int argc, char* argv[])
{
    vector<int> vi;
    int const n = 5;
    for (int i=0;i<n;++i) {
        vi.push_back(random()%100);
    }
    cout << vi << '\n';
    sort(vi.begin(), vi.end());
    cout << vi << '\n';

    vector<int>::iterator iter = lower_bound(vi.begin(), vi.end(),
                                             atoi(argv[1]));
    cout << "*iter = " << *iter << '\n';
    cout << "iter - begin = " << iter - vi.begin() << '\n';
}
