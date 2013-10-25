#include <iostream>
using namespace std;

#define TRACEME(X) X { cout << #X << '\n';

int
TRACEME(myfunc(int i))
return i;
}

int main()
{
    return myfunc(17);
}
