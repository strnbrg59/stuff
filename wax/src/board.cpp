#include "board.hpp"
#include "position.hpp"
#include "trace.hpp"
#include "timer.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstring>
using namespace std;

extern TimerContext g_legalMovesTimer;
extern TimerContext g_evaluateTimer;

void
Board::Init(int nRanks)
{
    m_nRanks = nRanks;
    typedef boost::shared_ptr<Square const> PCS;
    typedef boost::shared_array<PCS> PPCS;
    m_squares.reset(new PPCS[m_nRanks]);
    for(int i=0;i<m_nRanks;++i)
    {
        m_squares[i].reset(new PCS[m_nRanks]);
        for(int j=0;j<m_nRanks;++j)
        {
            m_squares[i][j].reset(new Square(i,j));
        }
    }
}


Square const &
Board::TheSquare(int rank, int file)
{
    return *m_squares[rank][file];
}


/** Allocates memory.
 *  Loads initial position from a file.
 *  Should only be called once; all other Position objects are created from this
 *  one via the copy ctor (and then modified by operator+=).
 */
Position::Position(char const * initialPositionFile, int nRanks)
  : m_halfMoves(0),
    m_nRanks(nRanks)
{
    Trace t("Position::Position(char const*, int)");
    m_bonusMobilityPoints[0] = m_bonusMobilityPoints[1] = 0.0;

    if(nRanks > 9)
    {
        t.FatalError("n_ranks (board size) above 9 not supported");
    }

    Board::Init(nRanks);

    typedef boost::shared_ptr<Piece const> PCT;
    typedef PCT* PPCT;
    m_pieces = new PPCT[m_nRanks];
    for(int i=0;i<m_nRanks;++i)
    {
        m_pieces[i] = new PCT[m_nRanks];
        for(int j=0;j<m_nRanks;++j)
        {
            m_pieces[i][j].reset((Piece const*)0);
        }
    }

    // Format of infile: white pieces are capitalized:
    // R c7
    // K e5
    // k g3
    FILE * infile = fopen(initialPositionFile, "r");
    assert(infile);

    unsigned int const maxLineLength(80);
    char line[maxLineLength+1];

    char letters[] = "abcdefghi";
    char numbers[] = "123456789";

    while(1)
    {
        char * pc;
        pc = fgets(line, maxLineLength, infile);
        if(!pc) break;
        assert(strlen(line) < maxLineLength);
        assert(line[strlen(line)-1] == '\n');
        if(feof(infile))
        {
            break;
        }
        if((line[0] == '#') || (line[0]=='\n'))
        {
            continue;
        }

        char * lastTok;
        char * tok;

        tok = strtok_r(line, " ", &lastTok);
        assert(tok);
        char pieceTok = tok[0];

        tok = strtok_r(0, "\n", &lastTok);
        assert(tok);
        assert(strlen(tok) == 2);
        char fileTok = tolower(tok[0]);
        char rankTok = tok[1];
        int nfile = strchr(letters, fileTok) - letters;
        assert((nfile>=0) && (nfile<m_nRanks));
        int rank = strchr(numbers, rankTok) - numbers;
        assert((rank>=0) && (rank<m_nRanks));

        Piece::Color color = (pieceTok==tolower(pieceTok)) 
                           ? Piece::black : Piece::white;
        switch(tolower(pieceTok))
        {
            case 'p' : m_pieces[rank][nfile].reset(new Pawn(color)); break;
            case 'r' : m_pieces[rank][nfile].reset(new Rook(color)); break;
            case 'n' : m_pieces[rank][nfile].reset(new Knight(color)); break;
            case 'b' : m_pieces[rank][nfile].reset(new Bishop(color)); break;
            case 'q' : m_pieces[rank][nfile].reset(new Queen(color)); break;
            case 'k' : m_pieces[rank][nfile].reset(new King(color)); break;
        }
    }
    fclose(infile);

    m_legalMoves[Piece::white].reset(new MovesVect());
    m_legalMoves[Piece::black].reset(new MovesVect());
    this->FindLegalMoves();

    m_value = this->Evaluate(true);
}


