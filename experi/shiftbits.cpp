#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <cassert>
using namespace std;
typedef unsigned int uint32;

uint32
release_parts_to_number(
        const uint32 parts[4])
{
    for(int i=0; i<4; i++) {
        // sanity check only; these should never be triggered.
        assert( (parts[i] & 0xFFFFFF00) == 0 );
    }
    return ( (parts[0]<<24) | (parts[1]<<16) | (parts[2]<<8) | (parts[3]) );
}


std::string
number_to_release_parts(uint32 number, uint32 parts[4])
{
    parts[0] = number >> 24;
    parts[1] = (number & 0x00FF0000) >> 16;
    parts[2] = (number & 0x0000FF00) >> 8;
    parts[3] = number & 0x000000FF;
    ostringstream ost;
    ost << parts[0] << '.' << parts[1] << '.' << parts[2] << '.';
    if (parts[3] <= 9) {
        ost << parts[3];
    } else {
        char c = parts[3];
        ost << c;
    }
    return ost.str();
}

int main()
{
    uint32 thing_parts[4] = {6,2,1,'c'};
    uint32 thing = release_parts_to_number(thing_parts);
    cout << "Thing = " << thing << '\n';
    cout << number_to_release_parts(thing, thing_parts) << '\n';
}
