#include "rubic.h"
#include "vtk_stuff.h"
#include <cstdlib>
#include <string>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int dim = 3;
    int opt;
    std::string moves = "";
    while (-1 != (opt = getopt(argc, argv, "d:t:")))
    {
        switch(opt)
        {
            case 'd' : dim = atoi(optarg); break;
            case 't' : moves = std::string(optarg); break;
            default : cerr << "Usage: " << argv[0] << " [-ddim] [ops]";
                      exit(2);
        }
    }

    switch(dim)
    {
        case 2 : vtk_stuff<2>(moves); break;
        case 3 : vtk_stuff<3>(moves); break;
        case 4 : vtk_stuff<4>(moves); break;
        // To support more dims, add to the explicit template
        // instantiations at the bottom of vtk_stuff.cxx.
        default : cerr << "dim=" << dim << " unsupported.\n"; exit(1);
    }
}
