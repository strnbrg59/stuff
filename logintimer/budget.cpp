#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>
#include "analogclock.h"
#include "budget.h"
using namespace std;

extern MasterTimer g_masterTimer;

namespace budget {

string budgetFileName()
{
    ostringstream budget_file_name;
    budget_file_name << getenv("HOME") << "/" << ".login_budget.dat";
    return budget_file_name.str();
}

unsigned int getBudget()
{
    FILE* budget_file = fopen(budgetFileName().c_str(), "r");
    assert(budget_file);
    char budget[80];
    fgets(budget, 79, budget_file);
    fclose(budget_file);
    return atoi(budget);
}

int budgetFileTime()
{
    struct stat mystat;
    stat(budgetFileName().c_str(), &mystat);
    return mystat.st_mtime;
}

void updateBudget()
{
    ofstream budget_file(budgetFileName().c_str());
    budget_file << g_masterTimer.budget() << '\n';
}

void atexitFunc()
{
    cerr << "atexitFunc()" << '\n';
    updateBudget();
}

void sighandler(int i)
{
    updateBudget();
    exit(0);
}

/* Ensure that no matter how the program dies, we update the budget file.
 * That's why I'm registering sighandler() with every cottonpickin signal.
 * Except for SIGCHLD that is, because we'll get a SIGCHLD from that
 * system() call in killFirefox() or killX(), and we don't want that to
 * terminate this program as it needs to keep killing any newly-launched firefox
 * processes.
 */
void signalHandling()
{
    atexit(atexitFunc);
    for (int i=0; i<32; ++i) {
        if (i != SIGCHLD) {
            signal(i, sighandler);
        }
    }
}

void killX()
{
    cerr << "Time to die, pid " << getpid() << '\n';
    system("kill `ps ux | grep 'fvwm$' | grep -v grep | awk '{print $2}'`");
    exit(0);
}

void killInternet()
{
    // Should be in ~strnbrg59/usr/local/scripts:
    system("sudo iptables-joe.sh");
}

void killFirefox()
{
    system("/home/strnbrg59/usr/local/scripts/kill_firefox.sh");
}

}
