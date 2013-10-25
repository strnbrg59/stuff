#include <cassert>
struct Foo
{
    int func(int n=0) {
        static int s_n;
        if (n != 0) {
            s_n = n;
        }
        return s_n;
    }
};

int main()
{
    Foo foo1, foo2;
    foo1.func(37);
    assert(foo2.func(0) == foo1.func(0));
}
