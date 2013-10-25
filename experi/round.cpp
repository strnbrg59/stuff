#include <cstdlib>
#include <iostream>
using namespace std;

/* Round down to nearest multiple of r. */
int round(int a, int r)
{
    return a - a%r;
}

int main(int argc, char* argv[])
{
    cout << round(atoi(argv[1]), atoi(argv[2])) << '\n';
}
