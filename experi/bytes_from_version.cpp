#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
using namespace std;
typedef unsigned int uint32;

uint32 version_from_bytes(int major, int minor, int micro, int eng)
{
    return ((major) << 24) | ((minor) << 16) | ((micro) << 8) | ((eng << 0));
}

string bytes_from_version(uint32 v) {
    ostringstream result;
    result << (v >> 24) << '.' << ((v >> 16) & 0xFF) << '.'
           << ((v >> 8) & 0xFF) << '.' << (v & 0xFF);
    return result.str();
}

int main(int argc, char** argv)
{
    uint32 v = version_from_bytes(atoi(argv[1]), atoi(argv[2]),
                                  atoi(argv[3]), atoi(argv[4]));

    cout << "(v >> 24) = " << (v >> 24) << '\n';
    cout << "(v >> 16) = " << (v >> 16) << '\n';
    cout << "(v >>  8) = " << (v >>  8) << '\n';

    cout << bytes_from_version(v) << " = " << v << '\n';
    printf("08X = %08X\n", v);
}
