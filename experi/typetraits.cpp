#include <boost/type_traits.hpp>


typedef int (*Func)();
template<bool B> int freeFunc() {
    return 0;
}
template<> int freeFunc<true>() {
    return 1;
}

template<typename T> struct S
{
    typedef typename boost::is_integral<T>::value_type is_integral;
    S();
//    Func f;
    template<bool B> int f();
};

template<typename T> S<T>::S()
  : f(freeFunc<is_integral>) {
}


int main()
{
    S<int> si;
    return si.f();
}
