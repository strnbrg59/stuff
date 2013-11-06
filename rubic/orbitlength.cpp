#include "rubic.h"
#include <cassert>
#include <iostream>
using namespace std;

template<int N> int doit(std::string moves)
{
    int orbit = 1;
    Permutation<N> I;
    Permutation<N> p;
    p.permute(moves);
    while (p != I) {
        p.permute(moves);
        ++orbit;
    }
    return orbit;
}

int main(int argc, char* argv[])
{
    int dim;
    int opt;
    std::string moves;

    while (-1 != (opt = getopt(argc, argv, "d:t:")))
    {
        switch(opt)
        {
            case 'd' : dim = atoi(optarg); break;
            case 't' : moves = std::string(optarg); break;
            default : cerr << "Usage: " << argv[0] << " [-ddim] [-tops]" ;
                      exit(2);
        }
    }


    switch(dim)
    {
        case 2 : std::cout << doit<2>(moves) << '\n'; break;
        case 3 : std::cout << doit<3>(moves) << '\n'; break;
        case 4 : std::cout << doit<4>(moves) << '\n'; break;
        default : std::cerr << "Dimension " << dim << " not supported.\n";
    }
}
