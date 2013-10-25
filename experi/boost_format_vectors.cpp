#include <iostream>
#include <string>
#include <vector>
#include <boost/format.hpp>
using namespace std;

int main()
{
    vector<int> vi;
    for (int i=0;i<5;++i) vi.push_back(i*i);


    // This doesn't compile (though Ali thought it just might).
    // cout << (boost::format("%5s ") % vi) << '\n';
}
