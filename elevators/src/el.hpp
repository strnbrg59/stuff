#ifndef EL_HPP_INCLUDED
#define EL_HPP_INCLUDED

#include "printable.hpp"
#include <curses.h>
#include <iosfwd>
#include <vector>
#include <list>
#include <deque>
#include <boost/shared_ptr.hpp>
using std::vector;
using std::list;
using std::deque;

struct Clock
{
    Clock();
    static void tick();
    static unsigned int time();
private:
    static unsigned int time_;
};


struct CursesStuff
{
    CursesStuff();
    ~CursesStuff();
    static WINDOW *mainwnd;
    static WINDOW *screen;
    static WINDOW *my_win;
};

class Floor;

struct Rider : public NonconstPrintable
{
    Rider(int from, int to);
    ~Rider();
    void Print(std::ostream& out);
    int from() const { return from_; }
    int to() const { return to_; }
private:
    int from_;
    int to_;
    unsigned time_created_;
};
typedef boost::shared_ptr<Rider> RiderPtr;

struct Riders
{
    static void generate();
    static RiderPtr pop();
    static bool empty();

    static unsigned n_arrived();
    static unsigned n_processed();
    static unsigned n_waiting();
    static unsigned time_in_system();
private:
    static deque<RiderPtr> new_riders_;

    static unsigned n_arrived_;
    static unsigned n_processed_;
    static unsigned n_waiting_;
    static unsigned time_in_system_;

    friend class Rider;
    friend class Floor;
};

struct Elevator : public NonconstPrintable
{
    Elevator();
    ~Elevator() {}
    void Print(std::ostream& out);
    void update_location();
    void update_riders(Floor&);
    int floor() const;
    int direction() const;
    unsigned n_riders() const;
    void add_destination(int floor, int* distance, bool just_asking=false);
private:
    void move(int n_floors);
    void come_to(int floor);
    int curr_floor_;
    int stop_at_;
    int direction_;
    list<int> destinations_;
    list<RiderPtr> riders_;
};

struct Floor
{
    Floor(int n) : number_(n) {}
    RiderPtr hand_off();
    vector<RiderPtr> waiters_;
    int number_;
};

struct Building : public NonconstPrintable
{
    Building();
    ~Building() {}
    void Print(std::ostream& out);
    void display();
    Elevator& elevator(int);
    void add_rider(RiderPtr);
    Elevator& closest_elevator(int floor, bool& valid_elevator);
    void update();
    unsigned riders_in_transit() const;
private:
    vector< vector<int> > state_;
    vector<Elevator> elevators_;
    vector<Floor> floors_;
    int n_floors_;
    int n_shafts_;

    void curses_display();
};

#endif // include guard
