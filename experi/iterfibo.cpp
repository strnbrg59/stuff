#include <iostream>
#include <cstdlib>
using namespace std;

int fibo(int n)
{
    int a_2 = 1;
    int a_1 = 1;
    int a;
    if(n < 2) return 1;

    for(int i=2; i<=n; ++i)
    {
        a = a_1 + a_2;
        a_2 = a_1;
        a_1 = a;
    }

    return a;
}

int main(int argc, char** argv)
{
    cout << fibo(atoi(argv[1])) << '\n';
}

