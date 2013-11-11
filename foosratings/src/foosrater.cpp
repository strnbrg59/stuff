#include "foosrater.h"
#include "tokenizer.hpp"
#include "cmdline.hpp"

#include <cassert>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

using namespace std;

ostream&
GameScore::Print(ostream& out) const
{
    out << "[(" << _winnerO << ", " << _winnerD << "; " << _winningScore
        << "), " << _loserO << ", " << _loserD << "; " << _losingScore
        << ")]";
    return out;
}


void
GameScore::GetMatchup( string matchup[4] ) const
{
    matchup[0] = _winnerO;
    matchup[1] = _winnerD;
    matchup[2] = _loserO;
    matchup[3] = _loserD;
}

/* Change a player's name.  For glommerating rare players' names into just one.
 */
void
GameScore::Replace(std::string const& playerFrom, std::string const& playerTo)
{
    vector<string*> names = boost::assign::list_of<string*>(&_winnerO)(&_winnerD)
        (&_loserO)(&_loserD);
    BOOST_FOREACH(string* name, names)
    {
        if (*name == playerFrom)
        {
            *name = playerTo;
        }
    }
}

std::ostream& operator<<(std::ostream& out, GameScore const& gs)
{
    return gs.Print(out);
}

void LoadGameScores( vector<GameScore>& scores, istream* infile )
{
    // Format is:
    // winnerO winnerD winningScore loserO loserD losingScore
    if (!infile)
    {
        infile = &std::cin;
    }
    while( !infile->eof() )
    {
        char line[256];
        infile->getline(line, 256);
        if(line[0] == '#') continue;
        vector<string> tokens;
        (Tokenizer(line))(tokens);
        bool singles = (tokens.size() == 4);
        if( (tokens.size() != 6) && (tokens.size() != 4))
        {
            if(tokens.size()!=0)
            {
                cerr << "Warning: invalid line, tokens.size()=" << tokens.size()
                     << "\n"; // Last line -- '\n' -- can have this effect.
            }
            continue;
        }

        char* endptr;
        int winningScore, losingScore;
        if(singles)
        {
            winningScore = strtol(tokens[1].c_str(), &endptr, 10);
            assert(!*endptr);
            losingScore = strtol(tokens[3].c_str(), &endptr, 10);
            assert(!*endptr);
            scores.push_back( GameScore(tokens[0],
                              tokens[0], tokens[2], tokens[2],
                              losingScore, winningScore) );
        } else
        {
            winningScore = strtol(tokens[2].c_str(), &endptr, 10);
            assert(!*endptr);
            losingScore = strtol(tokens[5].c_str(), &endptr, 10);
            assert(!*endptr);
            scores.push_back( GameScore(tokens[0],
                              tokens[1], tokens[3], tokens[4],
                              losingScore, winningScore) );
        }
    }

    // Find players who have played few games and combine them into a single
    // category, so we don't waste degrees of freedom.
    int hasima = CmdlineFactory::TheCmdline().Hasima();
    map<string, int> playerCount;
    for( vector<GameScore>::iterator i = scores.begin();
         i != scores.end(); ++i )
    {
        vector<string> names = boost::assign::list_of<string>
            (i->winnerO())(i->winnerD())(i->loserO())(i->loserD());
        BOOST_FOREACH(string const& name, names)
        {
            playerCount[name] ++;
        }
    }
    for( vector<GameScore>::iterator i = scores.begin();
         i != scores.end(); ++i )
    {
        vector<string> names = boost::assign::list_of<string>
            (i->winnerO())(i->winnerD())(i->loserO())(i->loserD());
        BOOST_FOREACH(string const& name, names)
        {
            if (playerCount[name] < hasima)
            {
                i->Replace(name, "other");
            }
        }
    }
}
            

void AssemblePlayers(vector<string>& players, vector<GameScore> const& scores,
                     int tail)
{
    set<string> playerset;
    for( vector<GameScore>::const_iterator i = scores.begin();
         i != scores.end()-tail;
         ++i )
    {
        string matchup[4];
        i->GetMatchup( matchup );
        for( int j=0; j<4; ++j )
        {
            playerset.insert( matchup[j] );
        }
    }
    // XXX Find slicker way to allocate enough space in players.
    for(int i=0;i<playerset.size();++i) players.push_back("x");
    copy(playerset.begin(), playerset.end(), players.begin());
}
