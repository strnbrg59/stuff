#include <iostream>

struct Foo
{
    Foo();
};

Foo foo;
int g_i(1);

Foo::Foo()
{
    std::cerr << "Foo::Foo(): g_i=" << g_i << ", now setting it to 5.\n";
    g_i = 5;
}

int main()
{
    std::cerr << "main(): g_i=" << g_i << '\n'; // Prints 5
}
