#include <iostream>
using namespace std;
#include <boost/type_traits/is_integral.hpp>

typedef int (*Func)();

template<bool B>
struct freeFunc
{
    static const Func free_func;
};
template <> struct freeFunc<false>
{
    static void free_func();
};
template <> struct freeFunc<true>
{
    static void free_func();
};

template <>
const void freeFunc<false>::free_func() { cout << "freeFunc<false>" << endl; }
template <>
const void freeFunc<true>::free_func() { cout << "freeFunc<true>" << endl; }

template<typename T>
struct S
{
     S();
     Func f;
};

template<typename T> S<T>::S()
    :
    f(freeFunc<boost::is_integral<T>::value>::free_func) {
}


int main()
{
     S<int> si;
     return si.f();
}
