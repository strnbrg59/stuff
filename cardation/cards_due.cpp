#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
using namespace std;

int g_T = 10;
int g_W = 1;
double g_F = 0.0;
bool g_H = false;

void getopts(int argc, char* argv[])
{
    const char* getopt_options = "T:W:F:H:";
    int optchar = getopt(argc, argv, getopt_options);
    while (optchar != -1)
    {
        switch (optchar)
        {
            case 'T': g_T = atoi(optarg); break;
            case 'W': g_W = atoi(optarg); break;
            case 'F': g_F = atof(optarg); break;
            case 'H': g_H = atoi(optarg); break;
            default : cerr << "Usage: " << argv[0] << " -T <niters> "
                           << " -W <words_per_day> "
                           << " -F <forget_fraction> "
                           << " -H <produce_html> "
                           << endl;
                      exit(1);
        }
        optchar = getopt(argc, argv, getopt_options);
    }
/*
    cout << "T=" << g_T << '\n';
    cout << "W=" << g_W << '\n';
    cout << "F=" << g_F << '\n';
    cout << "H=" << g_H << '\n';
*/
}

struct Word
{
    int due;
    int streak;
    Word(int d, int s) : due(d), streak(s) {}
};

double simulate(int T, int W, double F)
{
    typedef vector<Word> WordVect;
    WordVect words;
    int const ave_size = 20;
    double ave_last_n_due = 0.0;
    int last_1_due = 0;
    for (int t=0; t<T; ++t)
    {
        for (int w=0;w<W;++w)
        {
            words.push_back(Word(t, -1));
        }

        int n_due = 0;
        for (WordVect::iterator i = words.begin(); i!=words.end(); ++i)
        {
            if (i->due <= t)
            {
                ++n_due;
                if (double(random()) < F*RAND_MAX)
                {
                    i->streak = 0;
                } else
                {
                    i->streak += 1;
                }
                i->due = t + int(pow(2, i->streak));
            }
        }
//      cout << "(t, n_due) = (" << t << ", " << n_due << ")\n";

        // If F!=0.0, there'll be some variation.
        if ((T-t) < ave_size)
        {
            ave_last_n_due += n_due;
        }
        last_1_due = n_due;
    }

    if (fabs(F) < 0.00001)
    {
        return last_1_due;
    } else
    {
        return ave_last_n_due/ave_size;
    }
}    

void html()
{
    vector<int> Ts = boost::assign::list_of(365)(1825)(3650);
    vector<int> Ws = boost::assign::list_of(1)(2)(5)(10);
    vector<double> Fs = boost::assign::list_of(0.0)(0.05)(0.1)(0.15);

    cout << "<!-- produced with cards_due.cpp -->\n";
    cout << "<TABLE border=\"1\" align=\"center\">" << endl;
    cout << "<TR> <TD></TD>\n";

    // Top row -- Fs
    BOOST_FOREACH(double F, Fs)
    {
        cout << "<TD>" << F*100 << "% forgotten</TD>\n";
    }

    BOOST_FOREACH(int T, Ts)
    {
        cout << "<TR>\n";
        cout << "<TD> " << T << " days</TD>\n";
        BOOST_FOREACH(double F, Fs)
        {
            cout << "<TD> " << simulate(T, 1, F) << "</TD>\n";
        }
        cout << "</TR>\n";
    }

    cout << "</TABLE>\n";
}

int main(int argc, char* argv[])
{
    getopts(argc, argv);
    if (!g_H)
    {
        cout << "Average n_due towards end = "
                << simulate(g_T, g_W, g_F) << '\n';
    } else
    {
        html();
    }
}
