#include <string>
#include <set>
#include <iostream>
#include <cassert>
using namespace std;

struct NamedInt
{
    NamedInt(string n, int i) : _name(n), _i(i) {}
    string _name;
    int _i;
};

ostream& operator<<(ostream& out, NamedInt const& ni)
{
    out << "(" << ni._name << ", " << ni._i << ")";
    return out;
}

struct NamedIntComparison
{
    bool operator()(NamedInt const& i, NamedInt const& j) const
    {
        return i._name < j._name;
    }
};

template<typename T, typename CompT>
class IterablePriorityQueue
{
    typedef multiset<T, CompT> RepT;
    RepT _rep;
public:
    void pop() { _rep.erase(_rep.begin()); }
    void push(T i) { _rep.insert(i); }
    size_t size() { return _rep.size(); }

    typedef typename RepT::iterator iterator;
    iterator begin() { return _rep.begin(); }
    iterator end() { return _rep.end(); }
};

int main()
{
    typedef IterablePriorityQueue<NamedInt, NamedIntComparison> IPQ;
    IPQ si;
    si.push(NamedInt("fifty-four", 54));
    si.push(NamedInt("five", 5));
    si.push(NamedInt("minus-fourteen", -14));
    si.push(NamedInt("nine", 9));
    si.push(NamedInt("eleven", 11));
    si.push(NamedInt("minus-nine", -9));
    si.push(NamedInt("eleven", 11));

    assert(si.size() == 7);

    for(IPQ::iterator i=si.begin(); i!=si.end(); ++i) {
        cout << *i << " ";
    }
    cout << '\n';

    si.pop();
    assert(si.size() == 6);
    for(IPQ::iterator i=si.begin(); i!=si.end(); ++i) {
        cout << *i << " ";
    }
    cout << '\n';
}

