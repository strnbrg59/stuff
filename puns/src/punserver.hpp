#ifndef _PUNSERVER_HPP_
#define _PUNSERVER_HPP_

#include "punchlines.hpp"
#include <string>
#include <vector>
using std::map;
using std::multimap;
using std::string;
using std::vector;
class NetworkUtils;
class Cmdline;

class PunServer
{
  public:
    PunServer( Cmdline &, Phonics const &, Punchlines const & );
  private:
    string FindPun( string clue );
    int ConverseWithNetClient( int socket, NetworkUtils & netutils );
    
    void UnpackCluePackage( string cluePkg, string * clue,
                            int * badness, bool * multispan );
    
    vector<int> FindNormalizedWordStarts( string rawVerse,
                                          string normalizedVerse ) const;
    
    string
    TagMatch( vector<Punchline>::const_iterator punchlinesIter,
              BytePhoneticString bytePhoneticClue,
              BytePhoneticString::const_iterator positionInBytePhoneticPunchline,
              int * nWordsSpanned );
    
    //
    // Data
    //
    Cmdline& m_cmdline;
    Phonics const & m_phonics;
    Punchlines const & m_punchlines;
};

#endif // _PUNSERVER_HPP_
