#include <iostream>
using namespace std;

struct A
{
    A() {}
    virtual ~A() {}
    virtual void sayit()  = 0;
};

struct B1 : public A
{
    B1() : A() {}
    virtual void sayit() { cout << "B1 here\n"; }
};

struct B2 : public A
{
    B2() : A() {}
    virtual void sayit() { cout << "B2 here\n"; }
};

int main()
{
    B1* b1 = new B1;
    B2* b2 = new B2;
    delete b1;
    delete b2;
}
