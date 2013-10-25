#include <cassert>
#include <iostream>
using std::cout;

template<typename T> struct CountStars
{
    enum {val = 0};
};

template<typename T> struct CountStars<T*>
{
    enum {val = 1 + CountStars<T>::val};
};

int main()
{
    typedef double*** TripleDouble;
    typedef char***** QuintupleChar;

    assert(CountStars<TripleDouble>::val == 3);
    assert(CountStars<QuintupleChar>::val == 5);
}
