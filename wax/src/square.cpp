#include "square.hpp"
#include <string>

Square::Square(int rank, int file)
  : m_rep(rank,file)
{
}

bool
Square::operator==(Square const & that) const
{
    return (m_rep == that.m_rep);
}
bool
Square::operator!=(Square const & that) const
{
    return !operator==(that);
}


std::string
Square::Algebraic() const
{
    char letters[] = {'a','b','c','d','e','f','g','h','i'};
    char numbers[] = {'1','2','3','4','5','6','7','8','9'};
    char result[3];
    result[0] = letters[File()];
    result[1] = numbers[Rank()];
    result[2] = 0;
    return result;
}


std::ostream&
operator<<(std::ostream & o, Square const & square)
{
    o << square.Rank() << square.File();
    return o;
}
