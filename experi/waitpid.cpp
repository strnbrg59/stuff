#include <sys/wait.h>
#include <iostream>
#include <errno.h>
#include <cstdio>
using namespace std;

int main()
{

    int status;
    pid_t pid = fork();

    if (pid == 0) {
        sleep(1);
        int err = execl("./returnval", "returnval", "0", NULL);
        perror("execl");
    } else  if (pid > 0) {
        pid_t retpid = waitpid(pid, &status, 0);
        perror("waitpid");
    } else {
        perror("fork");
    }
    cout << "status=" << status << '\n';
}
