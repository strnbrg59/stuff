#include "userfunc_lib_float.h"
#include <iostream>
#include <vector>
#include "utils/Misc.h"

int
F( std::vector<int> vi, float * x, float * y )
{
    std::cerr << "vector=" << vi << std::endl;
    for( int i=0;i<10;++i ) x[i] += Plus10(y[i]);
    return 0;
}
