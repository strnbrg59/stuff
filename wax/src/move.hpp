#ifndef _INCLUDED_MOVE_H_
#define _INCLUDED_MOVE_H_

class Position;
#include "square.hpp"
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

class Move
{
  public:
    Move();
    Move(Square const & from, Square const & to);
    Move(std::string algebraic);
    Move & operator=(Move const & that);
    bool operator==(Move const & that) const;
    bool operator!=(Move const & that) const;
    Square const & From() const { return *m_rep.first; }
    Square const & To() const { return *m_rep.second; }
    std::string Algebraic(Position const &) const;
    bool  IsACapture() const { return m_isACapture; }
    void  IsACapture(bool value) { m_isACapture=value; }
    bool  IsOO() const { return m_OO; }
    bool  IsOOO() const { return m_OOO; }
  private:
    std::pair<Square const *, Square const *> m_rep;
    bool m_isACapture;
    bool m_OO;  // castle
    bool m_OOO;
};
std::ostream & operator<<(std::ostream &, Move const &);

typedef std::vector<Move> MovesVect;
typedef boost::shared_ptr<MovesVect> MovesVectPtr;

#endif
