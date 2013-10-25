#include "delfunc.h"
#include <iostream>
using namespace std;

template<typename T> int delFunc(T);
template<> int delFunc(int i) { cout << "delfunc<int>" << endl; }
template<> int delFunc(float f) { cout << "delfunc<float>" << endl; }
