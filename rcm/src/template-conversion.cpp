// This is an experiment with implicit type conversion between
// template types.  In main(), it doesn't work if we don't explicitly
// convert that f to a Bar<int>.  Yet if this were not templated code,
// there would be no problem!

#include <iostream.h>

template<class T> class Foo
{
public:
    Foo(T i) { fi=i; }
    operator Bar() { return Bar<T>(fi); }
    T fi;
};

template<class T> class Bar
{
public:
    Bar(T i) { bi=i; }
//  Bar(Foo<T> f) { bi=f.fi; }
    T bi;
};

template<class T> void barfunc(Bar<T> b)
{
    cout << b.bi << '\n';
} 

Bar<int>::Bar(Foo<int> f) { bi=f.fi; }

main()
{
    Foo<int> f(6);
    Bar<int> b(7);
    barfunc(f);
//  barfunc<int>(Bar<int>(Foo<int>(3)));
//  barfunc(Bar<int>(f));
}
