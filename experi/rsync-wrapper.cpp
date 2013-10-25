#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
using namespace std;

int main(int argc, char** argv)
{
    typedef char* CHARSTAR;
    CHARSTAR* child_argv = new CHARSTAR[argc+2];
    for (int i=0;i<argc;++i) {
        if (string(argv[i]) == "-aq") {
            child_argv[i] = "-a";
        } else {
            child_argv[i] = argv[i];
        }
    }
    child_argv[argc] = "--progress";
    child_argv[argc+1] = 0;
    int fd = open("/home/strnbrg/rsync-wrapper.out",
        O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) { perror("Error"); }
/*
    dup2(fd, 1);
    close(fd);
*/
    execv("/home/strnbrg/testme", child_argv);
/*
    execl("/home/strnbrg/testme", "/home/strnbrg/testme", "-v", "--progress",
          "/home/strnbrg/balcone.jpg", "mom:", 0);
*/
/*
    execl("/usr/bin/rsync", "/usr/bin/rsync",
          "-v", "--progress", "--bwlimit=10",
          "/home/strnbrg/balcone.jpg", "mom:", 0);
*/
}
