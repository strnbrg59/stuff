#include <iostream>
#include <cstdlib>
#include <deque>
#include <boost/foreach.hpp>
using namespace std;

int main(int argc, char* argv[])
{
    int m = atoi(argv[1]);
    cout << m << '\n';

    deque<int> result;
    do {
        result.push_front(m%2);
        m >>= 1;
    } while(m > 0);

    BOOST_FOREACH(int i, result) {
        cout << i;
    }
    cout << '\n';
}
