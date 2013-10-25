#include <cstdlib>
#include <vector>
#include <iostream>
#include <boost/foreach.hpp>
using namespace std;

ostream& operator<<(ostream& out, vector<int> const& v)
{
    BOOST_FOREACH(int const& s, v)
    {
        out << s << '\n';
    }
    return out;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " n p" << '\n';
        exit(1);
    }
    int const n = atoi(argv[1]);
    double const p = atof(argv[2]);
    int const n_sims = 10000;
    vector<int> streaks(n_sims);
    vector<int> successes(n_sims);

    for (int i=0;i<n_sims;++i)
    {
        streaks[i] = 0;
        successes[i] = 0;
        int streak = 0;
        for (int j=0;j<n;++j)
        {
            bool success = (random() < p*RAND_MAX);
            if (success)
            {
                ++streak;
                ++successes[i];
                if (streak > streaks[i])
                {
                    streaks[i] = streak;
                }
            } else
            {
                streak = 0;
            }
        }
    }

    cout << streaks;
//  cout << successes << '\n';
}

