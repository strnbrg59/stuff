#include "foosrater.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <map>
#include <string>
#include <values.h>
#include "cmdline.hpp"
using namespace std;

void Display(vector<double> const& selfOMargs,
             vector<double> const& selfDMargs,
             vector<double> const& partnerMargs,
             vector<double> const& opponentMargs,
             cvector const& mleRatings,
             vector<string> const& players)
{
    int const precision = 2;
    int dbl = CmdlineFactory::TheCmdline().ODDistinction() ? 2 : 1;
    cout << "<pre>\n";
    cout << setw(23) << ""
         << setw(29) << "------- rating ----------------- "
         << setw(68) << "----------------- average margin of victory -------------\n";
    cout << setw(28) << "total"
         << setw(14) << "offense"
         << setw(14) << "defense"
         << setw(24) << "self(offense)"
         << setw(14) << "self(defense)"
         << setw(15) << "partners"
         << setw(15) << "opponents" << '\n';
    for(unsigned p=0;p<players.size();++p)
    {
        cout << setw(14) << players[p]
             << setw(14) << setprecision(precision) << fixed << mleRatings[dbl*p] + mleRatings[dbl*p+dbl-1]
             << setw(14) << setprecision(precision) << fixed << mleRatings[dbl*p]
             << setw(14) << setprecision(precision) << fixed << mleRatings[dbl*p+dbl-1];
        if(selfOMargs[p] > INVALIDMARG)
        {
           cout << setw(24) << setprecision(precision) << fixed << selfOMargs[p];
        } else
        {
            cout << setw(24) << "- ";
        }
        if(selfDMargs[p] > INVALIDMARG)
        {
           cout << setw(14) << setprecision(precision) << fixed << selfDMargs[p];
        } else
        {
            cout << setw(14) << "- ";
        }

        if(partnerMargs[p] > INVALIDMARG)
        {
            cout << setw(15) << setprecision(precision) << fixed << partnerMargs[p];
        } else
        {
            cout << setw(15) << "- ";
        }

        if(opponentMargs[p] > INVALIDMARG)
        {
            cout << setw(15) << setprecision(precision) << fixed << opponentMargs[p];
        } else
        {
            cout << setw(15) << "- ";
        }
        cout << '\n';
    }
    cout << "</pre>\n";
}

void GraphViz(cvector const& mleRatings,
              vector<string> const& players,
              vector<GameScore> const& gamescores)
{
    assert(CmdlineFactory::TheCmdline().ODDistinction() == 0);
    string sport = CmdlineFactory::TheCmdline().Sport();
    ofstream out((tmpdir() + "/graphviz.dat").c_str());

    out << "digraph {\n";

    map<double, string> ratingMap;
    for(unsigned p=0;p<players.size();++p)
    {
        double rating = 2*mleRatings[p];
        ratingMap[rating] = players[p];
    }

    for(int p=players.size()-1;p>=0;--p)
    {
        out << p << " [shape=none, style=invisible]\n";
    }
    for(int p=players.size()-1;p>0;--p)
    {
        out << p << " -> ";
    }
    out << "0 [style=invisible]\n";

    for(unsigned g=0;g<gamescores.size();++g)
    {
        int w = gamescores[g]._winningScore;
        int l = gamescores[g]._losingScore;
        
        double marginHue;
        if(sport=="pingpong")
        {
            marginHue = ((min(w-l+0., .9*w)-2)/(.9*w-2))
                             *2/3.; // red to blue
        } else
        {
            marginHue = (w-l-1)*(2/3.0)/(5-1.0);
        }
        out << gamescores[g]._winnerO << " -> " << gamescores[g]._loserO
            << " [color=\"" << marginHue << " 1 0.9\"]\n";
    }

    int p=0;
    for(map<double, string>::const_iterator iter=ratingMap.begin();
        iter != ratingMap.end(); ++iter)
    {
        out << "{rank=same; " << iter->second << "; " << p << "}\n";
        ++p;
    }

    out << "}\n";
}
void RodGraph(cvector const& mleRatings,
              vector<string> const& players)
{
    // Find scaling factors.
    int dbl = CmdlineFactory::TheCmdline().ODDistinction() ? 2 : 1;
    double minrat=DBL_MAX, maxrat=-DBL_MAX;
    for(unsigned p=0;p<players.size();++p)
    {
        double tot_rating = mleRatings[dbl*p] + mleRatings[dbl*p+dbl-1];
        minrat = tot_rating < minrat ? tot_rating : minrat;
        maxrat = tot_rating > minrat ? tot_rating : maxrat;
    }
    double const scale(300/(maxrat-minrat));
    double const offset(-scale*minrat);

    ofstream outfile("rodgraph.html");
    string heading(
        string("<html>\n") +
        "  <head>\n" +
        "<title>" + CmdlineFactory::TheCmdline().Sport()
                  + " ratings</title>\n" +
        "\n" +
        "<BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#FFCC33\" VLINK=\"#CCCCCC\" ALINK=\"#FFFFFF\">\n" +
        "\n" +
        "<table border=\"0\" cellpadding=\"0\" cellspacing=\"5\">\n"
        );
    string closing("</table>\n"
                   "</body>\n"
                   "</html>\n");

    outfile << heading;
    for(unsigned p=0;p<players.size();++p)
    {
        double tot_rating = mleRatings[dbl*p] + mleRatings[dbl*p+dbl-1];
        outfile <<
            "<tr>\n"
            "<td ALIGN=\"left\" VALIGN=\"middle\"><img src=\"./images/bar1x9.gif\" width=\""
            << offset + tot_rating*scale
            << "\" height=\"9\" hspace=\"0\" vspace=\"7\" alt=\""
            << tot_rating
            << "\"><img src=\"./images/handle108x24.gif\" width=\"108\" height=\"24\" hspace=\"0\" vspace=\"0\" alt=\""
            << tot_rating
            << "\"></td>\n"
            "<td ALIGN=\"left\" VALIGN=\"middle\"><b>"
            << players[p]
            << "</b></td>\n"
            "</tr>\n";
    }
    outfile << closing;
}


