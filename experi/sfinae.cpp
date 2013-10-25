#include <iostream>
using namespace std;
#include <boost/type_traits.hpp>

struct CXXStruct
{
};

template<typename T>
class IsClassT
{
private:
    typedef char One;
    typedef struct { char a[2]; } Two;
    template<typename C> static One test(int C::*);
    template<typename C> static Two test(...);
public:
    enum { Yes = sizeof(IsClassT<T>::template test<T>(0)) == 1 };
};

int main()
{
//  cout << IsClassT<CXXStruct>::Yes << '\n';
    cout << boost::is_integral<CXXStruct>::value << '\n';
    cout << boost::is_integral<int>::value << '\n';
    cout << boost::is_scalar<int*>::value << '\n';
    cout << boost::is_scalar<CXXStruct>::value << '\n';
}
