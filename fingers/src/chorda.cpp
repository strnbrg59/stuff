#include "fingers.hpp"
#include <cassert>
#include <cstring>
using std::string;
using std::ostream;

double
Chorda::Freq() const
{
    assert(m_rep>=1 && m_rep<=4);
    Note openStrings[4] = {string("a'"), string("d"),
                           string("g,"), string("c,")};
    return openStrings[m_rep-1].Freq();
}


void
Chorda::Print(ostream& out) const
{
    out << m_rep;
}
