#include <set>
#include <string>
#include <cassert>
#include <algorithm>
using namespace std;

int main()
{
        set<string> myset;
        myset.insert("foo");
        myset.insert("bar");
        assert(std::find(myset.begin(), myset.end(), string("baz")) == myset.end());
}
