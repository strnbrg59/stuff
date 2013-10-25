#include <boost/foreach.hpp>
#include <iostream>
using std::cout;

int main()
{
    const char* dhcp_nodes[2] = {"/dhcp", "/dhcp_dyn_dns"};
    BOOST_FOREACH(const char* dhcp, dhcp_nodes) {
        cout << "|" << dhcp << "|\n";
    }
}
