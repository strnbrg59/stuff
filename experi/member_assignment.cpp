#include <iostream>
#include <vector>
#include <map>
using std::cout;

struct Foo {
    int i;
    int j;
    double x;
};

typedef int Foo::*FI;
typedef double Foo::*FD;

template<typename T> struct MT_Traits;
template<> struct MT_Traits<FI> {
    typedef int type;
};
template<> struct MT_Traits<FD> {
    typedef double type;
};


typedef std::map<int,FI> FIMAP;

template<typename MT> void assignMember(Foo& foo, MT& fm,
                                        typename MT_Traits<MT>::type t)
{
    foo.*fm = t;
}

int main()
{
    Foo foo;

    FI fi[2] = {&Foo::i, &Foo::j};
    FIMAP fimap;
    fimap[29] = fi[0];
    fimap[37] = fi[1];

    for( FIMAP::iterator mi=fimap.begin(); mi != fimap.end(); ++mi ) {
        assignMember<FI>(foo, mi->second, mi->first);
    }

    FD fd = &Foo::x;
    assignMember<FD>(foo, fd, 3.14);

    cout << "foo.i=" << foo.i << '\n';
    cout << "foo.j=" << foo.j << '\n';
    cout << "foo.x=" << foo.x << '\n';
}
