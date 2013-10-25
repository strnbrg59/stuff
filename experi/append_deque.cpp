#include <vector>
#include <iostream>
#include <boost/assign/list_of.hpp>
using namespace std;

int main()
{
    vector<int> vi1 = boost::assign::list_of<int>(1)(2)(3);
    vector<int> vi2 = boost::assign::list_of<int>(4)(5)(6);
    vector<int> viboth(vi1.begin(), vi1.end());
    for (int i=0;i<100;++i)
    {
        viboth.insert(viboth.end(), vi2.begin(), vi2.end());
    }

    for (vector<int>::iterator i = viboth.begin(); i != viboth.end(); ++i)
    {
        cout << *i << ' ';
    }
    cout << '\n';
}

