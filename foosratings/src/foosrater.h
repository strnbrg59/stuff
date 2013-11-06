#include <string>
#include <vector>
#include <set>
#include <iostream>
#include "ols.hpp"

#define INVALIDMARG -9999

class GameScore {
  public:
    GameScore( std::string winnerO, std::string winnerD,
               std::string loserO, std::string loserD,
               int losingScore, int winningScore) :
        _winnerO(winnerO),
        _winnerD(winnerD),
        _loserO(loserO),
        _loserD(loserD),
        _losingScore(losingScore),
        _winningScore(winningScore),
        _marginError(0.0)
    {
    }

  void Replace(std::string const& playerFrom, std::string const& playerTo);

  void GetMatchup( std::string matchup[4] ) const;
  std::ostream& Print(std::ostream&) const;
  
  friend cvector MLE(std::vector<std::string>&,
                     std::vector<GameScore> const&,
                     int tail);
  friend void WinLossStats(std::vector<std::string> const& players,
                           std::vector<GameScore>& scores,
                           std::vector<double>& selfORats,
                           std::vector<double>& selfDRats,
                           std::vector<double>& partnerRats,
                           std::vector<double>& opponentRats);
  friend void RatingsHistory(std::vector<std::string> const&,
                             std::vector<GameScore>&);

  friend void GraphViz(cvector const& mleRatings,
                       std::vector<std::string> const& players,
                       std::vector<GameScore> const& gamescores);

  std::string winnerO() const { return _winnerO; }
  std::string winnerD() const { return _winnerD; }
  std::string loserO() const  { return _loserO; }
  std::string loserD() const  { return _loserD; }

  private:
    std::string _winnerO, _winnerD, _loserO, _loserD;
    int _losingScore;
    int _winningScore;
    double _marginError; // Relative to favorite's expected margin.
};


std::ostream& operator<<(std::ostream&, GameScore const&);

void LoadGameScores( std::vector<GameScore>& scores, std::istream* infileName);
void AssemblePlayers(std::vector<std::string>& players,
                     std::vector<GameScore> const& scores,
                     int tail);

cvector MLE(std::vector<std::string>& players,
            std::vector<GameScore> const& scores, int tail);
void WinLossStats(std::vector<std::string> const& players,
                  std::vector<GameScore>& scores,
                  std::vector<double>& selfORats,
                  std::vector<double>& selfDRats,
                  std::vector<double>& partnerRats,
                  std::vector<double>& opponentRats);
void Display(std::vector<double> const& selfORats,
             std::vector<double> const& selfDRats,
             std::vector<double> const& partnerRats,
             std::vector<double> const& opponentRats,
             cvector const& mleRatings,
             std::vector<std::string> const& players);
void RodGraph(cvector const& mleRatings,
              std::vector<std::string> const& players);
void RatingsHistory(std::vector<std::string> const& players,
                    std::vector<GameScore>& gamescores);
double ExpectedMargin(double r1, double r2);
void ExpectedMarginHtmlTable(std::vector<std::string> const& players,
                             cvector const& mleratings);
void GraphViz(cvector const& mleRatings,
              std::vector<std::string> const& players,
              std::vector<GameScore> const& gamescores);

std::string tmpdir();