Position::~Position()
{
    Trace t("Position::~Position()");
    for(int i=0;i<m_nRanks;++i)
    {
        delete[] m_pieces[i];
    }
    delete[] m_pieces;
}


Position::Position(Position const & that)
  : m_halfMoves(that.m_halfMoves),
    m_nRanks(that.m_nRanks)
{
    Trace t("Position::Position(Position&)"); t.noop();
    typedef boost::shared_ptr<Piece const> PCT;
    typedef PCT* PPCT;
    m_pieces = new PPCT[m_nRanks];
    for(int i=0;i<m_nRanks;++i)
    {
        m_pieces[i] = new PCT[m_nRanks];
        for(int j=0;j<m_nRanks;++j)
        {
            m_pieces[i][j] = that.m_pieces[i][j];
        }
    }

    m_value = that.m_value;
    for (int c=Piece::white; c<=Piece::black; ++c)
    {
        m_legalMoves[c] = that.m_legalMoves[c];
        m_bonusMobilityPoints[c] = that.m_bonusMobilityPoints[c];
    }
}

/** For internal use by this class, as it leaves Position in an inconsistent
 *  state; the m_value member is not updated!
 *  Breaking this out of operation+= should be needed only until we implement a
 *  way to recalculate the mobility index incrementally (rather than requiring
 *  a Position update and a full recalculation of the mobility index).
 */
void
Position::Advance(Move const& move)
{
    if(move.IsOO() || move.IsOOO())
    {
        assert(m_nRanks > 7);
        int blackMoves(m_halfMoves%2);

        Square const & kingFrom(Board::TheSquare(blackMoves*7, 4));
        Square const & kingTo(Board::TheSquare(blackMoves*7,
                              (move.IsOO() ? 6:2)));
        Move kingMove(kingFrom, kingTo);

        Square const & rookFrom(Board::TheSquare(blackMoves*7,
                                (move.IsOO() ? 7:0)));
        Square const & rookTo(Board::TheSquare(blackMoves*7,
                              (move.IsOO() ? 5:3)));
        Move rookMove(rookFrom, rookTo);

        this->Advance(kingMove);
        --m_halfMoves;
        this->Advance(rookMove);
    } else
    {
        int fromRank = move.From().Rank();
        int toRank = move.To().Rank();
        int fromFile = move.From().File();
        int toFile = move.To().File();

        assert(m_pieces[fromRank][fromFile]); // Shoulda checked already.

        m_pieces[toRank][toFile] = m_pieces[fromRank][fromFile];
        m_pieces[fromRank][fromFile].reset((Piece const*)0);

        // Pawn promotion:
        if((m_pieces[toRank][toFile]->Type() == Piece::pawn)
        && ((toRank == m_nRanks-1) || (toRank == 0)))
        {
            Piece::Color color = m_pieces[toRank][toFile]->GetColor();
            m_pieces[toRank][toFile].reset(new Queen(color));
        }
    }

    ++m_halfMoves;
}

/** Move the piece currently at arg "from", over to arg "to".
 *  If a piece is captured in the process, eliminate it.
 *  Update m_value.
*/
void
Position::operator+=(Move const & move)
{
    m_value += this->IncrementalPieceValues(move);
    m_value -= this->MobilityIndex();
    this->Advance(move);
    this->FindLegalMoves();
    m_value += this->MobilityIndex();
}


