#include "el.hpp"
#include "trace.hpp"
#include "cmdline.hpp"
#include <cmath>
#include <iostream>

void
Rider::Print(std::ostream& out)
{
    out << "(" << from_ << ", " << to_ << ")";
}

bool
Riders::empty()
{
    return new_riders_.empty();
}

RiderPtr
Riders::pop()
{
    assert(!empty());
    RiderPtr result = new_riders_.front();
    new_riders_.pop_front();
    return result;
}

void
Riders::generate()
{
    Trace t("Riders::generate()");
    int floors = CmdlineFactory::TheCmdline().NFloors();
    double lambda = CmdlineFactory::TheCmdline().RiderArrivalRate();

    // From http://en.wikipedia.org/wiki/Poisson_distribution#Generating_Poisson-distributed_random_variables
    double exp_lambda = exp(-lambda);
    double p = random()/double(RAND_MAX);
    while (p >= exp_lambda) {
        p *= random()/double(RAND_MAX);
        new_riders_.push_back(RiderPtr(new Rider(random()%floors,
                                                 random()%floors)));
    }
    t.Info("new_riders_.size()=%d", new_riders_.size());
}

unsigned Riders::n_processed_;
/*static*/ unsigned Riders::n_processed() { return n_processed_; }
unsigned Riders::n_arrived_;
/*static*/ unsigned Riders::n_arrived() { return n_arrived_; }
unsigned Riders::n_waiting_;
/*static*/ unsigned Riders::n_waiting() { return n_waiting_; }
unsigned Riders::time_in_system_;
/*static*/ unsigned Riders::time_in_system() { return time_in_system_; }

Rider::Rider(int from, int to)
 : from_(from), to_(to) {
    time_created_ = Clock::time();
    ++Riders::n_arrived_;
    ++Riders::n_waiting_;
}

Rider::~Rider()
{
    Riders::time_in_system_ += Clock::time() - time_created_;
    ++Riders::n_processed_;
}
