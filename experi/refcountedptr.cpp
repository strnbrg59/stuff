//
// Simple ref-counted pointer class.  Leaks memory (don't know why yet).
//

#include <iostream>
#include <map>
using namespace std;

template<typename T> class RFP
{
public:
    RFP(T* p)
    {
        _rep = p;
        _refCount[_rep] = 1;
    }

    ~RFP()
    {
        if(0 == --(_refCount[_rep])) delete _rep;
    }

    RFP(RFP const& that) : _rep(that._rep)
    {
        ++_refCount[_rep];
    }

    RFP& operator==(RFP const& that)
    {
        if(0 == --(_refCount[_rep])) delete _rep;        
        _rep = that._rep;
        ++_refCount[_rep];
    }

    T* operator->() { return _rep; }

private:
    T* _rep;
    static map<T*, int> _refCount; // Hash table would be lots faster.
};

struct S
{
    S(int i): _i(i) { }
    int _i;
};

int main()
{
    RFP<S> ps(new S(17));
    cout << ps->_i << '\n';

    RFP<S> ps2(ps);

    RFP<S> ps3(new S(19));
    ps = ps2 = ps3;
}

template<typename S> map<S*,int> RFP<S>::_refCount;
