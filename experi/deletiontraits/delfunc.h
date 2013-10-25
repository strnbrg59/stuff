#ifndef INCLUDED_DELFUNC_H
#define INCLUDED_DELFUNC_H

#include <iostream>
using std::cout;
using std::endl;

template<typename T> int delFunc(T)
{
    std::cout << "delFunc<T> (generic)\n";
}

class MyClass {};
template<> static int delFunc(MyClass mc) { cout << "delfunc<MyClass>" << endl; }

#endif
