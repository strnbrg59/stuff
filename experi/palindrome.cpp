#include <iostream>
#include <vector>
#include <list>
#include <boost/assign/list_of.hpp>
using namespace std;

template<typename T> bool is_pali(vector<T> v) {
    typename vector<T>::iterator fi = v.begin();
    typename vector<T>::iterator ri = v.end(); --ri;

    while (fi < ri) {
        if (*fi != *ri) return false;
        ++fi;
        --ri;
    }

    return true;
}

int main()
{
    vector<char> vc = boost::assign::list_of<char>('h')('a')('n')('n')('a')('h');
    cout << "ispali(\"hannah\")=" << is_pali(vc) << '\n';

    vector<int> vc2 = boost::assign::list_of<int>(1)(2)(3)(4)(5);
    cout << "ispali(12345)=" << is_pali(vc2) << '\n';

    vector<int> vc3 = boost::assign::list_of<int>(1)(2)(3)(2)(1);
    cout << "ispali(12321)=" << is_pali(vc3) << '\n';

    vector<int> vc4 = boost::assign::list_of<int>(1)(2)(3)(0)(3)(2)(1);
    cout << "ispali(1230321)=" << is_pali(vc4) << '\n';

    vector<int> vc5 = boost::assign::list_of<int>(1)(2)(3)(0)(2)(3)(2)(1);
    cout << "ispali(12302321)=" << is_pali(vc5) << '\n';
}
