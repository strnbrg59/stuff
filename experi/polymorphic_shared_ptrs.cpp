#include <boost/shared_ptr.hpp>
#include <iostream>
#include <cassert>
using namespace std;
using boost::shared_ptr;

struct B {
    virtual void func()  = 0;
};

struct D1 : B {
    virtual void func() { cout << "D1" << '\n'; }
};
struct D2 : B {
    virtual void func() { cout << "D2" << '\n'; }
};

int main()
{
    shared_ptr<B> d1(new D1);
    shared_ptr<B> d2(new D2);

    d1->func();
    d2->func();
}
