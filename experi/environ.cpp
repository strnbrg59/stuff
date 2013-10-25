#include <iostream>
using namespace std;

int main()
{
    char** var = environ;
    while (*var) {
        cout << *var << '\n';
        ++var;
    }
}
