#include <stdio.h>

typedef struct S {
    int i;
    const char* str;
} S;

int main()
{

    struct S s1, s2, temp;

    s1.i = 11;
    s1.str = "s1";
    s2.i = 22;
    s2.str = "s2";

    printf("s1=(%d, %s), s2=(%d, %s)\n", s1.i, s1.str, s2.i, s2.str);

    temp = s1;
    s1 = s2;
    s2 = temp;
    printf("s1=(%d, %s), s2=(%d, %s)\n", s1.i, s1.str, s2.i, s2.str);
}
