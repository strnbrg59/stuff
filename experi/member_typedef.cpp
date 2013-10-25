#include <iostream>
#include <string>
using namespace std;

struct Foo
{
    typedef void (Foo::*funcT)();
    void func() { cout << "Foo::func()\n";}
};

void
callFunc(Foo& foo, Foo::funcT func)
{
    (foo.*func)();
}


int main()
{
    Foo foo;
    callFunc(foo, &Foo::func);
}
