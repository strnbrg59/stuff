#include <set>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>
#include <boost/assign/list_of.hpp>
using namespace std;

#define VERSION_FROM_BYTES(major, minor, micro, eng)                    \
    (((major) << 24) | ((minor) << 16) | ((micro) << 8) | ((eng << 0))) \

typedef unsigned uint32;

std::string
number_to_release_parts(uint32 number, uint32 parts[4]=0)
{
    uint32 temp_parts[4];
    temp_parts[0] = number >> 24;
    temp_parts[1] = (number & 0x00FF0000) >> 16;
    temp_parts[2] = (number & 0x0000FF00) >> 8;
    temp_parts[3] = number & 0x000000FF;
    std::ostringstream ost;
    ost << temp_parts[0] << '.' << temp_parts[1] << '.' << temp_parts[2] << '.';
    if (temp_parts[3] <= 9) {
        ost << temp_parts[3];
    } else {
        char c = temp_parts[3];
        ost << c;
    }
    if (parts) {
        memcpy(parts, temp_parts, 4*sizeof(uint32));
    }
    return ost.str();
}

struct VersionPair {
    VersionPair(uint32 rios, uint32 rsp) : rios_(rios), rsp_(rsp) {}
    uint32 rios_;
    uint32 rsp_;
};
bool operator<(VersionPair const& vp1, VersionPair const& vp2)
{
    return vp1.rios_ < vp2.rios_;
}

uint32 required_rsp(uint32 rios, bool* not_sure)
{
    *not_sure = false;
    typedef vector<VersionPair> VersionVect;

    // This information comes from the NBT/RSPInstallationImages twiki page.
    // The right column is the the RSP version that goes with RiOS versions
    // less than the left column (of the same row) but greater than the left
    // column of the previous row.
    uint32 const off_the_scale = VERSION_FROM_BYTES(0xFF,0,0,0);
    VersionVect vpi = boost::assign::list_of
      (VersionPair(VERSION_FROM_BYTES(5,5,0,0),   0))
      (VersionPair(VERSION_FROM_BYTES(5,5,6,0),   VERSION_FROM_BYTES(5,5,0,0)))
      (VersionPair(VERSION_FROM_BYTES(5,5,8,0),   VERSION_FROM_BYTES(5,5,6,0)))
      (VersionPair(VERSION_FROM_BYTES(6,0,0,0),   VERSION_FROM_BYTES(5,5,8,0)))
      (VersionPair(VERSION_FROM_BYTES(6,0,2,0),   VERSION_FROM_BYTES(6,0,0,0)))
      (VersionPair(VERSION_FROM_BYTES(6,1,0,0),   VERSION_FROM_BYTES(6,0,2,0)))
      (VersionPair(VERSION_FROM_BYTES(6,5,0xFF,0),VERSION_FROM_BYTES(6,1,0,0)))
      (VersionPair(off_the_scale,                 VERSION_FROM_BYTES(6,1,0,0)));
    sort(vpi.begin(), vpi.end());
/*
    for (unsigned i=0;i<vpi.size();++i) {
        cout << "vpi[" << i << "]=" << vpi[i].rios_ << '\n';
    }
*/
    VersionVect::iterator iter = vpi.begin();
    if (rios < iter->rios_) {
        return 0;
    }
    while (rios >= iter->rios_) {
        ++iter;
//      cout << "iter = " << iter->rios_ << '\n';
    }
    if (iter->rios_ == off_the_scale) {
        *not_sure = true;
    }

    return iter->rsp_;
}

int main(int argc, char** argv)
{
    bool not_sure;
    uint32 rios = VERSION_FROM_BYTES(
            atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
    cout << "rios=" << rios << ", " << "rsp=" <<
        number_to_release_parts(
            required_rsp(rios, &not_sure));
    cout << " not_sure=" << not_sure << '\n';
}
