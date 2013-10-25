#include <sstream>
#include <iostream>
#include <string>
using namespace std;

class Anything
{
public:
    explicit Anything(int i) {
        ostringstream out;
        out << i;
        m_rep = out.str();
    }
    explicit Anything(string s) : m_rep(s) { }

    operator int() const { return atoi(m_rep.c_str()); }
    operator string() const { return m_rep; }
private:
    string m_rep;
};


Anything favoriteNumber() { return Anything(13); }
Anything favoriteWord() { return Anything(string("roadtrip")); }


int main()
{
    int i = favoriteNumber();
    string h = favoriteWord();

    cout << "i=" << i << '\n';
    cout << "h=" << h << '\n';
}
