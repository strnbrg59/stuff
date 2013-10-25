#include <iostream>
using namespace std;

struct Base {
    void func() { cout << "Base::func()\n"; }
};

struct Derived : public Base {
    void func() {
        cout << "Derived::func()\n";
        Base::func();
    }
};

int main()
{
    Derived d;
    d.func();
}
