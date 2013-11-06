/* Do random moves from a list.  Stop when cube is once again unscrambled. */

#include "rubic.h"
#include "primes.h"
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <set>
#include <iostream>
using namespace std;

bool g_report_unique_permutations = false;

int urand(int lo, int hi)
{
    int result = lo + rand()%(hi-lo);
    return result;
}

string randomMove(vector<string> const& moves)
{
    string result = moves[urand(0,moves.size())];
    return result;
}

template<int N> void doit(int seed, string moves)
{
    Primes primes;
    vector<string> moves_vector;
    char* buf = new char[moves.size()+1];
    strcpy(buf, moves.c_str());
    char* tok = strtok(buf, ",");
    while (tok)
    {
        moves_vector.push_back(tok);
        tok = strtok(NULL, ",");
    }

    set< Permutation<N> > unique_permutations;
    unsigned n_unique_permutations = 0;
    srand(seed);
    Permutation<N> I;
    Permutation<N> p;
    p.permute(randomMove(moves_vector));
    unsigned i=1;
    while (p != I) {
        p.permute(randomMove(moves_vector));
        if (g_report_unique_permutations)
        {
            unique_permutations.insert(p);
            if (unique_permutations.size() > n_unique_permutations)
            {
                ++n_unique_permutations;
            }
        }
        ++i;
        if (i%100000 == 0)
        {
            cerr << '.';
            if (g_report_unique_permutations)
            {
                cout << "total permutations=" << i << ", unique="
                     << 100.0*n_unique_permutations/i << '\n';
            }
        }
    }
    cout << "total permutations=" << i << "=(" << primes.find_factors(i)
         << ")";
    if (g_report_unique_permutations)
    {
        cout << ", unique permutations=" << n_unique_permutations;
    }
    cout << '\n';
}

int main(int argc, char* argv[])
{
    int dim;
    int seed;
    string moves;
    int opt;

    while (-1 != (opt = getopt(argc, argv, "d:s:t:u")))
    {
        switch(opt)
        {
            case 'd' : dim = atoi(optarg); break;
            case 's' : seed = atoi(optarg); break;
            case 't' : moves = string(optarg); break;
            case 'u' : g_report_unique_permutations = true; break;
            default : cerr << "Usage: " << argv[0] << " -ddim -sseed -tturns \n"
                           << "where turns is, e.g., 'RU,ru,F'" << '\n';
                      exit(2);
        }
    }


    switch(dim)
    {
        case 2 : doit<2>(seed, moves); break;
        case 3 : doit<3>(seed, moves); break;
        default : std::cerr << "Dimension " << dim << " not supported.\n";
    }
}
