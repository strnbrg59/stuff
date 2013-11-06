#include "el.hpp"
#include <curses.h>

CursesStuff::CursesStuff()
{
   mainwnd = initscr();
   noecho();
   cbreak();
   nodelay(mainwnd, TRUE);
   refresh();
   wrefresh(mainwnd);
   screen = newwin(50, 100, 1, 1);
   box(screen, ACS_VLINE, ACS_HLINE);
}


CursesStuff::~CursesStuff()
{
   endwin();
}

WINDOW* CursesStuff::mainwnd;
WINDOW* CursesStuff::screen;
WINDOW* CursesStuff::my_win;