/** Populate m_legalMoves */
void
Position::FindLegalMoves()
{
    Trace t("Position::FindLegalMoves()"); t.noop();
    g_legalMovesTimer.Resume();

    bool stillHaveKing(false);

    for (int color=Piece::white; color<=Piece::black; ++color)
    {
        m_legalMoves[color]->clear();
        m_legalMoves[color]->reserve(64);
    }

    /* board-size integer array to keep track of squares the legal moves
     * might occupy, to help us calculate a more sophisticated "mobility index"
     * that takes account of the bonus value of having more than one piece
     * control a given square, and (later) that some squares (the central ones,
     * the ones near the king) are more worth controlling.
     */
    typedef boost::shared_array<int> intarray;
    typedef boost::shared_array<intarray> tempBoardT;
    tempBoardT tempBoard(new intarray[m_nRanks]);
    for (int i=0;i<m_nRanks;++i)
    {
        tempBoard[i].reset(new int[m_nRanks]);
        for (int j=0;j<m_nRanks;++j)
        {
            tempBoard[i][j] = 0;
        }
    }
    int const multiOccupancyBonus = 3.0; // 1.0 is more reasonable, but we'll
        // set it to 3.0 for starters just to make the effect obvious.
    m_bonusMobilityPoints[0] = m_bonusMobilityPoints[1] = 0.0;

    boost::shared_ptr<Piece const> piece;
    for(int i=0;i<m_nRanks;++i)
    {
        for(int j=0;j<m_nRanks;++j)
        {
            piece = m_pieces[i][j];
            if (!piece)
            {
                continue;
            }

            if(piece->Type() == Piece::king)
            {
                stillHaveKing = true;
            }

            MovesVect candidates = piece->FindLegalMoves(i,j,*this);
            // Pieces check all aspects of move legality.

            for(MovesVect::iterator iter = candidates.begin();
                 iter != candidates.end();
                 ++iter)
            {
                if(PieceAt(iter->To()))
                {
                    iter->IsACapture(true);
                }

                int r = iter->To().Rank();
                int f = iter->To().File();
                tempBoard[r][f] ++;
                if (tempBoard[r][f] > 1)
                {
                    assert( piece->GetColor() == 0
                        ||  piece->GetColor() == 1);
                    m_bonusMobilityPoints[piece->GetColor()] +=
                        multiOccupancyBonus;
                    if (abs(m_bonusMobilityPoints[piece->GetColor()]) > 1000)
                    {
                        assert(0);
                    }
                }
            }

            m_legalMoves[piece->GetColor()]->insert(
                m_legalMoves[piece->GetColor()]->end(),
                candidates.begin(), candidates.end());
        }
    }

    if(!stillHaveKing)
    {
        for (int color=Piece::white; color<=Piece::black; ++color)
        {
            m_legalMoves[color]->clear();
        }
    }
    g_legalMovesTimer.Pause();
}


bool
Position::IsLegalMove(Move const & move) const
{
    Trace t("Position::IsLegalMove()");

    if(move.IsOO() || move.IsOOO())
    {
        // Only human players castle; we'll assume they've checked things
        // out.
        return true;
    }

    if(move.From() == move.To())
    {
        t.Error("Cannot move a piece zero distance.");
        return false;
    }

    Square const & fromSquare(move.From());
    int fromRank(fromSquare.Rank());
    int fromFile(fromSquare.File());
    if(!m_pieces[fromRank][fromFile])
    {
        t.Error("There is no piece at [%d,%d]", fromRank, fromFile);
        return false;
    }
    return m_pieces[fromRank][fromFile]->IsLegalMove(move, *this);
}


boost::shared_ptr<Piece const> const
Position::PieceAt(Square const & square) const
{
    return m_pieces[square.Rank()][square.File()];
}


bool
Position::NoKing() const
{
    // This should do it.  It's wasteful to actually iterate over the whole
    // board to check if there's a king there.
    return fabs(m_value) > King::value/2;
}

void
Position::Display() const
{
    Trace t("Position::Display()"); t.noop();
    for(int i=m_nRanks-1;i>=0;--i)
    {
        for(int j=0;j<m_nRanks;++j)
        {
            cout << "+----";
        }
        cout << "+" << '\n';

        for(int j=0;j<m_nRanks;++j)
        {
            if(m_pieces[i][j])
            {
                cout << "|  " << m_pieces[i][j]->GetDisplaySymbol() << " ";
            } else
            {
                cout << "|    ";
            }
        }
        cout << "|" << '\n';
    }
    for(int j=0;j<m_nRanks;++j)
    {
        cout << "+----";
    }
    cout << "+" << '\n';
}


