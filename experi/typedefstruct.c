#include <stdio.h>

typedef struct Foo
{
    int i;
    int j;
} g_foo;

int main()
{
/*
    struct Foo foo = {17,19};
    printf("(%d, %d)\n", foo.i, foo.j);    
*/
    g_foo h_foo = {171, 192};
    printf("(%d, %d)\n", h_foo.i, h_foo.j);    

    return 0;
}
