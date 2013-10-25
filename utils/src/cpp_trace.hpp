// Print out any line of code, before executing it.

#include <stdio.h>
#ifdef NDEBUG
#define TRACE(s) s
#else
#define TRACE(s)  printf("%s\n", (#s)); s
#endif

#ifdef UNIT_TEST
void hello(char const * msg) { printf("%s\n", msg); }

int
main(int argc, char *argv)
{
   int i = 0;
   TRACE(i);
   TRACE(hello("Hello, world!"));
   return EXIT_SUCCESS;
}
#endif
