#include <assert.h>
#include "c.h"

int main(int argc, char* argv[])
{
    c_struct_array csa;
    c_struct_array_new(&csa);
    assert(csa);

    return 0;
}