double
Position::PieceValues() const
{
    double result = 0.0;

    for(int i=0;i<m_nRanks;++i)
    {
/*
        // Pawn value.  It's bad to move off home spot.  But after that, it gets
        // only better.  This isn't so great; it leads to lots of x2-x4 moves.
        // Should go with something nonlinear, and maybe also take account of
        // whether the pawn is a passed pawn.
        double pawnValueMultiple[2] = {1 + (i-1)/6.0, 1 + (6-i)/6.0};
        if (i == 2) {
            pawnValueMultiple[Piece::white] = 0.7;
        } else if (i == 5)
        {
            pawnValueMultiple[Piece::black] = 0.7;
        }
*/
    
        for(int j=0;j<m_nRanks;++j)
        {
            if(m_pieces[i][j])
            {
                boost::shared_ptr<Piece const> const piece(m_pieces[i][j]);

                double v = piece->GetValue();
/*
                // Simplify the pawn thing until you debug incremental
                // evaluation.
                if (piece->Type() == Piece::pawn) {
                    v *= pawnValueMultiple[piece->GetColor()];
                }                    
*/
                if(piece->GetColor() == Piece::white)
                {
                    result += v;
                } else
                {
                    result -= v;
                }
            }
        }
    }

    return result;
}


double
Position::MobilityIndex() const
{
    MovesVectPtr legalMovesWhite(GetLegalMoves(Piece::white));
    MovesVectPtr legalMovesBlack(GetLegalMoves(Piece::black));

    int netLegalMoves = int(legalMovesWhite->size())
                      - int(legalMovesBlack->size());
    double netBonusMobility = m_bonusMobilityPoints[Piece::white]
                            - m_bonusMobilityPoints[Piece::black];

    double result = (netLegalMoves + netBonusMobility)/10.0;

/*
    cout << "legalMoves = " << GetLegalMoves(Piece::white)
         << " " << GetLegalMoves(Piece::black) << '\n';
    cout << "bonusMobility = " << m_bonusMobilityPoints[Piece::white]
         << " " << m_bonusMobilityPoints[Piece::black];
*/

    return result;
}


/** Finds value this Position would have *after* move is made.
 *
 *  Return value reflects white's advantage.
 */
double
Position::IncrementalPieceValues(Move const& move) const
{
    int fromRank = move.From().Rank();
    int fromFile = move.From().File();
    int toRank = move.To().Rank();
    int toFile = move.To().File();

    int sign =
        (m_pieces[fromRank][fromFile]->GetColor() == Piece::white) ? 1:-1;

    if (move.IsACapture() && !(this->NoKing()))
    {
        boost::shared_ptr<Piece const> capturedPiece = m_pieces[toRank][toFile];
        double result = capturedPiece->GetValue() * sign;
        return result;
    } else
    if((m_pieces[fromRank][fromFile]->Type() == Piece::pawn)
        && ((toRank == m_nRanks-1) || (toRank == 0)))
    {
        Pawn p(Piece::white);
        Queen q(Piece::white);
        return (q.GetValue() - p.GetValue()) * sign;
    } else
    {
        return 0.0;
    }
}

/** Gives value of this Position.
 *  m_value is updated in operator+=.
 */
double
Position::Evaluate(bool initial /*=false*/) const
{
    Trace t("Position::Evaluate()"); t.noop();
    g_evaluateTimer.Resume();

    double result = 0.0;
    if (initial)
    {
        result = PieceValues() + MobilityIndex();
    } else
    {
        result = m_value;
    }

    g_evaluateTimer.Pause();
    return result;
}


Square const Board::nullSquare(-1,-1);
int Board::m_nRanks;
boost::shared_array<boost::shared_array<boost::shared_ptr<Square const>
 > > Board::m_squares(0);
