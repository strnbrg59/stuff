#ifndef INCLUDED_PLACEMENT_NEW_H
#define INCLUDED_PLACEMENT_NEW_H

#include <vector>

/** ---- PlacementNew ----
 * Manages a memory buffer and constructs arbitrary objects inside it.
*/
class PlacementNew
{
  public:
    PlacementNew( int bufQuantum );
    ~PlacementNew();
    template<class T> T * New( T );
    void UnitTest();
  private:
    vector<char *> m_bufs;
    int const m_bufQuantum;  // Always grows by integral multiple of this
    char * m_lastBuf;
    int m_lastBufSize;       // In last one on the vector (i.e. the most
    int m_lastBufPosition;   // recently allocated one).
};

ostream & operator<<( ostream & ost, PlacementNew const & pn )
{
    ost << "<instance of PlacementNew>";
    return ost;
}

#include "PlacementNew.cpp"

#endif
