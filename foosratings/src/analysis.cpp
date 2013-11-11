#include "trace.hpp"
#include "ols.hpp"
#include "foosrater.h"
#include "cmdline.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <gsl/gsl_multimin.h>

using namespace std;

int findElem(vector<string> const& sortedvector, string name)
{
    return find(sortedvector.begin(), sortedvector.end(), name)
           - sortedvector.begin();
}

struct LikelihoodParams
{
    LikelihoodParams(cvector const& w_, cvector const& m_, matrix const& X_)
        : w(w_), m(m_), X(X_) { }
    cvector const& w; // Winning scores
    cvector const& m; // Losing scores
    matrix const& X;
};

double
likelihood(const gsl_vector* beta, void* params)
{
    cvector const& w = ((LikelihoodParams*)params)->w;
    cvector const& m = ((LikelihoodParams*)params)->m;
    matrix const& X = ((LikelihoodParams*)params)->X;
    double result = 0;
    double weight_decay = CmdlineFactory::TheCmdline().WeightDecay();
    for(int g=0;g<m.shape()[0];++g)
    {
        double xb=0;
        for(int k=0;k<X.shape()[1];++k)
        {
            xb += gsl_vector_get(beta,k) * X[g][k];
        }
        double p = exp(xb)/(1+exp(xb));
        double partial = w[g]*log(p) + m[g]*log(1-p);
        // Combinatorial term doesn't contain p and so can be ignored.
        result -= partial*pow(weight_decay, double(-g));
    }
    // Add in logistic priors for the betas (mu=0, sigma=s).
    double s=CmdlineFactory::TheCmdline().LogisticS();
    double logistic_component = 0;
    for(int k=0;k<X.shape()[1];++k)
    {
        double e = exp(gsl_vector_get(beta,k)/s);
        logistic_component += log((1/s)*e/(1+e));
    }
    //cout << "logistic/negbin=" <<  logistic_component/result << '\n';
    result -= logistic_component;
    return result;
}

void
likelihood_gradient(const gsl_vector* beta, void* params, gsl_vector* gradient)
{
    Trace t("likelihood_gradient");
    double weight_decay = CmdlineFactory::TheCmdline().WeightDecay();
    cvector const& w = ((LikelihoodParams*)params)->w;
    cvector const& m = ((LikelihoodParams*)params)->m;
    matrix const& X = ((LikelihoodParams*)params)->X;
    assert(X.shape()[0] == m.shape()[0]);

    cvector gradient_accumulator(boost::extents[X.shape()[1]]);
    for(int k=0;k<X.shape()[1];++k)
    {
        gradient_accumulator[k] = 0;
    }
    for(int g=0;g<m.shape()[0];++g)
    {
        double xb=0;
        for(int k=0;k<X.shape()[1];++k)
        {
            xb += gsl_vector_get(beta,k) * X[g][k];
        }
        double p = exp(xb)/(1+exp(xb));
        double dLdp = w[g]/p - int(m[g]+.5)/(1-p);
        double decay = pow(weight_decay, double(-g));
        for(int k=0;k<X.shape()[1];++k)
        {
           gradient_accumulator[k] -= dLdp * X[g][k]*exp(xb)/pow(1+exp(xb),2)
                                    * decay;
        }
    }
    // Logistic prior part:
    double s=CmdlineFactory::TheCmdline().LogisticS();
    for(int k=0;k<X.shape()[1];++k)
    {
        double e = exp(gsl_vector_get(beta,k)/s);
        double logistic_component = 1/(s*(1+e));
        gradient_accumulator[k] -= logistic_component;
    }
    for(int k=0;k<X.shape()[1];++k)
    {
        gsl_vector_set(gradient, k, gradient_accumulator[k]);
    }

    //
    // Numerical computation of the gradient.  Use this to check your analytical
    // gradient.
    //
    static bool checkedNumericalGradient = false;
    if(!checkedNumericalGradient)
    {
        double dx=0.000000001*likelihood(beta, params);
        for(int k=0;k<X.shape()[1];++k)
        {
            double fx = likelihood(beta, params);
            gsl_vector_set((gsl_vector*)beta, k, gsl_vector_get(beta, k) + dx);
            double fdx = likelihood(beta, params);
            gsl_vector_set((gsl_vector*)beta, k, gsl_vector_get(beta, k) - dx);
            double num = (fdx-fx)/dx;
            double anal = gsl_vector_get(gradient, k);
            double grad_pct_diff = (num-anal)/num;
            if(fabs(grad_pct_diff) > .0001)
            {
                t.Warning("gradient: (numerical-analytical)/numerical=%f",
                         grad_pct_diff);
            }
        }
        checkedNumericalGradient = true;
    }
}

