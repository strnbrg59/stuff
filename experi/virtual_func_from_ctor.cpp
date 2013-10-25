struct B {
    B() { i_ = call_pure(); }
    virtual int pure() = 0;
    int call_pure() { return pure(); }
    int i_;
};

struct D : B {
    virtual int pure() { return 17; }
};

int main()
{
    B* d = new D();
}
