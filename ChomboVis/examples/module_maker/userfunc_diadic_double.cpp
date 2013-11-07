#include "userfunc_lib_double.h"
#include <vector>

int
F( std::vector<int> vi, double * x, double * y )
{
    for( int i=0;i<10;++i ) x[i] += Plus10(y[i]);
    return 0;
}