void
my_fdf(const gsl_vector *x, void *params,
       double *f, gsl_vector *df)
{
  *f = likelihood(x, params);
  likelihood_gradient(x, params, df);
}


cvector MLE(vector<string>& players, vector<GameScore> const& scores, int tail)
{
    Trace t("MLE");
    AssemblePlayers(players, scores, tail);
    int dbl = CmdlineFactory::TheCmdline().ODDistinction() ? 2 : 1;
    string sport = CmdlineFactory::TheCmdline().Sport();
    matrix X_degenerate(boost::extents[scores.size()-tail][dbl*players.size()]);
    matrix X(boost::extents[scores.size()-tail][dbl*players.size()-dbl]);

    for(int i=0;i<scores.size()-tail;++i)
    {
        for(int j=0;j<dbl*players.size();++j)
        {
            // XXX Isn't there a slicker way to initialize a boost matrix?
            X_degenerate[i][j] = 0;
        }
    }

    cvector w(boost::extents[scores.size()-tail]);
    cvector m(boost::extents[scores.size()-tail]);

    int foosfactor = 1;
    if((sport == "foosball") && (dbl==2)) foosfactor = 2;
    for(int t=0; t<scores.size()-tail; ++t)
    {
        double singlesFactor = (scores[t]._winnerO == scores[t]._winnerD) ?
                                2 : 1;
        X_degenerate[t][dbl*findElem(players, scores[t]._winnerO)] =
        X_degenerate[t][dbl*findElem(players, scores[t]._winnerD) + dbl-1]
            = singlesFactor/foosfactor;
        X_degenerate[t][dbl*findElem(players, scores[t]._loserO)] =
        X_degenerate[t][dbl*findElem(players, scores[t]._loserD) + dbl-1]
            = -singlesFactor/foosfactor;

        w[t] = int(scores[t]._winningScore + .5);
        m[t] = int(scores[t]._losingScore + .5);

        for(int j=0; j<players.size()-1; ++j)
        {
            // Constrain sum(offensive beta)=sum(defensive beta) = 0
            // (otherwise X is degenerate).
            X[t][dbl*j] = X_degenerate[t][dbl*j]
                        - X_degenerate[t][dbl*players.size()-dbl];
            if(dbl==2)
            {
                X[t][2*j+1] = X_degenerate[t][2*j+1] 
                            - X_degenerate[t][2*players.size()-2+1];
            }
        }
    }

    if(CmdlineFactory::TheCmdline().DebugLevel() > 3)
    {   /*
        cout << "X=\n" << X;
        cout << "--------------------------------\n";
        cout << "X_degenerate=\n" << X_degenerate;
        */
        matrix xpx(MatMatMult(MatTranspose(X),X));
        matrix xpxinv(MatInverse(xpx));
        matrix should_be_I(MatMatMult(xpx,xpxinv));
        MatRound(should_be_I, 0.0000001);
        cout << "should_be_I:\n" << should_be_I;
    }

    //
    // Set up multimin (code copied from gsl-1.13/doc/examples/multimin.c)
    //
    int status;
    size_t iter = 0;

    const gsl_multimin_fdfminimizer_type *T;
    gsl_multimin_fdfminimizer *s;
    gsl_vector *x;
    gsl_multimin_function_fdf my_func;

    my_func.n = X.shape()[1];
    my_func.f = likelihood;
    my_func.df = likelihood_gradient;
    my_func.fdf = my_fdf;
    LikelihoodParams lp(w,m,X);
    my_func.params = &lp;

    /* Initial guess */
    x = gsl_vector_alloc(X.shape()[1]);
    for(int k=0;k<X.shape()[1];++k)
    {
      gsl_vector_set (x, k, 0.0);
    }

    T = gsl_multimin_fdfminimizer_conjugate_fr;
    s = gsl_multimin_fdfminimizer_alloc (T, X.shape()[1]);

    gsl_multimin_fdfminimizer_set (s, &my_func, x, 0.01, 1e-4);

    do
    {
        iter++;
        status = gsl_multimin_fdfminimizer_iterate (s);

        if (status)
          break;

        status = gsl_multimin_test_gradient (s->gradient, 1e-3);
/*
        if (status == GSL_SUCCESS)
          t.Info() << "Minimum found at:\n";
        t.Info("iter=%d ", iter);
        for(int k=0;k<X.shape()[1];++k)
        {
          t.Info() << gsl_vector_get(s->x, k) << ' ';
        }
        t.Info("val=%f", s->f);
*/
    } while (status == GSL_CONTINUE && iter < 100);

    //
    // MLE done.  Prepare beta_degenerate (beta plus the statistically
    // unidentifiable parameters of the nth player).
    //
    cvector beta_degenerate(boost::extents[dbl*players.size()]);
    double beta_sumO = 0;
    double beta_sumD = 0;
    for(int j=0;j<players.size()-1;++j)
    {  
        beta_degenerate[dbl*j] = gsl_vector_get(s->x, dbl*j);
        beta_sumO += beta_degenerate[dbl*j];
        if(dbl==2)
        {
            beta_degenerate[2*j+1] = gsl_vector_get(s->x, 2*j+1);
            beta_sumD += beta_degenerate[2*j+1];
        }
    }
    beta_degenerate[dbl*players.size()-dbl] = -beta_sumO;
    if(dbl==2)
    {
        beta_degenerate[2*players.size()-1] = -beta_sumD;
    }

    gsl_multimin_fdfminimizer_free (s);
    gsl_vector_free (x);

    return beta_degenerate;
}

