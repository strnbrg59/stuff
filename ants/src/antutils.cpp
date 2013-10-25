#include "antutils.hpp"
#include <time.h>

void fsleep(double dt)
{
    timespec ts;
    ts.tv_sec = time_t(dt);
    ts.tv_nsec = long(1E9 * (dt - ts.tv_sec));
    nanosleep(&ts,0);
}
