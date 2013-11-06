#ifndef __INCLUDED_RHOVOT_HPP__
#define __INCLUDED_RHOVOT_HPP__

#include <curses.h>
#include <vector>
#include <boost/shared_ptr.hpp>

class Car;
typedef boost::shared_ptr<Car> CarPtr;

class Track;
typedef boost::shared_ptr<Track> TrackPtr;

typedef double TrackPos;

class Car
{
    double speed_;
    double accel_;

    TrackPtr track_;
    CarPtr ahead_;
    TrackPos pos_;

public:
    Car(TrackPtr track, CarPtr ahead, TrackPos pos) :
        speed_(0.0), accel_(0.0),
        track_(track),
        ahead_(ahead),
        pos_(pos) {}
    void decide();
    void update();
    TrackPtr track() const { return track_; }
    double pos() const { return pos_; }
    double speed() const { return speed_; }
    static std::vector<CarPtr> init_cars(TrackPtr track0);
};

class Track
{
    TrackPos begin_;
    TrackPos end_;
    TrackPtr left_;
    TrackPtr right_;
    int num_;

public:
    Track();
    TrackPos advance(double speed, TrackPos const& from);
    TrackPos const& begin() const { return begin_; }
    TrackPos const& end() const { return end_; }
    TrackPtr left() const { return left_; }
    TrackPtr right() const { return right_; }
    int num() const { return num_; }
    static TrackPtr init_tracks();
};

struct CursesStuff
{
    CursesStuff();
    ~CursesStuff();
    static WINDOW *mainwnd;
    static WINDOW *screen;
    static WINDOW *my_win;
};


#endif
