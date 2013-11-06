#ifndef __INCLUDED_FIELD_HPP__
#define __INCLUDED_FIELD_HPP__

#include <vector>
#include <iostream>
#include <memory>
using namespace std;
class Ant;
typedef unsigned short coord_t;
typedef unsigned char pheromone_t;

struct Coords
{
    Coords() : x(0), y(0) { }
    Coords(coord_t _x, coord_t _y) : x(_x), y(_y) {}
    bool operator==(Coords const&) const;
    bool operator!=(Coords const&) const;
    coord_t operator-(Coords const&) const; // Manhattan norm
    coord_t x;
    coord_t y;

    void Print(ostream&) const;
};
ostream& operator<<(ostream&, Coords const&);


class Square
{
public:
    bool SamePlace(Square const&) const;
    void Print(ostream&) const;
    void ReplenishPheromone();
    void DecayPheromone();
    Coords coord;
    unsigned int  nAnts;
    unsigned int  nLoadedAnts;
    pheromone_t amtPheromone;
    void IncrFood(int n=1) { m_amtFood+=n; }
    void DecrFood(int n=1);
    unsigned int GetAmtFood() const { return m_amtFood; }
private:
    Square();
    Square(Square const&);
    Square& operator=(Square const&);
    bool operator==(Square const&) const;
    unsigned int m_amtFood;
    enum {pheromone_max = 255};
    friend class Field;
};
ostream& operator<<(ostream&, Square const&);


class Field
{
public:
    void Reposition();
    Square* operator[](coord_t x) const;
    Square& operator[](Coords const& c) const;
    void Print(ostream&) const;
    static coord_t rank;
    void AntLeft(Coords const&, Ant const&);
    void AntArrived(Coords const&, Ant const&);
    void AntDied(Coords const&, Ant const&);
    Coords const& GetHome() const;
    void AddFood();
    void PrintStats() const;
    ~Field();
private:
    Field(int rank);
    Square** m_squares;
    Coords m_home;
    friend class FieldFactory;
};
ostream& operator<<(ostream&, Field const&);


class FieldFactory
{
public:
    static void Init();
    static Field& TheField();
private:
    static auto_ptr<Field> m_field;
};

#endif
