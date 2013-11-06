#include "rhovot.hpp"
#include "cmdline.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

void Car::decide() {
    /*
        x_0, v_0, a_0 : our car's position, speed, acceleration.
        x_1, v_1, a_1 : position, speed and acceleration of car in front of us.

        v_0,t+1 = v_0,t + a_0,t
        x_0,t+1 = x_0,t + (v_0,t + v_0,t+1)/2  // Our predicted position
        x_1,t+1 = x_1,t + v_1,t           // predicted position of car in front.
        x_1,t+1 - x_0,t+1 = phi * v_0,t+1 // phi=min safe following distance.
     */

    double phi = CmdlineFactory::TheCmdline().Phi();
    double min_a = CmdlineFactory::TheCmdline().MinA();
    double max_a = CmdlineFactory::TheCmdline().MaxA();
    double dp = ahead_->pos_ - pos_;
    if (dp < 0) dp += CmdlineFactory::TheCmdline().TrackLen();

    double desired_accel = (dp + ahead_->speed_)/(phi + 0.5)
                         - speed_*(1 + (1/(2*phi + 1)));
    double max_speed_accel = CmdlineFactory::TheCmdline().MaxSpeed()
                           - speed_;
    double zero_speed_accel = -speed_;

    accel_ = max(
                 min(
                     min(
                         max(desired_accel, min_a),
                         max_a),
                     max_speed_accel),
                 zero_speed_accel);
}

void Car::update() {
    double newspeed = speed_ + accel_;
    pos_ = track_->advance(0.5*(speed_ + newspeed), pos_);
    if (pos_ > track_->end()) {
        pos_ -= track_->end();
    }
    speed_ = newspeed;
}


/*static*/ vector<CarPtr> Car::init_cars(TrackPtr track0) {
    int n_cars = CmdlineFactory::TheCmdline().NCars();
    int n_tracks = CmdlineFactory::TheCmdline().NTracks();

    vector<CarPtr> result;

    TrackPos carpos = n_cars*5;
    CarPtr first_car(new Car(track0, CarPtr(), carpos));
    CarPtr curr_car = first_car;
    result.push_back(curr_car);
    for (int c=1; c<n_cars; ++c) {
        CarPtr new_car(new Car(track0, curr_car, carpos));
        carpos -= 5.0;
        curr_car = new_car;
        result.push_back(curr_car);
    }
    first_car->ahead_ = curr_car;

    return result;
}
