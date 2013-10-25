#include "move.hpp"
#include "position.hpp"
#include "board.hpp"
#include "trace.hpp"
#include <string>
#include <cstring>
#include <cassert>
using namespace std;

Move::Move()
  : m_rep(&Board::nullSquare, &Board::nullSquare),
    m_isACapture(false),
    m_OO(false),
    m_OOO(false)
{
}


Move::Move(Square const & from, Square const & to)
  : m_rep(&from, &to),
    m_isACapture(false),
    m_OO(false),
    m_OOO(false)
{
}


Move &
Move::operator=(Move const & that)
{
    m_rep = that.m_rep;
    m_isACapture = that.m_isACapture;
    m_OO = that.m_OO;
    m_OOO = that.m_OOO;
    return *this;
}


Move::Move(string algebraic)
  : m_rep(&Board::nullSquare, &Board::nullSquare),
    m_isACapture(false),
    m_OO(false),
    m_OOO(false)
{
    Trace t("Move::Move()"); t.noop();
    // For now, accept only fully-qualified algebraic notation, i.e. that which
    // always indicates the moving piece's location, e.g. Bc1-b2.
    // For castling, use "OO" and "OOO".

    // Deal first with castling.
    if((algebraic=="OO") || (algebraic=="OOO"))
    {
        if(algebraic=="OO")
        {
            m_OO = true;
        } else
        {
            m_OOO = true;
        }
        return;
    }

    char letters[] = "abcdefghi";
    char * buf = new char[algebraic.size()+1];
    strcpy(buf, algebraic.c_str());
    
    char fromAlg[3];
    if(isupper(buf[0]))
    {
        strncpy(fromAlg, buf+1, 2);
    } else
    {
        strncpy(fromAlg, buf, 2);
    }
    fromAlg[2]=0;

    char * pc = strchr(letters, fromAlg[0]);
    assert(pc);
    int file = pc - letters;
    int rank = atoi(fromAlg+1) - 1;
    Square const & fromSquare(Board::TheSquare(rank, file));

    char toAlg[3];
    if(isupper(buf[0]))
    {
        strncpy(toAlg, buf+4, 2);
    } else
    {
        strncpy(toAlg, buf+3, 2);
    }
    toAlg[2]=0;
    pc = strchr(letters, toAlg[0]);
    assert(pc);
    file = pc - letters;
    rank = atoi(toAlg+1) - 1;
    Square const & toSquare(Board::TheSquare(rank, file));

    delete [] buf;

    m_rep = pair<Square const *,Square const *>(&fromSquare, &toSquare);
}

string
Move::Algebraic(Position const & position) const
{
    Trace t("Move::Algebraic()"); t.noop();
    std::ostringstream ost;
    boost::shared_ptr<Piece const> const fromPiece(position.PieceAt(From()));
    boost::shared_ptr<Piece const> const toPiece(position.PieceAt(To()));

    Move nullMove;
    if(*this == nullMove) return "nullmove";
    
    if(fromPiece->Type() != Piece::pawn)
    {
        char str[2];
        str[0] = toupper(fromPiece->GetDisplaySymbol());
        str[1] = 0;        
        ost << str;
    }
    ost << From().Algebraic();
    if(toPiece) // capture
    {
        ost << "x";
    } else
    {
        ost << "-";
    }
    ost << To().Algebraic();

    return ost.str();
}


bool
Move::operator==(Move const & that) const
{
    return (*(m_rep.first) == *(that.m_rep.first))
        && (*(m_rep.second) == *(that.m_rep.second))
        && (m_OO == that.m_OO)
        &&  (m_OOO == that.m_OOO);
    // Don't compare m_isACapture fields -- for human-entered moves, it's always
    // false (even when it should be true), and that would be a problem in
    // Piece::IsLegalMove().
}
bool
Move::operator!=(Move const & that) const
{
    return !operator==(that);
}


std::ostream &
operator<<(std::ostream & o, Move const & move)
{
    o << "(" << move.From() << "-" << move.To() << ")";
    return o;
}
