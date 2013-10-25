#include "field.hpp"
#include "ant.hpp"
#include "cmdline.hpp"
#include "antutils.hpp"
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

extern long long g_iter;

bool
Coords::operator==(Coords const& that) const
{
    return this->x == that.x  &&  this->y == that.y;
}

bool
Coords::operator!=(Coords const& that) const
{
    return this->x != that.x  ||  this->y != that.y;
}

coord_t
Coords::operator-(Coords const& that) const
{
    return abs(this->x - that.x) + abs(this->y - that.y);
}


Square::Square()
  : coord(0,0),
    nAnts(0),
    nLoadedAnts(0),
    amtPheromone(0),
    m_amtFood(0)
{
}


bool
Square::SamePlace(Square const& that) const
{
    return this->coord == that.coord;
}


void
Square::ReplenishPheromone()
{
    amtPheromone = pheromone_max;
}


void
Square::DecayPheromone()
{
    // Want to give display.DrawPheromone() a chance to catch it just before
    // it's completely decayed, so as to erase its pixel (rather than leave it
    // lingering, colored black, indefinitely).
    if(amtPheromone == 0)
    {
    } else if(amtPheromone == 1)
    {
        amtPheromone = 0;
    } else
    {
        float floatAmt =
            amtPheromone * CmdlineFactory::TheCmdline().PheromoneDecayRate();
        if(floatAmt > 1.0)
        {
            amtPheromone = pheromone_t(floatAmt);
        } else
        {
            amtPheromone = 1;
        }
    }
}


void
Coords::Print(ostream& out) const
{
    out << "(" << x << ", " << y << ")";
}

void
Square::Print(ostream& out) const
{
    out << coord;
}

ostream& operator<<(ostream& out, Coords const& coord)
{
    coord.Print(out);
    return out;
}

ostream& operator<<(ostream& out, Square const& square)
{
    square.Print(out);
    return out;
}

ostream& operator<<(ostream& out, Field const& field)
{
    field.Print(out);
    return out;
}


/*static*/ Field&
FieldFactory::TheField()
{
    return *m_field;
}


/*static*/ void
FieldFactory::Init()
{
    Cmdline const& cmdline(CmdlineFactory::TheCmdline());
    m_field.reset(new Field(cmdline.Rank()));
}


void
Field::AddFood()
{
    double feedingFreq = CmdlineFactory::TheCmdline().FeedingFreq();
    assert((feedingFreq>0.0) && (feedingFreq<1.0));
    int sqrtAmt = random()%10;
    Coords center(random()%Field::rank, random()%Field::rank);

    if(random()%(int(1/feedingFreq)) != 0)
    {
        return;
    }
    for(coord_t i=0; i<sqrtAmt; ++i) for(coord_t j=0; j<sqrtAmt; ++j)
    {
        if(center != m_home)
        {
            (*this)[(Field::rank + center.x+i-sqrtAmt/2)%Field::rank]
               [(Field::rank + center.y+j-sqrtAmt/2)%Field::rank].IncrFood();
        }
    }
}


Square&
Field::operator[](Coords const& c) const
{
    return m_squares[c.x][c.y];
}


Square*
Field::operator[](coord_t x) const
{
    return m_squares[x];
}


Coords const&
Field::GetHome() const
{
    return m_home;
}


Field::Field(int _rank)
  : m_home(_rank/2, _rank/2)
{
    rank = _rank;
    typedef Square* PSquare;
    m_squares = new PSquare[rank];
    for(coord_t x=0;x<rank;++x)
    {
        m_squares[x] = new Square[rank];
        for(coord_t y=0;y<rank;++y)
        {
            m_squares[x][y].coord = Coords(x,y);
        }
    }
}


Field::~Field()
{
    for(coord_t x=0;x<rank;++x) delete[] m_squares[x];
    delete[] m_squares;
}


void
Field::AntLeft(Coords const& pos, Ant const& ant)
{
    assert((*this)[pos].nAnts > 0);
    (*this)[pos].nAnts--;

    if(ant.HasFood() && ((*this)[pos].nLoadedAnts > 0))
    {   // Will be ==0 in epoch when ant has picked up food.
        (*this)[pos].nLoadedAnts--;
    }
}

void
Field::AntArrived(Coords const& pos, Ant const& ant)
{
    (*this)[pos].nAnts++;
    if(ant.HasFood() && (pos != m_home))
    {
        (*this)[pos].nLoadedAnts++;
    }
}


void
Field::AntDied(Coords const& pos, Ant const& ant)
{
    (*this)[pos].nAnts--;
    if(ant.HasFood())
    {
        (*this)[pos].nLoadedAnts--;
        (*this)[pos].IncrFood();
    }
}

void
Field::Reposition()
{
    AntFactory::ReapDeadAnts();
    AntFactory::SpawnNewAnts();
    AddFood();

    for(int a=0; a<AntFactory::NAnts(); ++a)
    {
        Ant& ant(AntFactory::GetAnt(a));
        ant.Age();
        ant.Move();
    }

    PrintStats();
}


void
Field::PrintStats() const
{
    if(g_iter % 100 == 0)
    {
        cerr << g_iter << " "
             << AntFactory::NAnts() << " "
             << AntFactory::NBabies() << '\n';
    }
}


void
Square::DecrFood(int n/*=1*/)
{
    assert(m_amtFood > 0);
    m_amtFood-=n;
}


void
Field::Print(ostream& out) const
{
    for(coord_t x=0;x<rank;++x)
    {
        for(coord_t y=0;y<rank;++y)
        {
            if(m_squares[x][y].nAnts != 0)
            {
                out << "a";
            } else
            {
                out << " ";
            }
        }
        out << '\n';
    }
}

coord_t Field::rank;
auto_ptr<Field> FieldFactory::m_field;
