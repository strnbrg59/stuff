#include "delfunc.h"
#include <iostream>
using namespace std;


template<typename T> struct c_auto_ptr
{
    typedef int (*FreeFuncType)(T);

    c_auto_ptr(T t, FreeFuncType f) : m_rep(t), m_freeFunc(f) { }
    c_auto_ptr(T t) : m_rep(t), m_freeFunc(delFunc<T>) { }
    ~c_auto_ptr();

    T m_rep;
    FreeFuncType const m_freeFunc ;
};

template<typename T> c_auto_ptr<T>::~c_auto_ptr()
{
    m_freeFunc(m_rep);
}
