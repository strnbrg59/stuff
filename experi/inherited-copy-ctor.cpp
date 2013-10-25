#include <iostream>
using namespace std;

struct B {
    B() : i_(0) {}
    B(int i) : i_(i) {}

    virtual void func() const = 0;
    virtual void print() const = 0;

    int i_;
};

struct D : B {
    D() : B(), j_(0) {}
    D(int i, int j) : B(i), j_(j) {}
        
    virtual void func() const { print(); }
    virtual void print() const {
        cout << "(" << i_ << ", " << j_ << ")" << '\n';
    }

    int j_;
};

ostream& operator<<(ostream& out, D const& d)
{

    return out;
}

void dispatch(B& b)
{
    b.func();
}

int main()
{
    D d1(11, 12);
    dispatch(d1);

    D d2;
    d2 = d1;
    dispatch(d2);
}
