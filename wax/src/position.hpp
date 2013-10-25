#ifndef _INCLUDED_POSITION_H_
#define _INCLUDED_POSITION_H_

#include "piece.hpp"
#include <boost/shared_ptr.hpp>

class Position
{
  public:
    Position(char const * infile, int nRanks); // Initial game position
    Position(Position const &);
    virtual ~Position();
    bool IsLegalMove(Move const &) const;
    void operator+=(Move const &);
    boost::shared_ptr<Piece const> const PieceAt(Square const &) const;
    MovesVectPtr const& GetLegalMoves(Piece::Color color) const
        { return m_legalMoves[color]; }
    bool IsOnBoard(int rank, int file) const;
    double Evaluate(bool initial=false) const;
    double IncrementalPieceValues(Move const&) const;
    bool NoKing() const;
    void Display() const;
    int NRanks() const { return m_nRanks; }

  private:
    Position & operator=(Position const &); // deliberately unimplemented
    double PieceValues() const;
    double MobilityIndex() const;
    void Advance(Move const&);
    void FindLegalMoves();

    boost::shared_ptr<Piece const> ** m_pieces;
    int m_halfMoves;
    int const m_nRanks;
    double m_value;
    MovesVectPtr m_legalMoves[2];
    double m_bonusMobilityPoints[2];
};


inline bool
Position::IsOnBoard(int rank, int file) const
{
    return (rank>=0) && (rank<m_nRanks) && (file>=0) && (file<m_nRanks);
}

#endif // include guard
