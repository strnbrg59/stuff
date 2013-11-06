#include "rhovot.hpp"
#include "cmdline.hpp"

CursesStuff::CursesStuff()
{

    mainwnd = initscr();
    noecho();
    cbreak();
    nodelay(mainwnd, TRUE);
    refresh();
    wrefresh(mainwnd);
    screen = newwin(10, CmdlineFactory::TheCmdline().DisplayWidth()+1,
                    1, 1);
    box(screen, ACS_VLINE, ACS_HLINE);
}


CursesStuff::~CursesStuff()
{
   endwin();
}

WINDOW* CursesStuff::mainwnd;
WINDOW* CursesStuff::screen;
WINDOW* CursesStuff::my_win;
