#include <iostream>
using namespace std;

struct B {
    B() {}
    ~B() {}
    virtual void f() { cout << "B::f()\n"; }
};

struct D : B {
    D() : B() {}
    ~D() {}
    void f() { cout << "D::f()\n"; }
};

int main()
{
    B* d = new D;
    d->f();
}
