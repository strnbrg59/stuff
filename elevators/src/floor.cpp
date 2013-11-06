#include "el.hpp"

RiderPtr
Floor::hand_off()
{
    RiderPtr result = waiters_.back();
    waiters_.pop_back();
    --Riders::n_waiting_;
    return result;
}
