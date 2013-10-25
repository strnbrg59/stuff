#include <list>
#include <iostream>
#include <boost/assign/list_of.hpp>

using namespace std;

bool is_even(int i)
{
    return i%2;
}


int main()
{
    list<int> li = boost::assign::list_of(1)(2)(3)(4)(5);
    for(list<int>::iterator i=li.begin(); i!=li.end(); ++i) {
        cout << *i << ' ';
    }
    cout << '\n';

    li.remove_if(is_even);

    for(list<int>::iterator i=li.begin(); i!=li.end(); ++i) {
        cout << *i << ' ';
    }
    cout << '\n';
}
