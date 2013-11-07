#include <map>
#include <multimap.h>
#include <string>
#include <vector>
#include <iosfwd>

using std::vector;
using std::pair;
using std::ostream;
using std::string;
using std::map;

typedef pair<int,int> Corner;

/** The "CV" prefix is because on CygWin, there's a class called Rectangle in
 *  some system library or something like that.
*/
struct CVRectangle
{
    CVRectangle() { }
    CVRectangle( Corner lo, Corner hi, int id=0 );
    CVRectangle( CVRectangle const & );

    void ShiftTo( int x, int y );

    ostream & PrintAscii( ostream & ) const;
    ostream & PrintPostscript( ostream &, double hsv[3] ) const;

    Corner m_lo; // lower-left-hand corner, first=x, second=y.
    Corner m_hi; // upper-right-hand corner
      // Number of cells = (1+hi.first-lo.first)*(1+hi.second-lo.second)
      // Convention follows ChomboVis Box class.
    int id; // ID number -- to keep track of who's who when we sort.

};
ostream & operator<<( ostream&, CVRectangle const & );
bool operator<( CVRectangle const & rect1, CVRectangle const & rect2 );

struct PackStats
{
    int nRectangles;
    pair<int,int> domainSize;
    int domainArea;
    int usedDomainArea;
    ostream & Print( ostream & ) const;
};
ostream & operator<<( ostream&, PackStats const & );


/** Subclass must define Pack() method. */
class RectanglePacker
{
  public:
    RectanglePacker( vector<CVRectangle> const & );
    virtual ~RectanglePacker();

    // The main thing.
    virtual void Pack() = 0;

    // Info.
    CVRectangle GetPackedRectangle( int id ) const;
    CVRectangle GetUnpackedRectangle( int id ) const;
    CVRectangle GetPackedDomain() const;
    void Stats( PackStats & ) const;

    // Printing.
    void PackedToAscii( ostream & ) const;
    void UnpackedToAscii( ostream & ) const;
    void PackedToPostscript( string outfileName ) const;
    void UnpackedToPostscript( string outfileName ) const;

  protected:
    void _ToPostscript( string outfileName,vector<CVRectangle> const &)
        const;
    void _ToAscii( ostream &, vector<CVRectangle> const & ) const;
    void _ComputePackedDomain();

    vector<CVRectangle> const & m_unpackedRectangles;
    vector<CVRectangle>         m_packedRectangles;
    CVRectangle m_packedDomain;
    mutable map<int,int> m_keyToPackedRectangles;
    mutable map<int,int> m_keyToUnpackedRectangles;
    bool m_calledPack;
};


/** Leaves rectangles where they are. */
class NullRectanglePacker : public RectanglePacker
{
  public:
    NullRectanglePacker( vector<CVRectangle> const & );
    virtual ~NullRectanglePacker();

    void Pack();
};


/** Packs rectangles in a long row. */
class OKRectanglePacker : public RectanglePacker
{
  public:
    OKRectanglePacker( vector<CVRectangle> const & );
    virtual ~OKRectanglePacker();

    void Pack();

  private:

    typedef multimap<int,int> HeadroomRep;
    class Headroom
    {
      public:
        Headroom( int maxHeight );
        int FindRoom( int height ) const;
        void MarkUsed( int lo, int hi );
        bool IsFullOver( int lo, int hi ) const;
        bool IsAllClear() const;
      private:
        HeadroomRep m_rep; // key=space, value=lo  (i.e. lo+space-1 = hi )
        int m_maxHeight;
    };

    void _FindASpot( CVRectangle & currRect ) const;
    void _UpdateHeadroom( CVRectangle const & placedRect );

    vector<Headroom> m_headroom; // YO 
    int m_packedDomainHeight;

};


class RandomRectangleGenerator
{
  public:
    RandomRectangleGenerator(
                        int nRectangles,
                        double muX, double muY,
                        double sdX, double sdY, double correlXY,
                        bool disjoint=true );
    vector<CVRectangle> const & GetRectangles() const;
    static double Urand();

  private:
    vector<CVRectangle> m_rep;
};
