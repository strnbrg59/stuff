#include <unistd.h>
#include <iostream>
using namespace std;

void foo(void* p)
{
    cout << sizeof(p) << '\n';
}

int main()
{
    pid_t p = getpid();
    cout << sizeof(pid_t) << '\n';
    int* pi = &p;
    foo(pi);
}
