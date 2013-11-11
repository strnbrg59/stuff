#ifndef _INCLUDED_PIECE_H_
#define _INCLUDED_PIECE_H_

#include "move.hpp"
class Position;

class Piece
{
  public:
    enum Color { white, black };
    enum PieceType { pawn, rook, knight, bishop, queen, king };

    Piece(Color);
    virtual ~Piece();
    char GetDisplaySymbol() const { return m_displaySymbol; }
    Color GetColor() const { return m_color; }
    static Color OtherColor(Color);
    virtual MovesVect FindLegalMoves(int rank, int file, Position const &)
        const = 0;
    virtual bool IsLegalMove(Move const &, Position const &) const;
    virtual PieceType Type() const = 0;
    virtual double GetValue() const = 0;
  protected:
    Color m_color;
    char  m_displaySymbol;
};


class Pawn : public Piece
{
  public:
    Pawn(Color);
    virtual ~Pawn();
    virtual MovesVect FindLegalMoves(int rank, int file, Position const &)
        const;
    virtual PieceType Type() const { return pawn; }
    virtual double GetValue() const { return 1.0; }
};
class Rook : public Piece
{
  public:
    Rook(Color);
    virtual ~Rook();
    virtual MovesVect FindLegalMoves(int rank, int file, Position const &)
        const;
    virtual PieceType Type() const { return rook; }
    virtual double GetValue() const { return 5.0; }
};
class Knight : public Piece
{
  public:
    Knight(Color);
    virtual ~Knight();
    virtual MovesVect FindLegalMoves(int rank, int file, Position const &)
        const;
    virtual PieceType Type() const { return knight; }
    virtual double GetValue() const { return 3.0; }
};
class Bishop : public Piece
{
  public:
    Bishop(Color);
    virtual ~Bishop();
    virtual MovesVect FindLegalMoves(int rank, int file, Position const &)
        const;
    virtual PieceType Type() const { return bishop; }
    virtual double GetValue() const { return 3.0; }
};
class Queen : public Piece
{
  public:
    Queen(Color);
    virtual ~Queen();
    virtual MovesVect FindLegalMoves(int rank, int file, Position const &)
        const;
    virtual PieceType Type() const { return queen; }
    virtual double GetValue() const { return 9.0; }
};
class King : public Piece
{
  public:
    King(Color);
    virtual ~King();
    virtual MovesVect FindLegalMoves(int rank, int file, Position const &)
        const;
    virtual PieceType Type() const { return king; }
    enum {value = 1000};
    virtual double GetValue() const { return King::value; }
};

#endif // include guard
