#include "cmdline.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
using namespace std;

struct Player
{
    Player(string name_, double betaO_, double betaD_)
        : name(name_), betaO(betaO_), betaD(betaD_)  { }
    Player(Player const& that)
        : name(that.name), betaO(that.betaO), betaD(that.betaD) { }
    string name;
    double betaO, betaD;
    ostream& print(ostream& out) const {
        out << "(" << name << ": " << betaO << ", " << betaD << ")";
        return out;
    }
};
ostream& operator<<(ostream& out, Player const& p)
{
    return p.print(out);
}


int main(int argc, char** argv)
{
    Cmdline cmdline(argc, argv);
    CmdlineFactory::Init(cmdline);

    int const n_players = cmdline.SimulateNPlayers();
    int const n_games = cmdline.SimulateNGames();
    int const winningScore = 5;

    //
    // Generate players and their betas.
    //
    vector<Player> players;
    double totBetaOs = 0.0;
    double totBetaDs = 0.0;
    char curName = 'a';
    for(int p=0;p<n_players;++p)
    {
        ostringstream name;
        name << curName++;
        double betaO, betaD;
        if(p<n_players-1)
        {
            betaO = 2.0*p/n_players - 1;
            betaD = 2.0*(n_players-p)/n_players - 1;
        } else
        {
            betaO = -totBetaOs;
            betaD = -totBetaDs;
        }
        totBetaOs += betaO;
        totBetaDs += betaD;
        Player player(Player(name.str(), betaO, betaD));
        players.push_back(player);
    }

    //
    // Generate games.
    //
    srand(10);
    for(int g=0;g<n_games;++g)
    {
        std::set<string> fourNames;
        vector<Player> fourPlayers;
        while(fourNames.size() < 4)
        {
            int p = random()%n_players;
            if(find(fourNames.begin(), fourNames.end(), players[p].name)
                == fourNames.end())
            {
                fourNames.insert(players[p].name);
                fourPlayers.push_back(players[p]);
            }
        }
        /*
        cout << "foursome: ";
        for(int p=0; p<4; ++p) cout << fourPlayers[p].name << ' ';
        cout << '\n';
        */
        //
        // Generate a game.
        //
        int singlesOffset = 
            (random() < 
             CmdlineFactory::TheCmdline().SimulateSinglesProb()*RAND_MAX)*1;
        double betaSum = fourPlayers[0].betaO 
                            + fourPlayers[1-singlesOffset].betaD
                       - fourPlayers[2].betaO
                            - fourPlayers[3-singlesOffset].betaD;
        double prob = exp(betaSum)/(1+exp(betaSum));
        int scoreFavorite=0;
        int scoreUnderdog=0;
        while((scoreFavorite<winningScore) && (scoreUnderdog<winningScore))
        {
            if(random() < RAND_MAX*prob)
            {
                scoreFavorite++;
            } else
            {
                scoreUnderdog++;
            }
        }
        if(scoreFavorite==winningScore)
        {
            cout << fourPlayers[0].name << ' ' 
                    << fourPlayers[1-singlesOffset].name << ' '
                 << winningScore << ' '
                 << fourPlayers[2].name << ' ' 
                    << fourPlayers[3-singlesOffset].name << ' '
                 << scoreUnderdog << '\n';
        } else
        {
            cout << fourPlayers[2].name << ' ' 
                    << fourPlayers[3-singlesOffset].name << ' '
                 << winningScore << ' '
                 << fourPlayers[0].name << ' ' 
                    << fourPlayers[1-singlesOffset].name << ' '
                 << scoreFavorite << '\n';
        }
    }
}
