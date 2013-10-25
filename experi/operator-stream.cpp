#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
using namespace std;

template<typename C> struct Streamable
{
    Streamable(C const& c) : rep_(c) {}
    C const& rep_;
};

template<typename C> Streamable<C>
streamer(C const& c) {
    return Streamable<C>(c);
}

template<typename K, typename V> ostream&
operator<<(ostream& out, std::pair<K,V> const& p) {
    out << "(" << p.first << ", " << p.second << ")";
    return out;
}

template<typename C> ostream&
operator<<(ostream& out, Streamable<C> const& cw)
{
    out << "{";
    BOOST_FOREACH(typename C::value_type const& v, cw.rep_) {
        out << v << ", ";
    }
    out << "}";
    return out;
}

int main()
{
    vector<int> vi;
    deque<int> di;
    map<int, double> mi;
    map<int, vector<double> > miv;
    std::set<double> sd;

    for (int i=0;i<10;++i) {
        if (i%2) {
            vi.push_back(i);
            mi[i] = i/2.0;
            miv[i].push_back(i/2.0);
            miv[i].push_back(i/4.0);
        } else {
            di.push_back(i);
            sd.insert(i + 0.0);
        }
    }

    cout << streamer(vi) << '\n';
    cout << streamer(di) << '\n';
    cout << streamer(mi) << '\n';
    cout << streamer(sd) << '\n';

    // This doesn't work though:
    // cout << streamer(miv) << '\n';
}    
