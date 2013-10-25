#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void xorstring(char const* txt, char const* key, int len, char** result)
{
    int i;
    for (i=0;i<len;i++) {
        (*result)[i] = txt[i] ^ key[i];
    }
}

void prettybinary(char const* binary, int len)
{
    int i;
    printf("{");
    for(i=0;i<len;i++) {
        printf("%d, ", binary[i]);
    }
    printf("}\n");
}

int main()
{
    char const* plain = "Over the river and through the woods...";
    char const* key   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM";
    assert(strlen(plain) == strlen(key));
    char *result = (char*)calloc(strlen(plain),1);
    char *resultchk = (char*)calloc(strlen(plain),1);

    xorstring(plain, key, strlen(plain), &result);
    prettybinary(result, strlen(plain));
    xorstring(result, key, strlen(plain), &resultchk);
    printf("resultchk=%s\n", resultchk);

    char stage1[] = {46, 20, 6, 22, 69, 18, 15, 13, 73, 24, 2, 26, 8, 28, 79, 17, 31, 22, 83, 0, 29, 4, 24, 13, 30, 18, 97, 54, 43, 33, 101, 49, 40, 39, 45, 57, 101, 98, 99 };
    xorstring(stage1, key, strlen(plain), &resultchk);
    printf("resultchk=%s\n", resultchk);

    return 0;
}
