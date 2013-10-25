#ifndef __INCLUDED_ANT_HPP__
#define __INCLUDED_ANT_HPP__

#include "field.hpp"
#include <deque>

class Ant
{
public:
    void Move();
    bool HasFood() const { return m_haveFood; }
    void Age();
private:
    Coords m_position;
    Coords m_home;
    bool   m_haveFood;
    unsigned int m_age;
    Field* m_field;
    // Pointers, rather than const references, so we can copy instances.

    Ant(Coords const&);
    void RandomMove();
    void PureRandomMove();
    void FoodFirstMove(bool* foundIt);
    void PheromoneFollowingMove();
    void GoHomeMove();

    friend class AntFactory;
};


class AntFactory
{
public:
    static void Init();
    static Ant& GetAnt(int);
    static void ReapDeadAnts();
    static void SpawnNewAnts();
    static void LayEggs();
    static int NAnts() { return m_ants.size(); }
    static int NBabies() { return m_babies.size(); }
private:
    static deque<Ant> m_ants;
    static deque<unsigned int> m_babies;
};

#endif
