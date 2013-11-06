#include "utils.hpp"
#include "el.hpp"
#include "cmdline.hpp"
#include <iostream>
#include <iomanip>
#include <time.h>
using namespace std;

int main(int argc, char** argv)
{
    Cmdline cmdline(argc, argv);
    CmdlineFactory::Init(cmdline);
    bool use_curses = CmdlineFactory::TheCmdline().UseCurses();

    Building building;
    Riders riders;
    CursesStuff* curses=0;

    ofstream stats_file("elevators.out");
    
    if (use_curses) curses = new CursesStuff;

    timespec my_timespec;
    my_timespec.tv_sec = 0;
    my_timespec.tv_nsec = CmdlineFactory::TheCmdline().Delay()*1000000;

    for (int t=0; t!=CmdlineFactory::TheCmdline().NIterations();++t) {
        if (!use_curses) cout << "----------------------------\n";
        building.update();
        building.display();
        bool valid_elevator;

        riders.generate();
        while (!riders.empty()) {
            RiderPtr rider(riders.pop());
            building.add_rider(rider);
            Elevator& closest(building.closest_elevator(rider->from(),
                                                        valid_elevator));
            if (valid_elevator) {
                closest.add_destination(rider->from(), 0);
                if (!use_curses) {
                    cout << "rider=" << *rider << ", closest E="
                         << closest << '\n';
                }
            } else {
                if (!use_curses) {
                    cout << "All elevators full!\n";
                }
            }
        }

        nanosleep(&my_timespec, 0);
        Clock::tick();
        stats_file << "throughput/second = "
                       << double(Riders::n_processed())/Clock::time()
                   << " waittime/rider = "
                       << double(Riders::time_in_system())/Riders::n_processed()
                   << " waiting = "
                       << Riders::n_waiting()
                   << " riders_in_transit = "
                       << building.riders_in_transit()
                   << '\n' << flush;
    }

    delete curses;
}