double ExpectedMargin(double r1, double r2)
{
    string sport = CmdlineFactory::TheCmdline().Sport();
    int w;
    assert((sport=="foosball") || (sport=="pingpong"));
    if(sport=="foosball") w=5;
    else                  w=21;

    double margin = w*(1 - exp(-abs(r1-r2)));
    return r1>r2 ? margin : -margin;
}

// When printing the margins for the top player, print the other players' names,
// in html comments, so we can base the gnuplot barchart on that data.
void ExpectedMarginHtmlTable(vector<string> const& players,
                             cvector const& mleRatings)
{
    typedef map<string,double> DataT;
    typedef DataT::const_iterator DataIterT;
    string sport = CmdlineFactory::TheCmdline().Sport();

    DataT data;
    int dbl = CmdlineFactory::TheCmdline().ODDistinction() ? 2 : 1;
    double maxRating = -DBL_MAX;
    string topPlayer;
    for(unsigned p=0;p<players.size();++p)
    {
        data[players[p]] = mleRatings[dbl*p] + mleRatings[dbl*p+dbl-1];
        if(data[players[p]] > maxRating)
        {
            maxRating = data[players[p]];
            topPlayer = players[p];
        }
    }

    ofstream barchartdata((tmpdir() + "/spread_against_best.gpd").c_str());
    barchartdata << "name score\n";

    cout << "<HTML>\n";
    cout << "<H4><CENTER> Expected Point Spreads </CENTER></H4>\n";
    cout << "See below for technical and nontechnical explanations.<BR><BR>\n";

    cout << "<TABLE BORDER=1 CELLPADDING=10 CELLSPACING=1>\n";
    cout << "<TR><TD></TD>\n";

    // Player's abbreviated names along the top:
    for(DataIterT i = data.begin(); i != data.end(); ++i)
    {
        cout << "<TD> " << string(i->first, 0, 4) << "</TD>\n";
    }

    // Player by player rows:

    int w;
    if(sport=="foosball") w=5;
    else                  w=21;
    for(DataIterT i = data.begin(); i != data.end(); ++i)
    {
        cout << "<TR>\n";
        cout << "<TD> " << i->first << "</TD>\n";  // player name
        for(DataIterT j = data.begin(); j != data.end(); ++j)
        {
            double em = ExpectedMargin(i->second, j->second);
            if(i->first == j->first)
            {
                cout << "<TD></TD>\n";
            } else
            {
                cout << "<TD> " << setw(4) << setprecision(1) << fixed << em << "</TD>\n";
            }
            if(i->first == topPlayer)
            {
                barchartdata << j->first << " " << w - em << '\n';
            }
        }
        cout << "</TR>\n";
    }
    cout << "</TABLE>\n";
    cout << "</HTML>\n";
}

string
tmpdir()
{
    if (!getenv("USER"))
    {
        cerr << "Please define USER in your environment, e.g. "
             << "\"export USER=smith\"" << '\n';
        exit(1);
    }
    string result = "/tmp/" + string(getenv("USER"));
    int err = system((string("mkdir -p ") + result).c_str());
    if (err)
    {
        cerr << "Unable to create staging directory " << result << ".  Please\n"
             << "fix the permissions there.\n";
        exit(1);
    }
    return result;
}
