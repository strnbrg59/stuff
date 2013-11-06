#include "el.hpp"

Clock::Clock() {}

void
Clock::tick()
{
    ++Clock::time_;
}

unsigned int
Clock::time()
{
    return Clock::time_;
}

unsigned int Clock::time_;
