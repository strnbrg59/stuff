#include <iostream>
#include <cstdlib>
#include <unistd.h>
using namespace std;

int main()
{
    pid_t pid = fork();
    if (pid == -1 ) {
        cerr << "fork() error\n";
    } else
    if (pid > 0) {
        exit(0);
    } else {
        pid_t ret = setsid();
        if (ret==-1) {
            cerr << "setsid() error, returned " << ret << "\n"; exit(1);
        }
        pid = fork();
        if (pid < 0) { cerr << "Second fork failed\n"; exit(pid); }
        if (pid > 0) { exit(0); }
        system("rsync --progress ~tsternberg/public_html/foos-graphviz.gif root@bravo-sh83: 2>&1 | tee temp.out");
    }
}
