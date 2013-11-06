#include "rhovot.hpp"
#include "cmdline.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/foreach.hpp>

using namespace std;

int main(int argc, char** argv)
{
    Cmdline cmdline(argc, argv);
    CmdlineFactory::Init(cmdline);

    double track_len = CmdlineFactory::TheCmdline().TrackLen();

    /*
     * Initialization.
     */
    TrackPtr track0 = Track::init_tracks();
    vector<CarPtr> cars = Car::init_cars(track0);

    /*
     * Simulation loop.
     */
    CursesStuff* curses = new CursesStuff;
    int display_width = CmdlineFactory::TheCmdline().DisplayWidth();

    timespec my_timespec;
    my_timespec.tv_sec = 0;
    my_timespec.tv_nsec = CmdlineFactory::TheCmdline().Delay()*1000000;

    ofstream outfile("rhovot.dat");
    /* gnuplot command: plot "rhovot.dat" using 1:2 with linespoints t 'oto0', "rhovot.dat" using 1:3 with linespoints t 'oto1'
     */

    const int T = CmdlineFactory::TheCmdline().NIter();
    for (int t=0; t<T || T==0; ++t) {
        int ic = 0;
        curs_set(0);
        wclear(CursesStuff::screen);
        outfile << t << ' ';

        BOOST_FOREACH(CarPtr car, cars) {
            if (ic==0) outfile << car->speed() << ' ';
            if (ic==1) outfile << car->speed() << '\n';

            char buf[2]; buf[1] = 0x0;
            buf[0] = 'a' + ic++;

            int scaled_pos = int(car->pos()*display_width/track_len + 0.5);
            mvwprintw(CursesStuff::screen, car->track()->num(), scaled_pos,
                      buf);
            car->decide();
        }

        wrefresh(CursesStuff::screen);
        refresh();
        nanosleep(&my_timespec, 0);

        BOOST_FOREACH(CarPtr car, cars) {
            car->update();
        }
    }

    delete curses;
}
