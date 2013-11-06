#include <iostream>
using namespace std;
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

typedef boost::shared_ptr<int> PtrI;

struct MS
{
    MS(PtrI const& p) : _p(p) {}
    PtrI _p;
};

int main()
{
    PtrI p(new int(17));
    MS ms1(p);
    PtrI& pcopy(ms1._p);
    cout << p.use_count() << " " << pcopy.use_count() << '\n';

    {
        MS ms2(p);
        cout << p.use_count() << " " << pcopy.use_count() << '\n';
        p = PtrI(new int(23));
        cout << p.use_count() << " " << pcopy.use_count() << '\n';
    }
    cout << p.use_count() << " " << pcopy.use_count() << '\n';
}
