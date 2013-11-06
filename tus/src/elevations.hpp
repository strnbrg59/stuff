#ifndef INCLUDED_ELEVATIONS_HPP
#define INCLUDED_ELEVATIONS_HPP

#include <vector>
#include <string>

class Elevations
{
  public:
    Elevations();
    double val(int i, int j) const { return m_rep[i][j]; }
    enum {srtmSize=3601};
  private:
    void GenerateFakeScenery();
    void LoadRealScenery(char const* infilename);

    std::vector<std::vector<double> > m_rep;
};

#endif // include guard
