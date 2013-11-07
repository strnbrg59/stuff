#include "userfunc_lib_float.h"
#include <vector>

int
F( std::vector<int> vi, float * x )
{
    for( int i=0;i<10;++i ) x[i] = Plus10(x[i]);
    return 0;
}

