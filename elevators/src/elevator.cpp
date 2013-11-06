#include "el.hpp"
#include "utils.hpp"
#include "cmdline.hpp"
#include "trace.hpp"
#include <iosfwd>
#include <curses.h>

Elevator::Elevator()
  : curr_floor_(0),
    stop_at_(0),
    direction_(0)
{}


void
Elevator::Print(std::ostream& out)
{
    out << "(fl=" << curr_floor_ << ", di=" << direction_ << ", sa="
        << stop_at_ << "; [";
    for (list<int>::iterator i=destinations_.begin(); i!=destinations_.end();
         ++i) {
        out << *i << " ";
    }
    out << "])";
}


int
Elevator::floor() const
{
    return curr_floor_;
}

int
Elevator::direction() const
{
    return direction_;
}

void
Elevator::move(int floors)
{
    Trace t("Elevator::move()");
    t.Info("floors=%d", floors);
    direction_ = floors == 0 ? 0 : abs(floors)/floors;
    stop_at_ = curr_floor_ + floors;
    assert(stop_at_ >= 0);
    assert(stop_at_ < CmdlineFactory::TheCmdline().NFloors());
}

void
Elevator::update_location()
{
    curr_floor_ += direction_;
    if (   ((direction_ > 0) && (curr_floor_ >= stop_at_))
        || ((direction_ < 0) && (curr_floor_ <= stop_at_))) {
        curr_floor_ = stop_at_;
        direction_ = 0;
    }

    while ((!destinations_.empty()) && (destinations_.front() == curr_floor_)) {
        destinations_.pop_front();
    }

    if (direction_ == 0) {
        if (!destinations_.empty()) {
            come_to(destinations_.front());
        }
    }
}

void
Elevator::update_riders(Floor& floor)
{
    unsigned capacity = CmdlineFactory::TheCmdline().ElevatorCapacity();
    while ((!floor.waiters_.empty()) && (riders_.size() < capacity)) {
        RiderPtr new_rider(floor.hand_off());
        riders_.push_back(new_rider);
        add_destination(new_rider->to(), 0);
    }
    list<RiderPtr>::iterator r=riders_.begin();
    while (r!=riders_.end()) {
        if ((*r)->to() == floor.number_) {
            list<RiderPtr>::iterator temp = r;
            ++r;
            riders_.erase(temp);
        } else {
            ++r;
        }
    }
}

void
Elevator::come_to(int floor)
{
    assert(floor < CmdlineFactory::TheCmdline().NFloors());
    assert(floor >= 0);
    move(floor - curr_floor_);
}


void
Elevator::add_destination(int floor, int* distance, bool just_asking/*=false*/)
{
    if (distance) *distance = 0;
    list<int>::iterator insertion_point;
    if (destinations_.empty()) {
        if (distance) *distance = abs(curr_floor_ - floor);
        insertion_point = destinations_.begin();
    } else
    if (floor == curr_floor_) {
        if (distance) *distance = 0;
        return;
    } else
    if (floor == destinations_.front()) {
        if (distance) *distance = abs(destinations_.front() - curr_floor_);
        return;
    } else
    if (utils::in_order(curr_floor_, floor, destinations_.front())) {
        insertion_point = destinations_.begin();
        if (distance) *distance = abs(floor - curr_floor_);
    } else {
        list<int>::iterator curr = destinations_.begin();
        list<int>::iterator next = curr; ++next;
        if (distance) *distance = abs(*curr - curr_floor_);
        while (1) {
            if (   (floor == *curr)
                || ((next!=destinations_.end()) && (floor == *next))) {
                if (distance) *distance += abs(floor - *curr);
                return;
            }
            if (next == destinations_.end()) {
                insertion_point = next;
                if (distance) *distance += abs(floor - *curr);
                break;
            }
            if (utils::in_order(*curr, floor, *next)) {
                insertion_point = next;
                if (distance) *distance += abs(floor - *curr);
                break;
            }
            if (distance) *distance += abs(*next - *curr);
            ++next;
            ++curr;
        }
    }

    if (!just_asking) {
        destinations_.insert(insertion_point, floor);
    }
}

unsigned
Elevator::n_riders() const
{
    return riders_.size();
}

deque<RiderPtr> Riders::new_riders_;
