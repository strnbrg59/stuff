#include "tokenizer.hpp"
#include <sstream>
#include <cassert>
using namespace std;

Tokenizer::Tokenizer(string str)
  : _rep(str)
{
}

/** Appends to v */
void
Tokenizer::operator()(vector<string>& v) const
{
    stringstream ss(_rep);
    string tok;
    while( ss >> tok )
    {
        v.push_back( tok );
    }
}

#ifdef UNITTEST
#include <iostream>
using namespace std;
int main()
{
    string str("Amber  Ashley  5 Heather Tiffany 3");
    vector<string> result;
    (Tokenizer(str))(result);
    assert( result.size() == 6 );
    for(vector<string>::const_iterator i=result.begin();
        i!=result.end(); ++i)
    {
        cout << "|" << *i << "|\n";
    }
}
#endif
