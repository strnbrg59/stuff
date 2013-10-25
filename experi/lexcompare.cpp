#include <cassert>
#include <string>
#include <iostream>
using namespace std;

struct MS
{
    MS(int ii, int jj, int kk) : i(ii), j(jj), k(kk) {}
    int i;
    int j;
    int k;
    bool operator<(MS const& that) {
        return  (i < that.i)
            ||  ((i == that.i) && (j < that.j))
            ||  ((i == that.i) && (j == that.j) && (k < that.k));
    }
};


int main()
{
    string s1("a"), s2("b"), s3("a");
    assert(s1 < s2);
    assert(s2 > s1);
    assert(!(s3 < s1));
}
