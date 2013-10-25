#include <assert.h>
#include <stdio.h>

typedef struct {
    int i;
    double x;
} S_t;

S_t s = {.x = 3.14, .i=23};


struct Foo_t;

void func(struct Foo_t*);

typedef struct
{
    int i;
} Foo_t;

void func(struct Foo_t* f)
{
    assert(f);
}

typedef struct Goo
{
    int i;
} Goo;


void gunc(struct Goo* g);

void gunc(struct Goo* g)
{
    assert(g);
}

int main()
{
    printf("%d %f\n", s.i, s.x);
    return 0;
}
