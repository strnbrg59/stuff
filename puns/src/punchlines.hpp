#ifndef _PUNCHLINES_HPP_
#define _PUNCHLINES_HPP_

#include "phonics.hpp"
#include <string>
#include <vector>

struct Punchline
{
    std::string english;
    std::string normalizedEnglish;
    std::string phonetic;
    BytePhoneticString bytePhonetic;
    std::vector<std::string::const_iterator> wordPositions;
};

class Punchlines
{
  public:
    Punchlines( char const * infilename, Phonics const & );
    std::vector<Punchline>::const_iterator begin() const;
    std::vector<Punchline>::const_iterator end() const;
  private:
    std::vector<Punchline> m_punchlines;
    Phonics const & m_phonics;
};

#endif // _PUNCHLINES_HPP_
