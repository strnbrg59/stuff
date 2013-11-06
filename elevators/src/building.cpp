#include "el.hpp"
#include "limits.h"
#include "cmdline.hpp"
#include <curses.h>
#include <iostream>
using std::cout;

void
Building::display()
{
    if (CmdlineFactory::TheCmdline().UseCurses()) {
        curses_display();
    } else {
        cout << *this << '\n';
        for (int i=0;i<n_shafts_;++i)
        {
            cout << elevator(i) << '\n';
        }
        cout << "\n\n";
    }
}

void
Building::curses_display()
{
    curs_set(0);
    wclear(CursesStuff::screen);

    char buf[2]; buf[1] = 0x0;
    for (int i=0;i<int(state_.size());++i) {
        int ii = int(state_.size()-1) - i;
        for(unsigned j=0; j<state_[i].size(); ++j) {
            if (state_[i][j] < 0) {
                mvwprintw(CursesStuff::screen,ii,j, " ");
            } else {
                assert(state_[i][j]<16);
                buf[0] = "0123456789ABCDEF"[state_[i][j]];
                mvwprintw(CursesStuff::screen,ii,j, buf);
            }
        }
    }
    wrefresh(CursesStuff::screen);
    refresh();
}

void
Building::Print(std::ostream& out)
{
    for (int i=int(state_.size()-1); i>=0; --i) {
        for(unsigned j=0; j<state_[i].size(); ++j) {
            out << state_[i][j] << " ";
        }
        if (i > 0) out << '\n';
    }
}


Building::Building()
  : n_floors_(CmdlineFactory::TheCmdline().NFloors()),
    n_shafts_(CmdlineFactory::TheCmdline().NShafts())
{
    for (int i=0; i<n_shafts_; ++i) {
        elevators_.push_back(Elevator());
    }
    for (int j=0; j<n_floors_; ++j) {
        floors_.push_back(Floor(j));
    }

    for (int i=0;i<n_floors_;++i) {
        vector<int> floor;
        for (int j=0;j<n_shafts_;++j) {
            floor.push_back(-1);
        }
        state_.push_back(floor);
    }
}


// Should be called exactly once per clock-tick.
void
Building::update()
{
    for (int i=0;i<n_shafts_;++i) {
        for (int j=0;j<n_floors_;++j) {
            state_[j][i] = -1;
        }
        elevator(i).update_location();
        int elevator_at = elevator(i).floor();
        elevator(i).update_riders(floors_[elevator_at]);
        state_[elevator_at][i] = elevator(i).n_riders();
    }
}    

void
Building::add_rider(RiderPtr rider)
{
    floors_[rider->from()].waiters_.push_back(rider);
}

Elevator&
Building::elevator(int e)
{
    return elevators_[e];
}

Elevator&
Building::closest_elevator(int floor, bool& valid_elevator)
{
    int elevator_capacity = CmdlineFactory::TheCmdline().ElevatorCapacity();
    int min_dist = INT_MAX;
    int dist;
    vector<Elevator>::iterator result = elevators_.end();
    int i = 0;
    for (vector<Elevator>::iterator e=elevators_.begin();
         e!=elevators_.end(); ++e) {
        e->add_destination(floor, &dist, true);
        if (!CmdlineFactory::TheCmdline().UseCurses()) {
            cout << "dist(e(" << i++ << ")=" << dist << '\n';
        }
        if ((dist < min_dist) && (int(e->n_riders()) < elevator_capacity)) {
           min_dist = dist;
           result = e;
        }
    }
    valid_elevator = (result != elevators_.end());
    return *result;
}

unsigned
Building::riders_in_transit() const
{
    unsigned result = 0;
    for (vector<Elevator>::const_iterator e=elevators_.begin();
         e!=elevators_.end(); ++e) {
        result += e->n_riders();
    }
    return result;
}
