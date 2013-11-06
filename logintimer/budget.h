#ifndef _INCLUDED_BUDGET_H_
#define _INCLUDED_BUDGET_H_

#include <string>
#include <cstdlib>

namespace budget {

unsigned int getBudget();

int budgetFileTime();

void updateBudget();

void atexitFunc();

void signalHandling();

void killX();
void killInternet();
void killFirefox();

}

#endif
