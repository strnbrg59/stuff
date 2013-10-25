#include <cstdio>
#include <iostream>
using std::cout;

int main()
{
    char buf[256];

    FILE* output = popen("/bin/ls /", "r");
    while (!feof(output)) {
        fgets(buf, 256, output);
        cout << buf;
    }
    pclose(output);
}
