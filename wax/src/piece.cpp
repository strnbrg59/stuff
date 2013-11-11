#include "piece.hpp"
#include "board.hpp"
#include "position.hpp"
#include "move.hpp"
#include "trace.hpp"
#include <cassert>

Piece::Piece(Piece::Color c)
  : m_color(c)
{
}


Piece::~Piece()
{
}


Pawn::Pawn(Piece::Color c)
  : Piece(c)
{
    if(c == Piece::white)
    {
        m_displaySymbol = 'A';
    } else
    {
        m_displaySymbol = 'a';
    }
}

Pawn::~Pawn()
{
}


Rook::Rook(Piece::Color c)
  : Piece(c)
{
    if(c == Piece::white)
    {
        m_displaySymbol = 'R';
    } else
    {
        m_displaySymbol = 'r';
    }
}

Rook::~Rook()
{
}

Knight::Knight(Piece::Color c)
  : Piece(c)
{
    if(c == Piece::white)
    {
        m_displaySymbol = 'N';
    } else
    {
        m_displaySymbol = 'n';
    }
}

Knight::~Knight()
{
}

Bishop::Bishop(Piece::Color c)
  : Piece(c)
{
    if(c == Piece::white)
    {
        m_displaySymbol = 'B';
    } else
    {
        m_displaySymbol = 'b';
    }
}

Bishop::~Bishop()
{
}

Queen::Queen(Piece::Color c)
  : Piece(c)
{
    if(c == Piece::white)
    {
        m_displaySymbol = 'Q';
    } else
    {
        m_displaySymbol = 'q';
    }
}

Queen::~Queen()
{
}

King::King(Piece::Color c)
  : Piece(c)
{
    if(c == Piece::white)
    {
        m_displaySymbol = 'K';
    } else
    {
        m_displaySymbol = 'k';
    }
}

King::~King()
{
}


bool
Piece::IsLegalMove(Move const & move, Position const & position) const
{
    Trace t("Piece::IsLegalMove()");
    int fromRank(move.From().Rank());    
    int fromFile(move.From().File());    
    MovesVect mv(position.PieceAt(move.From())->FindLegalMoves(fromRank,
                                                                  fromFile,
                                                                  position));
    return std::find(mv.begin(), mv.end(), move) != mv.end();
}

/*static*/ Piece::Color
Piece::OtherColor(Color c)
{
    if (c == Piece::white)
    {
        return Piece::black;
    } else
    {
        return Piece::white;
    }
}

/** Return value format is [fromSquare,toSquare].
 *  Fill return value with all moves that this piece could make on an empty
 *  board.
 *
 *  En passant not implemented yet.
*/
MovesVect
Pawn::FindLegalMoves(int rank, int file, Position const & position) const
{
    Trace t("Pawn::FindLegalMoves()");

    MovesVect result;
    result.reserve(4);
    Square const & currentSquare(Board::TheSquare(rank,file));

    int forwardSign;
    if(m_color == white)
    {
        forwardSign = 1;
    } else
    {
        forwardSign = -1;
    }

    if(((m_color==white) && (rank < position.NRanks()-1))
    ||  ((m_color==black) && (rank > 0)))
    {
        // One square forward.
        if(!position.PieceAt(Board::TheSquare(rank+forwardSign,file)))
        {
            Square const & toSquareFwd(
                Board::TheSquare(rank + forwardSign, file));
            result.push_back(Move(currentSquare, toSquareFwd));
        }

        // Two capture directions.
        for(int rightwards = 1; rightwards>=-1; rightwards-=2)
        {
            if((((rightwards==+1) && (file < position.NRanks()-1))
                ||  ((rightwards==-1) && (file > 0)))
            && (position.PieceAt(Board::TheSquare(rank+forwardSign,
                                                  file+rightwards)))
            && (position.PieceAt(Board::TheSquare(rank+forwardSign,
                                                  file+rightwards))
                ->GetColor()!=m_color))
            {
                Square const & toSquareCapture(
                    Board::TheSquare(rank+forwardSign, file+rightwards));
                result.push_back(Move(currentSquare, toSquareCapture));
            }
        }
    }

    // Two-square move from initial position.
    if(((m_color==white) && (rank == 1)
        ||  (m_color==black) && (rank == position.NRanks()-2))
    &&  (position.NRanks() > 3)
    && (!position.PieceAt(Board::TheSquare(rank+forwardSign,file)))
    && (!position.PieceAt(Board::TheSquare(rank+forwardSign*2,file))))
    {
        Square const & toSquare(Board::TheSquare(rank+forwardSign*2, file));
        result.push_back(Move(currentSquare, toSquare));
    }

    for(unsigned m=0; m<result.size(); ++m)
    {
        assert(result[m].To().File() >= 0);
    }        

    return result;
}

