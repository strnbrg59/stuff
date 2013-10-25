#include <iostream>
#include <cassert>
using namespace std;

struct B {
    B(int i) : i_(i) {}
    virtual bool operator==(B const& that) {
        return i_ == that.i_;
    }

    virtual B& operator=(B const& that) {
        i_ = that.i_;
        return *this;
    }

    int i_;
};

struct D : B {
    D(int i, int j) : B(i), j_(j) {}
    virtual bool operator==(B const& that) {
        D const* downcast = dynamic_cast<D const*>(&that);
        return B::operator==(that) &&
               (j_ == downcast->j_);
    }

    virtual D& operator=(D const& that) {
        D const* downcast = dynamic_cast<D const*>(&that);
        B::operator=(that);
        j_ = downcast->j_;
        return *this;
    }

    int j_;
};

ostream& operator<<(ostream& out, D const& d)
{
    out << "[" << d.i_ << ", " << d.j_ << "]";
    return out;
}

int main()
{
    D d1(11,12);
    D d2(11,12);
    D d3(11,13);

    assert(d1 == d2);
    assert(!(d1 == d3));

    D d4(0,0);
    d4 = d1;
    cout << "d1 = " << d1 << ", d4 = " << d4 << '\n';

    assert(d4 == d1);
}
