#include <stdio.h>
#include <string.h>

int main( int argc, char * argv[] )
{
    int debug = 0;
    int s;
    char **cp, *path;
    extern char *ipcpath();

    if( argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        argv++;
        debug = 1;
    }
    if( argc < 3 )
    {
        fprintf(stderr, "usage:scanports ipchost port...\n");
        return 99;
    }

    for( cp=&argv[2]; *cp; cp++)
    {
        path = ipcpath(argv[1], "tcp", *cp);
        if( debug) fprintf(stderr, "%s\n", path);
        s = ipcopen(path, "");
        if( s<0)
        {
            if( debug) ipcperror("scanports");
            continue;
        }
        printf("%s ", *cp);
        close(s);
    }
    printf("\n");
    return 0;
}
