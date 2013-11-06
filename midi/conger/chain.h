/* chain.h   header file for chain.c */

    
struct strchain {
    char *line;
    struct strchain *next;
};

struct strchain *inpchain(), *chain(), *chainalloc();
char *strsave();

