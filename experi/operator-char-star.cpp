#include <iostream>
using namespace std;

struct Foo {
    operator const char* () { return "helloworld"; }
    operator int () { return 37; }
};

int main()
{
    Foo foo;
    cout << (const char*)foo << '\n';
    cout << (const char*)(foo) << '\n';
    cout << int(foo) << '\n';
    cout << (int)foo << '\n';
}
