#include "cmdline.hpp"
#include "fingers.hpp"
#include <ostream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <string>
#include <cstring>
#include <map>
using namespace std;

class Note2FreqTable
{
public:
    Note2FreqTable();
    double operator[](string str) const;
private:
    struct StringComparer
    {
        int operator()(string const& s1, string const& s2) const
        {
            if(s1.size() == s2.size())
            {
                return s1.compare(s2) < 0;
            } else
            {
                return int(s1.size()) < int(s2.size());
            }
        }
    };
    typedef map<string, double, StringComparer> MapT;

    MapT m_rep;
};


Note2FreqTable::Note2FreqTable()
{
    double aFreq = CmdlineFactory::TheCmdline().AFreq()/2;
    // Ignore other octaves.
    int diatonicDegrees[7] =   {0,   2,   3,   5,   7,   8,   10};
    string diatonicNames[7] = {"a", "b", "c", "d", "e", "f", "g"};
    for(int i=0;i<7;++i)
    {
        string accids[3] = {string("@"), string(""), string("#")};
        for(int j=0;j<3;++j)
        {
            m_rep[diatonicNames[i] + accids[j]] =
                aFreq * pow(2, (diatonicDegrees[i]+j-1)/12.0);
        }
    }
}
    

double
Note2FreqTable::operator[](string str) const
{
    MapT::const_iterator iter = m_rep.find(str);
    assert(iter != m_rep.end());
    return iter->second;
}


Note::Note(string name)
  : m_rep(name)
{
}


Note::Note(Note const& n)
  : m_rep(n.m_rep)
{
}


void
Note::Print(ostream& out) const
{
    out << setw(2) << left << m_rep;
}


unsigned countChar(char c, string str)
{
    unsigned result = 0;
    string::size_type iter = str.find(c);
    while(iter < str.size())
    {
        ++result;
        iter = str.find(c, iter+1);
    }
    return result;
}


double Note::Freq() const
{
    static Note2FreqTable table;
    unsigned octavesUp = countChar('\'', m_rep);
    unsigned octavesDown = countChar(',', m_rep);
    string withoutOctaves(m_rep, 0, 
        strpbrk(m_rep.c_str(), "\',") - m_rep.c_str());
    return table[withoutOctaves] 
          * pow(2, float(octavesUp)-float(octavesDown));
}
