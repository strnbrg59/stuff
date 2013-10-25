#include <stdio.h>
#include <getopt.h>

int main(int argc, char** argv)
{
    char c;
    while(-1 != (c = getopt(argc, argv, "a:bc"))) {
        printf("%c = %s\n", c, optarg);
    }

    return 0;
}
