// Doesn't compile, but that's the point; we use this to produce error messages
// that we can then apply cleanup-template-messages and
// restore-template-messages on.

#include <map>
#include <string>
using std::map;
using std::string;

string func( map<int,string>& themap, int key )
{
    std::find( themap.begin(), themap.end(), "foo" );
    return themap.find(key)->second;
}

int main()
{
    map<int,string> const themap;
    func( themap );
}
