#include "ant.hpp"
#include "cmdline.hpp"
#include "antutils.hpp"
#include <cassert>
#include <vector>
using namespace std;

Ant::Ant(Coords const& initialPosition)
  : m_haveFood(false),
    m_age(0),
    m_field(&FieldFactory::TheField())
    
{
    m_position = m_home = initialPosition;
    m_field->AntArrived(m_position, *this);
}


void
Ant::FoodFirstMove(bool* foundFood)
{
    *foundFood = false;
    short smellRadius = CmdlineFactory::TheCmdline().SmellRadius();
    for(short dx=-smellRadius; dx<smellRadius+1; ++dx)
    {
        for(short dy=-smellRadius; dy<smellRadius+1; ++dy)
        {
            Coords adjacentPos((Field::rank+m_position.x+dx)%Field::rank,
                               (Field::rank+m_position.y+dy)%Field::rank);
            if(adjacentPos == m_home)
            {
                continue;
            }
            if((*m_field)[adjacentPos].GetAmtFood() > 0)
            {
                // Take a step in direction of food.
                m_position.x = (Field::rank + m_position.x
                             +  (adjacentPos.x > m_position.x)
                             -  (adjacentPos.x < m_position.x))%Field::rank;
                m_position.y = (Field::rank + m_position.y
                             +  (adjacentPos.y > m_position.y)
                             -  (adjacentPos.y < m_position.y))%Field::rank;
                *foundFood = true;
                return;
            }
        }
    }
}


void
Ant::RandomMove()
{
    m_position.x = (Field::rank + m_position.x + random()%3 - 1)
                    %Field::rank;
    m_position.y = (Field::rank + m_position.y + random()%3 - 1)
                    %Field::rank;
    // Adding Field::rank in there in case the rest is negative; (-1)%n is
    // -1, and not, as one might expect, n-1.
}


void
Ant::GoHomeMove()
{
    m_position.x += (m_home.x > m_position.x) - (m_home.x < m_position.x);
    m_position.y += (m_home.y > m_position.y) - (m_home.y < m_position.y);
}


void
Ant::PheromoneFollowingMove()
{
    // My first strategy -- following the downward gradient of a pheromone trail
    // -- wasn't correct because if multiple ants are laying down a trail over
    // the same squares, the pheromone amount will rise and fall in a
    // complicated way.
    
    // Go to any adjacent square that has pheromone, preferring squares in
    // a direction away from home.
    short smellRadius = CmdlineFactory::TheCmdline().SmellRadius();
    Coords bestMove = m_position;
    for(short dx=-smellRadius; dx<smellRadius+1; ++dx)
    {
        for(short dy=-smellRadius; dy<smellRadius+1; ++dy)
        {
            Coords adjacentPos((Field::rank+m_position.x+dx)%Field::rank,
                               (Field::rank+m_position.y+dy)%Field::rank);
            if( ((dx==0) && (dy==0))
            || (abs(adjacentPos - m_home) <= 2*smellRadius))
            {
                continue;
            }
            if(((*m_field)[adjacentPos].amtPheromone > 0)
            && ((adjacentPos - m_home) > (bestMove - m_home)))
            {
                bestMove = adjacentPos;
            }
        }
    }
    if(bestMove != m_position)
    {
        // Take a step in direction of bestMove.
        m_position.x = (Field::rank + m_position.x
                     +  (bestMove.x > m_position.x)
                     -  (bestMove.x < m_position.x))%Field::rank;
        m_position.y = (Field::rank + m_position.y
                     +  (bestMove.y > m_position.y)
                     -  (bestMove.y < m_position.y))%Field::rank;
        return;
     }

    // If got here, found no pheromone.
    RandomMove();
}


void
Ant::Move()
{
    // Let Field do ant population updating.
    m_field->AntLeft(m_position, *this);

    //
    // Update m_position
    //
    if(m_haveFood) 
    {   // Go home
        GoHomeMove();
    } else
    {
        bool foundFood;
        FoodFirstMove(&foundFood);
        if(!foundFood)
        {
            PheromoneFollowingMove();
        }
    }

    //
    // Field updates first, then ant.  But ant adjusts Field::amtFood.
    //
    m_field->AntArrived(m_position, *this);

    if(m_haveFood)
    {
        if(m_position == m_home)
        {
           // Drops off food
            m_haveFood = false; 
            (*m_field)[m_home].IncrFood();
            AntFactory::LayEggs();
        } else
        {  // Leaves pheromone
            (*m_field)[m_position].ReplenishPheromone();
        }
    } else if(((*m_field)[m_position].GetAmtFood() > 0) && !m_haveFood
                && (m_position != m_home))
    {   // Picks up food
        (*m_field)[m_position].DecrFood();
        m_haveFood = true;
        (*m_field)[m_position].ReplenishPheromone();
    }
}


void
Ant::Age()
{
    ++m_age;
}


void
AntFactory::Init()
{
    Cmdline const& cmdline(CmdlineFactory::TheCmdline());
    Coords initialPosition;
    initialPosition = FieldFactory::TheField().GetHome();
    for(int a=0; a<cmdline.NAnts(); ++a)
    {
        Ant ant(initialPosition);
        m_ants.push_back(ant);
    }
}

void
AntFactory::ReapDeadAnts()
{
    if(m_ants.size() == 0)
    {
        return;
    }
    Cmdline const& cmdline(CmdlineFactory::TheCmdline());
    unsigned int dead=0;
    while((m_ants[dead].m_age > unsigned(cmdline.AntLife()))
    &&    (dead < m_ants.size()))
    {
        ++dead;
    }
    for(unsigned int i=0;i<dead;++i)
    {
        FieldFactory::TheField().AntDied(m_ants[0].m_position, m_ants[0]);
        m_ants.pop_front();        
    }
}


void
AntFactory::LayEggs()
{
    m_babies.push_back(0);
}


void
AntFactory::SpawnNewAnts()
{
    //
    // Should be called once each iteration.
    //

    // Increment all the babies' ages.
    for(unsigned i=0; i<m_babies.size(); ++i)
    {
        m_babies[i] ++;
    }

    Field& field(FieldFactory::TheField());
    Coords home(field.GetHome());
    Cmdline const& cmdline(CmdlineFactory::TheCmdline());

    unsigned int matured = 0;
    while((m_babies[matured] > unsigned(cmdline.GestationPeriod()))
    &&    (matured < m_babies.size()))
    {
        ++matured;
    }
    for(unsigned i=0; i<matured; ++i)
    {
        m_babies.pop_front();
        m_ants.push_back(Ant(home));
        field[home].DecrFood();
    }
}


Ant&
AntFactory::GetAnt(int a)
{
    return m_ants[a];
}

deque<Ant> AntFactory::m_ants;
deque<unsigned int> AntFactory::m_babies;
