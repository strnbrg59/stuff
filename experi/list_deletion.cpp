#include <cassert>
#include <list>
#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
using namespace std;

bool equals_zero(int i) { return i==0; }

int main()
{
    list<int> stages = boost::assign::list_of(1)(0)(3)(0)(5)(0);
    
    BOOST_FOREACH(int& s, stages) {
        cout << s << ' ';
    }
    cout << '\n';

    list<int>::iterator new_last = remove_if(stages.begin(), stages.end(), 
                                             boost::bind(equal_to<int>(), 0, _1));
    stages.erase(new_last, stages.end());
    BOOST_FOREACH(int& s, stages) {
        cout << s << ' ';
    }
    cout << '\n';


    cout << "Backwards: ";
    for (list<int>::reverse_iterator iter = stages.rbegin();
         iter != stages.rend(); ++iter) {
        cout << *iter << " ";
    }
    cout << '\n';

    list<int>::iterator l1 = stages.begin();
    list<int>::iterator l2 = ++l1;
    cout << "l1 = " << *l1 << '\n';
    cout << "l2 = " << *l2 << '\n';

    list<int>::reverse_iterator ri = stages.rbegin();
    cout << stages.front() << '\n';
    assert(*ri != stages.front());
}
