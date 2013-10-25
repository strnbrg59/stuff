#include <iostream>
using namespace std;

struct B {
    int i_;
    B(int i) : i_(i) {}
    B(B const& that) : i_(that.i_) {}
};
ostream& operator<<(ostream& out, B const& b)
{
    out << b.i_;
    return out;
}

struct D : B {
    int j_;
    D(int i, int j) : B(i), j_(j) {}
    D(D const& that) : B(that), j_(that.j_) {}
};
ostream& operator<<(ostream& out, D const& d)
{
    out << "(" << d.i_ << ", " << d.j_ << ")";
    return out;
}

int main()
{
    D d1(7, 9);
    D d2(d1);
    cout << "d1 = " << d1 << '\n';
    cout << "d2 = " << d2 << '\n';
}