//  This is a misleading name, because this macro handles all moves, not
//  just captures (but if there's a capture, it calls "break").
#   define BREAK_IF_CAPTURE                                            \
        boost::shared_ptr<Piece const> captured(position.PieceAt(toSquare)); \
        if(captured)                                                 \
        {                                                              \
            if(captured->GetColor() != m_color)                        \
            {                                                          \
                result.push_back(Move(currentSquare, toSquare));   \
            }                                                          \
            break;                                                     \
        } else                                                         \
        {                                                              \
            result.push_back(Move(currentSquare, toSquare));       \
        }

#   define CAPTURE                                                     \
        boost::shared_ptr<Piece const> captured(position.PieceAt(toSquare)); \
        if(captured)                                                 \
        {                                                              \
            if(captured->GetColor() != m_color)                        \
            {                                                          \
                result.push_back(Move(currentSquare, toSquare));   \
            }                                                          \
        } else                                                         \
        {                                                              \
            result.push_back(Move(currentSquare, toSquare));       \
        }


/** It would be more elegant for the pieces not to know anything about class
 *  Position.  But it's a tremendous optimization to let the pieces quickly
 *  stop producing "legal" moves once they reach an interposed piece.  Hence
 *  arg position.
 *  Position::FindLegalMoves() still needs to check that the color of any
 *  "captured" pieces is the opposite color.
*/
MovesVect
Rook::FindLegalMoves(int rank, int file, Position const & position) const
{
    Trace t("Rook::FindLegalMoves()");
    MovesVect result;
    result.reserve(14);
    Square const & currentSquare(Board::TheSquare(rank,file));

    for(int sr=-1; sr<2; ++sr)
    {
        for(int sf=-1; sf<2; ++sf)
        {
            if(! ((sr==0) ^ (sf==0))) continue;
            int i = rank + sr;
            int j = file + sf;
            while(position.IsOnBoard(i,j))
            {
                Square const & toSquare(Board::TheSquare(i,j));
                BREAK_IF_CAPTURE
                i += sr;
                j += sf;
            }
        }
    }

    return result;
}


MovesVect
Knight::FindLegalMoves(int rank, int file, Position const & position) const
{
    Trace t("Knight::FindLegalMoves()");
    MovesVect result;
    result.reserve(8);
    Square const & currentSquare(Board::TheSquare(rank,file));

    int offsets[8][2] = {{1,-2},
                         {2,-1},
                         {2,1},
                         {1,2},
                         {-1,2},
                         {-2,1},
                         {-2,-1},
                         {-1,-2}};
    for(int m=0;m<8;++m)
    {
        int newRank = rank+offsets[m][0];
        int newFile = file+offsets[m][1];
        if(position.IsOnBoard(newRank, newFile))
        {
            Square const & toSquare(Board::TheSquare(newRank,newFile));
            CAPTURE
        }
    }

    return result;
}


MovesVect
Bishop::FindLegalMoves(int rank, int file, Position const & position) const
{
    Trace t("Bishop::FindLegalMoves()");
    MovesVect result;
    result.reserve(14);
    Square const & currentSquare(Board::TheSquare(rank,file));

    for(int sr=1; sr>=-1; sr-=2) // signum rank
    {
        for(int sf=1; sf>=-1; sf-=2) // signum file
        {
            int i, j;
            for(i=rank+sr, j=file+sf;
                 position.IsOnBoard(i,j);
                 i+=sr, j+=sf)
            {
                Square const & toSquare(Board::TheSquare(i,j));
                BREAK_IF_CAPTURE
            }
        }
    }

    return result;
}


MovesVect
Queen::FindLegalMoves(int rank, int file, Position const & position) const
{
    Trace t("Queen::FindLegalMoves()");
    MovesVect result;
    result.reserve(28);
    Square const & currentSquare(Board::TheSquare(rank,file));

    for(int sr=-1; sr<2; ++sr)
    {
        for(int sf=-1; sf<2; ++sf)
        {
            if((sr==0) && (sf==0)) continue;
            int i = rank + sr;
            int j = file + sf;
            while(position.IsOnBoard(i,j))
            {
                Square const & toSquare(Board::TheSquare(i,j));
                BREAK_IF_CAPTURE
                i += sr;
                j += sf;
            }
        }
    }

    return result;
}


/** Castling not implemented yet. */
MovesVect
King::FindLegalMoves(int rank, int file, Position const & position) const
{
    Trace t("King::FindLegalMoves()");
    MovesVect result;
    result.reserve(8);
    Square const & currentSquare(Board::TheSquare(rank,file));

    for(int i=-1; i<2; ++i)
    {
        for(int j=-1; j<2; ++j)
        {
            if((i==0) && (j==0)) continue;
            if(position.IsOnBoard(rank+i, file+j))
            {
                Square const & toSquare(Board::TheSquare(rank+i,file+j));
                CAPTURE
            }
        }
    }

    return result;
}

#undef BREAK_IF_CAPTURE
#undef CAPTURE
