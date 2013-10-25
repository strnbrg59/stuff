#include <boost/shared_ptr.hpp>
#include <iostream>
#include <set>
using namespace std;
int main()
{
    typedef boost::shared_ptr<int> IntPtr;
    typedef std::set<IntPtr> IntPtrSet;
/*
    IntPtrSet ips;
    for (IntPtrSet::iterator i=ips.begin(); i!=ips.end(); ++i) {
        i->reset();
    }
*/

    IntPtr ip1(new int(17));
    IntPtr ip2 = ip1;
    ip1.reset();
    cout << "ip1.use_count()=" << ip1.use_count() << '\n';
    cout << "ip2.use_count()=" << ip2.use_count() << '\n';
}
