#include <iostream>
#include <complex>
#include <boost/numeric/interval.hpp>
using namespace std;
using boost::numeric::interval;

struct MyComplex
{
    MyComplex(double x, double y) : rep_(x,y) {}
    double real() const { return rep_.real(); }
    double imag() const { return rep_.imag(); }
    bool operator<<(MyComplex const& that)
    {
        return this->real() < that.real() 
            && this->imag() < that.imag();
    }
    std::complex<double> rep_;
};

int main()
{
    MyComplex c1(2.7, 3.1);
    MyComplex c2(1.1, 2.9);
    cout << "c2 << c1 = " << (c2 << c1) << '\n';

    interval<double> i1(2.0, 4.0);
    interval<double> i2(2.5, 3.0);
    cout << subset(i2, i1) << '\n';
    interval<double> ratio = i1/i2;
    cout << "ratio = (" << ratio.lower() << ", " << ratio.upper() << ")\n";
}
