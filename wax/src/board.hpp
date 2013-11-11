#ifndef _INCLUDED_BOARD_H_
#define _INCLUDED_BOARD_H_

#include "square.hpp"
#include "piece.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

struct WithPawns
{
    enum {False=false, True=true};
};


/**
 * A singleton, and a speed optimization.
 * We used to construct Square objects in a million places, and that became too
 * expensive speed-wise.  So now instead of constructing Squares, we use a const
 * reference to a Square that's already in the Board object.
*/
class Board
{
  public:
    static void Init(int nRanks);
    static Square const & TheSquare(int rank, int file);
    static Square const nullSquare;
  private:
    static boost::shared_array<
        boost::shared_array<
            boost::shared_ptr<Square const> > > m_squares;
    static int m_nRanks;
};

#endif // include guard
