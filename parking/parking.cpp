#include "parking.hpp"
#include "cmdline.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
using namespace std;

double Space::car_length = 10.0;
double Space::size() const { return end_ - start_; }

pair<Space,Space>
Space::divide(double& absolute_rear) const
{
    double relative_rear = (random()/double(RAND_MAX)) * (size()-car_length);
    absolute_rear = start_ + relative_rear;
    pair<Space,Space> result = 
        make_pair(Space(start_, start_ + relative_rear),
                  Space(start_ + relative_rear + car_length, end_));
/**/
    cout << "  absolute_rear = " << absolute_rear << '\n';
/**/
    return result;
}


Space&
Space::operator+=(Space const& that)
{
    assert(   equal_enough(this->end_, that.start_)
           || equal_enough(this->start_, that.end_));

    if (equal_enough(this->end_, that.start_))
    {
        this->end_ = that.end_;
    } else
    if (equal_enough(this->start_, that.end_))
    {
        this->start_ = that.start_;
    }
    return *this;
}


ostream& operator<<(std::ostream& out, Space const& space)
{
    out << "(" << space.start_ << ", " << space.end_ << ")";
    return out;
}


int main(int argc, char** argv)
{
    Cmdline cmdline(argc, argv);
    CmdlineFactory::Init(cmdline);
    srand(CmdlineFactory::TheCmdline().Srand());
    for (int i=0;i<CmdlineFactory::TheCmdline().NIterations();++i) {
        simulation();
    }
}


Curb::Curb(double length)
  : length_(length)
{
    spaces_.insert(Space(0, length_));
}


void Curb::arrive()
{
    if (spaces_.empty())
    {
        return;
    } 

   // Find a random space
    set<Space>::iterator iter;
    int r = random()%spaces_.size();
    iter = spaces_.begin();
    for (int i=0;i<r;++i) ++iter;

    // Park in it, dividing it into two smaller spaces.
    double rear;
    pair<Space,Space> children = iter->divide(rear);
    rears_.insert(rear);
    spaces_.erase(iter);
    if (children.first.size() > Space::car_length) {
        spaces_.insert(children.first);
    }
    if (children.second.size() > Space::car_length) {
        spaces_.insert(children.second);
    }

    debug_dump();
}


void Curb::debug_dump() const
{
    cout << "spaces_.size() = " << spaces_.size() << ", "
         << "n_cars_parked = " << rears_.size() << '\n';
    for (set<Space>::iterator i=spaces_.begin(); i!=spaces_.end(); ++i) {
        cout << "   " << *i << '\n';
    }
    for (set<double>::iterator i=rears_.begin(); i!=rears_.end(); ++i) {
        cout << "   " << *i << '\n';
    }
    cout << "\n\n";
}

void Curb::depart()
{
    if (rears_.empty())
    {
        return;
    }

    // Find a random car's rear.
    set<double>::iterator rear_iter;
    int r = random()%rears_.size();
    rear_iter = rears_.begin();
    for (int i=0;i<r;++i) ++rear_iter;

    //
    // Merge car's space with adjoining spaces.
    //
    Space new_space(*rear_iter, *rear_iter + Space::car_length);
    cout << "Car leaving with rear at " << *rear_iter << '\n';

    // Merge with small space (if any) to car in back. 
    if (rear_iter != rears_.begin())
    {
        set<double>::iterator prev_rear = rear_iter;
        --prev_rear;
        if (*prev_rear + 2*Space::car_length > *rear_iter)
        {
            Space smidgen(*prev_rear + Space::car_length, *rear_iter);
            new_space += smidgen;
        }
    }

    // Merge with small space (if any) to car in front.
    set<double>::iterator next_rear = rear_iter;
    ++next_rear;
    if (next_rear != rears_.end())
    {
        if (*rear_iter + 2*Space::car_length > *next_rear)
        {
            Space smidgen(*rear_iter + Space::car_length, *next_rear);
            new_space += smidgen;
        }
    }

    // Merge with Space behind (which is of course at least one car-length
    // in size).
    set<Space>::iterator space_iter = spaces_.begin();
    while (   (space_iter->start() < new_space.start())
           && (space_iter != spaces_.end()))
    {
        ++space_iter;
    }
    if (space_iter != spaces_.begin())
    {
        --space_iter;
    }
    if (equal_enough(space_iter->end(), new_space.start()))
    {
        new_space += *space_iter;
        spaces_.erase(space_iter);
    }

    // Merge with Space ahead.
    space_iter = spaces_.begin();
    while (   (space_iter->end() < new_space.start())
           && (space_iter != spaces_.end()))
    {
        ++space_iter;
    }
    if (   (space_iter != spaces_.end())
        && (equal_enough(space_iter->start(), new_space.end())))
    {
        new_space += *space_iter;
        spaces_.erase(space_iter);
    }

    // Extend new space to ends of curb, if there are no cars in the way.
    if (new_space.start() < Space::car_length)
    {
        Space smidgen(0, new_space.start());
        new_space += smidgen;
    }
    double curb_length = CmdlineFactory::TheCmdline().CurbLength();
    if (new_space.end() + Space::car_length > curb_length)
    {
        Space smidgen(new_space.end(), curb_length);
        new_space += smidgen;
    }

    // Remove car.
    rears_.erase(rear_iter);

    // Add new space.
    spaces_.insert(new_space);

    debug_dump();
}    


bool equal_enough(double x, double y)
{
    return fabs(x-y) < 0.000001;
}

void simulation()
{
    Curb curb(CmdlineFactory::TheCmdline().CurbLength());

    while (!curb.full()) {
        curb.arrive();
    }
    cout << "n_cars_parked=" << curb.cars_parked() << '\n';

  while (1)
  {
    for (unsigned i=0; i<curb.cars_parked()/2; ++i)
    {
        curb.depart();
    }
    cout << "n_cars_parked=" << curb.cars_parked() << '\n';
    while (!curb.full()) {
        curb.arrive();
    }
    cout << "n_cars_parked=" << curb.cars_parked() << '\n';
  }
}
