#include "cmdline.hpp"
#include "foosrater.h"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

int main(int argc, char** argv)
{
    Cmdline cmdline(argc, argv);
    CmdlineFactory::Init(cmdline);
    string sport = CmdlineFactory::TheCmdline().Sport();
    bool od_distinction = CmdlineFactory::TheCmdline().ODDistinction();

    cout << "<html>\n";
    cout << "<H1><center> " << sport << " ratings </center></H1>\n";

    vector<GameScore> gamescores;
    string infileName(CmdlineFactory::TheCmdline().GamesFile());
    if (infileName == "stdin")
    {
        LoadGameScores(gamescores, 0);
    } else
    {
        ifstream* infile = new ifstream(infileName.c_str());
        LoadGameScores(gamescores, infile);
    }

    vector<string> players;
    cvector mleRatings(MLE(players, gamescores, 0));

    if((sport == "foosball") && (od_distinction))
    {
        vector<double> selfORats, selfDRats;
        vector<double> partnerRats;
        vector<double> opponentRats;
        WinLossStats(players, gamescores,
                     selfORats, selfDRats, partnerRats, opponentRats);
        Display(selfORats, selfDRats, partnerRats, opponentRats, mleRatings,
                players);
        // RodGraph(mleRatings, players);
    }

    if(CmdlineFactory::TheCmdline().HistoryLength() > 0)
    {
        RatingsHistory(players, gamescores);
        int ret = system(string("gnuplot " + tmpdir() + "/cmds.gp").c_str());
        assert(!ret);
    }

    // Need to check if, for foosball, we have to take the total or half the
    // total rating, for expected margin calculation.  In any event the
    // expected margin thing will only work if the Bayesian adjustment is turned
    // off (i.e. logistic_s is set to 1000 or so).
    if((sport == "pingpong") || !od_distinction)
    {
        ExpectedMarginHtmlTable(players, mleRatings);
        GraphViz(mleRatings, players, gamescores);
    }
}
