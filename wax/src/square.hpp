#ifndef _INCLUDED_SQUARE_H_
#define _INCLUDED_SQUARE_H_

#include <string>
#include <fstream>

class Square
{
  public:
    bool operator==(Square const & that) const;
    bool operator!=(Square const & that) const;
    int Rank() const { return m_rep.first; }
    int File() const { return m_rep.second; }
    std::string Algebraic() const;
  private:
    Square(int rank, int file);
    std::pair<int,int> m_rep;
    Square(Square const &);                  // unimplemented
    Square & operator=(Square const & that); // unimplemented
    friend class Board;
};

std::ostream& operator<<(std::ostream &, Square const &);

#endif
