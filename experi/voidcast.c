#include <stdlib.h>

typedef struct tstring {
    char* rep;
} tstring;

int main()
{
    tstring* var=NULL;
    (void)*(var);
    free(var);
    return 0;
}
