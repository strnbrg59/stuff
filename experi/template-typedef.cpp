#if 0
#include <map>
template<typename T> struct S1 {};

template<typename T> struct S2 {
    typedef typename std::multimap<T, S1<T> > RepT;
};
int main()
{
    S2<int>* s2 = new S2<int>;
    delete s2;
}
#endif

#include <iostream>
using namespace std;

struct MyStruct {
    void f(int) { cout << "I am MyStruct::f()\n"; }
};

template<typename T> struct FuncTrait {
    typedef void (T::*F)(void);
};

template<typename T> void myfunc(typename FuncTrait<T>::F f)
{
    f();
}

int main()
{
    MyStruct ms;
    myfunc<int>(&(MyStruct::f));
}
