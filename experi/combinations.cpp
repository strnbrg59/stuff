#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
using namespace std;

template<typename T>
ostream& operator<<(ostream& out, vector<T> const& v)
{
    for (typename vector<T>::const_iterator iter=v.begin(); iter!=v.end(); ++iter) {
        out << *iter << " ";
    }
    return out;
}
    

/**
 * indexes contains k distinct integers on [0,n-1].
 * The first time this function is called, indexes should be {0,1,...,k-1}.
 * The last combination is defined as {n-k,n-k+1,...,n-1}.
 * 
 * As long as indexes is not the last combination, this function "rolls" indexes
 * to the next combination and returns false.
 * Returns true when indexes is the last combination (and indexes is then
 * undefined).
 */
bool next_combination_indexes(unsigned n, unsigned k, vector<unsigned>& indexes)
{
    assert(indexes.size() == k);
    int j=k-1;
    while (indexes[j] >= (n - 1) - (k - 1 - j)) {
        if (j == 0) {
            return true;
        }
        --j;
    }
    indexes[j] ++;
    for (unsigned i=j+1; i<k; ++i) {
        indexes[i] = indexes[i-1] + 1;
    }
    return false;
}

template<typename T>
void print_combinations(unsigned n, unsigned k, vector<T> const& all_n)
{
    vector<unsigned> indexes;
    assert(n>=k);
    assert(k>0);
    vector<T> comb;
    for (unsigned i=0;i<k;++i) {
        indexes.push_back(i);
        comb.push_back(all_n[i]);
    }
    bool done = false;
    do {
        cout << comb << '\n';
        done = next_combination_indexes(n, k, indexes);
        for (unsigned i=0;i<k;++i) {
            comb[i] = all_n[indexes[i]];
        }
    } while (!done);
}

int main(int argc, char** argv)
{
    assert(argc==3);
    vector<char> vc;
    for (int i=0;i<atoi(argv[1]);++i) vc.push_back('a' + i);
    print_combinations(atoi(argv[1]), atoi(argv[2]), vc);
}
    


