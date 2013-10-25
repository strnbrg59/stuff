#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

unsigned base = 2;
unsigned log2_base = 1;

unsigned int_pow(unsigned x, unsigned p)
{
    unsigned result = 1;
    for (unsigned i=0;i<p;++i) {
        result *= x;
    }
    return result;
}

string numeric2string(unsigned n)
{
    ostringstream ost;
    vector<unsigned> result;
    for (int i=0;i<4;++i) {
        result.push_back(n % base);
        n >>= log2_base;
    }
    for (int i=0;i<4;++i) {
        ost << result[3-i] << '.';
    }
    return ost.str();
}

unsigned make_int(unsigned digits[4])
{
    unsigned result = 0;
    for (int i=0;i<4;++i) {
        result += int_pow(base,i) * digits[3-i];
    }
    return result;
}

int main()
{
    unsigned digits[4] = {1, 0, 1, 0};
    unsigned int_ver = make_int(digits);
    cout << "int_ver = " << int_ver << '\n';
    string version = numeric2string(int_ver);
    cout << version << '\n';
}
