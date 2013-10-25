#include <iostream>
#include <string>
using namespace std;

struct B {
    virtual void foo(int i) = 0;
    void foo(string s) { cout << s << '\n'; }
};

struct D : B {
    virtual void foo(int i) { cout << i << '\n'; }
};


int main()
{
    D d;
    d.foo(string("hello"));
}
