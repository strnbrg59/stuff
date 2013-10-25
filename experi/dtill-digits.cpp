#include <cassert>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>

using namespace std;

/**
 * indexes contains k distinct integers on [0,n-1].
 * The last combination is defined as {n-k,n-k+1,...,n-1}.  So if you want to
 * run through all n-choose-k combinations, make sure indexes is initially
 * {0,1,...,k-1}.
 *
 * As long as indexes is not the last combination, this function "rolls" indexes
 * to the next combination and returns false.
 * Returns true when indexes is the last combination (and indexes is then
 * undefined).
 */
bool next_combination_indexes(unsigned n, unsigned k, vector<int>& indexes)
{
    assert(n >= k);
    assert(k > 0);
    assert(indexes.size() == k);

    int j=k-1;
    while (indexes[j] >= n - (k - j)) {
        if (j == 0) {
            return true;
        }
        --j;
    }
    indexes[j] ++;
    for (int i=j+1; i<k; ++i) {
        indexes[i] = indexes[i-1] + 1;
    }
    return false;
}

ostream& operator<<(ostream& out, vector<int> const& v)
{
    for (vector<int>::const_iterator i = v.begin(); i!=v.end(); ++i) {
        out << *i << ' ';
    }
    return out;
}
ostream& operator<<(ostream& out, set<int> const& v)
{
    for (set<int>::const_iterator i = v.begin(); i!=v.end(); ++i) {
        out << *i << ' ';
    }
    return out;
}

vector<int> digits(int n)
{
    int residual = n;
    int digit;
    vector<int> result;
    while (residual > 0) {
        digit = residual % 10;
        result.push_back(digit);
        residual = (residual-digit)/10;
    }
    // Will be backwards but doesn't matter here.
    return result;
}

int main()
{
    vector<int> lhs;
    for (int i=0;i<5;++i) {
        lhs.push_back(i);
    }

    do {
        do {
            int f1 = lhs[0]*100 + lhs[1]*10 + lhs[2];
            int f2 = lhs[3]*10 + lhs[4];
            int prod = f1*f2;

            vector<int> rhs(digits(prod));

            set<int> slhs(lhs.begin(), lhs.end());
            set<int> srhs(rhs.begin(), rhs.end());

            vector<int> intersection;
            intersection.reserve(5);
            set_intersection(slhs.begin(), slhs.end(), srhs.begin(), srhs.end(),
                             back_inserter(intersection));
            cout << f1 << "*" << f2 << " = " << prod
                 << " : intersection.size()=" << intersection.size();

            if ((rhs.size() != 4) || (srhs.size() != 4)) {
                cout << " ...but rhs is invalid";
            }
            cout << '\n';
        } while (next_permutation(lhs.begin(), lhs.end()));
    }
    while (!next_combination_indexes(10, 5, lhs));
}