void tabulateOthers(vector<double>& otherMargs,
                    vector<double> const& selfOMarg,
                    vector<double> const& selfDMarg,
                    vector<set<string> > const otherNames,
                    vector<string> players)
{
    for(unsigned p=0;p<players.size();++p)
    {
        double otherMarg=0;
        int n_otherMargs=0; // Includes O and D ratios
        for(set<string>::const_iterator i=otherNames[p].begin();
            i!=otherNames[p].end(); ++i)
        {
            double otherMargO = selfOMarg[findElem(players, *i)];
            if(otherMargO > INVALIDMARG)
            {
                ++n_otherMargs;
            } else
            {
                otherMargO = 0;
            }
            double otherMargD = selfDMarg[findElem(players, *i)];
            if(otherMargD > INVALIDMARG)
            {
                ++n_otherMargs;
            } else
            {
                otherMargD = 0;
            }
            otherMarg += otherMargO + otherMargD;
        }
        if(n_otherMargs > 0)
        {
            otherMarg /= n_otherMargs;
        } else
        {
            otherMarg = INVALIDMARG;
        }
        otherMargs.push_back(otherMarg);
    }
}


void WinLossStats(vector<string> const& players,
                  vector<GameScore>& games,
                  vector<double>& selfOMargs,
                  vector<double>& selfDMargs,
                  vector<double>& partnerMargs,
                  vector<double>& opponentMargs)
{
    vector<set<string> > allPartnerNames, allOpponentNames;
    for(unsigned p=0;p<players.size();++p)
    {
        int marginO=0, marginD=0;
        int gamesPlayedO=0, gamesPlayedD=0;
        set<string> partnerNames, opponentNames;
        string player = players[p];
        for(unsigned g=0;g<games.size();++g)
        {
            int margin = games[g]._winningScore - games[g]._losingScore;
            if(player==games[g]._winnerO)
            {
                ++gamesPlayedO;
                marginO += margin;
                if(games[g]._winnerD != player)
                {
                    partnerNames.insert(games[g]._winnerD);
                }
                opponentNames.insert(games[g]._loserO);
                opponentNames.insert(games[g]._loserD);
            } else if(player==games[g]._loserO)
            {
                ++gamesPlayedO;
                marginO -= margin;
                if(games[g]._loserD != player)
                {
                    partnerNames.insert(games[g]._loserD);
                }
                opponentNames.insert(games[g]._winnerO);
                opponentNames.insert(games[g]._winnerD);
            }
            if(player==games[g]._winnerD)
            {
                ++gamesPlayedD;
                marginD += margin;
                if(games[g]._winnerO != player)
                {
                    partnerNames.insert(games[g]._winnerO);
                }
                opponentNames.insert(games[g]._loserO);
                opponentNames.insert(games[g]._loserD);
            } else if(player==games[g]._loserD)
            {
                ++gamesPlayedD;
                marginD -= margin;
                if(games[g]._loserO != player)
                {
                    partnerNames.insert(games[g]._loserO);
                }
                opponentNames.insert(games[g]._winnerO);
                opponentNames.insert(games[g]._winnerD);
            }
            // else, player didn't play this game
        }
        selfOMargs.push_back(gamesPlayedO > 0 ?
                            (0.0+marginO)/gamesPlayedO : INVALIDMARG);
        selfDMargs.push_back(gamesPlayedD > 0 ?
                            (0.0+marginD)/gamesPlayedD : INVALIDMARG);
        allPartnerNames.push_back(partnerNames);
        allOpponentNames.push_back(opponentNames);
    }

    // Compute partner and opponent stats
    for(unsigned p=0;p<players.size();++p)
    {
        tabulateOthers(partnerMargs, selfOMargs, selfDMargs, allPartnerNames,
                       players);
        tabulateOthers(opponentMargs, selfOMargs, selfDMargs, allOpponentNames,
                       players);
    }
}


