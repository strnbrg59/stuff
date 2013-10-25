#include <iostream>
#include <string>
using namespace std;

template<typename T> struct Base {};

template<typename T> struct General : public Base<General<T> > {
    General() {
        cout << T::name() << '\n';
    }
    string name;
};

struct Foo {
    static string name() {
        return "Foo";
    }
};
struct Bar {
    static string name() {
        return "Bar";
    }
};

typedef General<Foo> GeneralFoo;
typedef General<Bar> GeneralBar;

int main()
{
    GeneralFoo f;
    GeneralBar b;
}
