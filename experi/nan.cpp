#include <cassert>
#include <cmath>
#include <iostream>
using namespace std;

int main()
{
    double nan1 = log(-5.0);
    double nan2 = log(-5.0);
    cout << "nan1=" << nan1 << '\n';
    cout << "nan2=" << nan2 << '\n';
    assert(nan1 != nan2);
    assert(nan != nan);
}