/** Run "gnuplot cmds.gp -" to see the graph.
 */
void RatingsHistory(vector<string> const& players,
                    vector<GameScore>& gamescores)
{
    int dbl = CmdlineFactory::TheCmdline().ODDistinction() ? 2 : 1;
    int history_length = std::min(CmdlineFactory::TheCmdline().HistoryLength(),
                                  int(gamescores.size()-1));
    string sport = CmdlineFactory::TheCmdline().Sport();
    ofstream gp_dat((tmpdir() + "/gp.dat").c_str());  // Expected margin vs benchmark player
    ofstream gp_cmds((tmpdir() + "/cmds.gp").c_str());
    ofstream pt_dat((tmpdir() + "/pt.dat").c_str());  // A point on a player's graph, indicating which
                                // games he played.
    ofstream errors_dat((tmpdir() + "/errors.dat").c_str()); // Actual vs expected margin
    ofstream augmented_dat((tmpdir() + "/augmented.dat").c_str()); // Like games file, but more
    vector<string> distinguished_players;
    string const benchmark_player = (sport=="foosball") ?
                                     "tsternberg" : "dchu";
/*
    if(sport == "foosball")
    {
        distinguished_players.push_back("tsternberg");
        distinguished_players.push_back("dchu");
        distinguished_players.push_back("wtai");
        distinguished_players.push_back("ksalman");
        distinguished_players.push_back("kcheung");
        distinguished_players.push_back("gbrown");
        distinguished_players.push_back("jonpaul");
        distinguished_players.push_back("acehreli");
    } else
*/
    {
        distinguished_players = players;
    }
    double offset = -999.999;
    sort(distinguished_players.begin(), distinguished_players.end());
    double w;
    if(sport=="foosball") w=5.0;
    else                  w=21.0;

    int tail=0;
    for(; tail<=history_length; tail+=1)
    {
        bool error_noted = false;
        cerr << '.';
        gp_dat << "-" << tail << " ";

        double benchmark_player_rating;
        map<string,double> distinguished_ratings;
        vector<string> players_at_tail;
        cvector mleRatings(MLE(players_at_tail, gamescores, tail));
        for(unsigned p=0; p<players_at_tail.size(); ++p)
        {
            double rating = mleRatings[dbl*p] + mleRatings[dbl*p+dbl-1];
            if(find(distinguished_players.begin(), distinguished_players.end(),
                    players_at_tail[p]) != distinguished_players.end())
            {
                distinguished_ratings[players_at_tail[p]] = rating;
            }
            if(players_at_tail[p] == benchmark_player)
            {
                benchmark_player_rating = rating;
            }
        }
        for(unsigned dp=0; dp<distinguished_players.size(); ++dp)
        {
            if(distinguished_ratings.find(distinguished_players[dp]) == distinguished_ratings.end())
            {
                distinguished_ratings[distinguished_players[dp]] = -999;
            }
        }

        for(unsigned dp=0; dp<distinguished_players.size(); ++dp)
        {
            string player = distinguished_players[dp];
            double expected_margin_vs_benchmark = ExpectedMargin(distinguished_ratings[player],
                                                                 benchmark_player_rating);
            if(distinguished_ratings[player] < -990)
            {
                gp_dat << setw(14) << '-';
            } else
            {
                gp_dat << setw(14) << expected_margin_vs_benchmark;
            }
            GameScore& gs(gamescores[gamescores.size()-1-tail]);
            if((player==gs._winnerO) || (player==gs._winnerD)
            || (player==gs._loserO) || (player==gs._loserD))
            {
                pt_dat << -tail << " " << expected_margin_vs_benchmark << '\n';

                double expected_margin_game =
                    ExpectedMargin(distinguished_ratings[gs._winnerO],
                                   distinguished_ratings[gs._loserO]);
                double actual_margin_game = (w/gs._winningScore)
                                          * (gs._winningScore - gs._losingScore);
                if(!error_noted)
                {
                    double error = actual_margin_game - expected_margin_game;
                    if(expected_margin_game < 0) { error *= -1; }
                    // We display the amount by which the favorite beat the expected margin.
                    // Without that "error *= -1" sign flip we'd be displaying the amount by
                    // which the *winner* beat the expected margin.
                    errors_dat << -tail << " " << error << " "
                           << gs._winnerO << " " << gs._loserO << "\n";
                    gs._marginError = error;
                    augmented_dat<<gs._winnerO << " " << gs._winningScore << " "
                                 << gs._loserO << " " << gs._losingScore << " "
                                 << error << '\n';
                    error_noted = true;
                }
            }
        }
        gp_dat << '\n';
    }

    //
    // Gnuplot data columns can having missing values; indicate them with
    // a dash.
    //
    if(!getenv("NO_GNUPLOT_GIF"))
    {
        gp_cmds << "set terminal post size 20,20 enhanced color\n";
//      gp_cmds << "set terminal gif\n";
        gp_cmds << "set output \"" << tmpdir() << "/history.ps\"\n";
    }
    gp_cmds << "set xlabel 'Games before latest'\n";
    gp_cmds << "set ylabel 'points (out of " << w << ") --circles indicate games played in'\n";
    gp_cmds << "set title 'history of expected point spreads versus "
            << benchmark_player << '\n';
    gp_cmds << "set key box\n";
    gp_cmds << "set key height 2\n"; // Fiddle with this if any of the legend
                                     // lines are covered by part of a graph.
    gp_cmds << "set key left top\n";
    gp_cmds << "set pointsize 1\n";
    gp_cmds << "set grid\n";
    gp_cmds << "set xrange [:1]\n";
    gp_cmds << "set xtics " << -(int(tail/10 + .5))*10 << ", 10\n";
    gp_cmds << "plot ";
    for(unsigned p=0; p<distinguished_players.size(); ++p)
    {
        gp_cmds << "'" << tmpdir() << "/gp.dat' u 1:" << (p+2) << " t '" << distinguished_players[p]
                << "' w lines linewidth 9, ";
    }
    gp_cmds << " '" << tmpdir() << "/pt.dat' w points pointtype 6 lt rgb \"#000000\" notitle";
    if (sport == "pingpong")
    {
        gp_cmds << ", '" << tmpdir() + "/errors.dat' w impulses t 'error'\n";
    }
}
