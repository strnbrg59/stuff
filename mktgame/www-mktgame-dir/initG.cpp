// Initialize the G structure (global variables).
#include "aqc.hpp"

Names*		G::names=NULL;
int			G::N_players = load_players( &G::names );

int			G::N_sec=0;
Security**	G::securitydata = load_securities(); // sets N_sec, too.
