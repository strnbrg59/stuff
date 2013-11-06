// Author: Ted Sternberg, strnbrg@trhj.homeunix.net

#include <string>
#include <vector>

class Reader
{
  public:
    std::vector<std::string> ReadCommands();
  private:
    enum       {m_maxBytes = 128};
    char       m_buf[ m_maxBytes ];
    int        m_leftover;
};

Reader * TheReader();
