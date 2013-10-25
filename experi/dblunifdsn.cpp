#include <iostream>
#include <cstdlib>
using namespace std;

int main()
{
    int const N = 1000000;
    for(int i=0;i<N;++i) {
        int a = 1+ random()%200;
        cout << random()%a << '\n';
    }
}
