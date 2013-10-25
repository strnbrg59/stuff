#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#define BLKSIZE 8

int main()
{
    int err=0;
    int childpid=0;
    int fd[2];
    char buf[BLKSIZE];
    int bytesread=0;

    assert(!pipe(fd));
    if ((childpid = fork()) == 0) { // child
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execlp("/home/strnbrg/usr/local/scripts/count.sh", "count.sh", NULL);
        perror("The exec of count.sh failed.");
    } else if (childpid > 0) { // parent
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        while ((bytesread = read(STDIN_FILENO, buf, BLKSIZE)) > 0) {
            printf("%s\n", buf);
        }
    } else assert(0);

    return 0;
}

