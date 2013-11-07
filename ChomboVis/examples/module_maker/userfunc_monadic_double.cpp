#include "userfunc_lib_double.h"
#include <vector>
#include <iostream>

int
F( std::vector<int> vi, double * x )
{
    for( int i=0;i<10;++i )
    {
        std::cerr << "x[" << i << "]=" << x[i] << std::endl;
        x[i] = Plus10(x[i]);
    }
    return 0;
}

