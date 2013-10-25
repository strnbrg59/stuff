#include <set>
#include <string>
#include <iostream>
using namespace std;

template<typename T> class ListOf
{
public:
    ListOf(T a)
    {
        rep_.insert(a);
    }

    ListOf& operator()(T a)
    {
        rep_.insert(a);
        return *this;
    }

    operator set<T>() { return rep_; }

private:
    set<T> rep_;
};

template<typename T> ListOf<T> listof(T t)
{
    return ListOf<T>(t);
};

int main()
{
    set<int> msi = listof(1)(2)(3)(4);
    for(set<int>::const_iterator i=msi.begin(); i!=msi.end(); ++i) {
        cout << *i << " ";
    }
    cout << '\n';

    set<string> mss = listof(string("foo"))("bar")("baz");
    for(set<string>::const_iterator i=mss.begin(); i!=mss.end(); ++i) {
        cout << *i << " ";
    }
    cout << '\n';
}
