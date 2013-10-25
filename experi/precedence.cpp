#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
    int i = 1000
          + (argc % 2) ? 1 : 0;
    cout << i << '\n';
}
