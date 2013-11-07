#include "RectanglePacker.h"
#include "Trace.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>

using std::max;
using std::map;
using std::vector;
using std::pair;
using std::ostream;
using std::ofstream;
using std::string;
using std::cout;
using std::endl;

//
// For test stub, see rectpack_test.cpp.
//

//
//--------------------------- PackStats -------------------------------
//
ostream &
PackStats::Print( ostream & ost ) const
{
    ost << "Packing statistics" << endl
        << "  n =          " << nRectangles << endl
        << "  domainSize = (" << domainSize.first << ", " << domainSize.second
            << ")" << endl
        << "  domainArea = " << domainArea << endl
        << "  efficiency = " << (usedDomainArea+0.0)/domainArea << endl;

    return ost;
}

ostream & operator<<( ostream & ost, PackStats const & ps )
{
    return ps.Print( ost );
}


//
//--------------------------- CVRectangle --------------------------------
//
/** Convention is, number of cells = (1+hiX-loX)*(1+hiY-loY).
 *  This ctor used only for testing.
*/
CVRectangle::CVRectangle( Corner lo, Corner hi, int idNum )
  : m_lo(lo),
    m_hi(hi),
    id(idNum)
{
}

CVRectangle::CVRectangle( CVRectangle const & other )
  : m_lo( other.m_lo ),
    m_hi( other.m_hi ),
    id( other.id )
{
}


/** Shift so lower-left-hand corner is at (x,y) */
void
CVRectangle::ShiftTo( int x, int y )
{
    int dx = x - m_lo.first;
    int dy = y - m_lo.second;

    m_lo.first  += dx;
    m_lo.second += dy;
    m_hi.first  += dx;
    m_hi.second += dy;
}


/** Arg hsv is hue-saturation-value for the rectangle's color.
 *
 *  Arg name is a string to print in the middle of the rectangle.  This
 *  presumes that earlier in the postscript file we said something like...
 *  /Times-Roma findfont
 *  12 scalefont
 *  setfont
*/
ostream &
CVRectangle::PrintPostscript( ostream & ost, double hsv[3] ) const
{
    // Find the center of the rectangle -- that's where we'll print arg name.
    pair<int,int> center;
    center.first = (m_hi.first + m_lo.first)/2;
    center.second = (m_hi.second + m_lo.second)/2;

    ost << "newpath" << endl
        << m_lo.first << " " << m_lo.second << " " << "moveto" << endl
        << 0 << " " << (1+m_hi.second-m_lo.second) << " " << "rlineto" << endl
        << (1+m_hi.first-m_lo.first) << " " << 0 << " " << "rlineto" << endl
        << 0 << " " << (m_lo.second-m_hi.second-1) << " " << "rlineto" << endl
        << hsv[0] << " " << hsv[1] << " " << hsv[2] << " sethsbcolor" << endl
        << "closepath" << endl
        << "fill" << endl
        << "stroke" << endl
        << center.first << " " << center.second << " " << "moveto" << endl;
    if( hsv[2] > 0.5 )
    {
        ost << "0.0 setgray" << endl;
    } else
    {
        ost << "1.0 setgray" << endl;
    }
    ost << "(" << id << ") show" << endl;

    return ost;
}

ostream &
CVRectangle::PrintAscii( ostream & ost ) const
{
    ost << "(" << "[" << m_lo.first << ", " << m_lo.second << "], "
               << "[" << m_hi.first << ", " << m_hi.second << "]" << ")" << endl;
    return ost;
}


ostream & operator<<( ostream & ost, CVRectangle const & r )
{
    return r.PrintAscii( ost );
}

/** Lexicographic order on height, then width. */
bool operator<( CVRectangle const & rect1, CVRectangle const & rect2 )
{
    int height1( 1 + rect1.m_hi.second - rect1.m_lo.second );
    int height2( 1 + rect2.m_hi.second - rect2.m_lo.second );
    int width1(  1 + rect1.m_hi.first  - rect1.m_lo.first );
    int width2(  1 + rect2.m_hi.first  - rect2.m_lo.first );

    if( height1 < height2 )
    {
        return true;
    } else
    if( height1 == height2 )
    {
        if( width1 < width2 )
        {
            return true;
        } else
        {
            return false;
        }
    } else
    {
        return false;
    }
}
//
//----------------------------- RectanglePacker ------------------------
//
RectanglePacker::RectanglePacker( vector<CVRectangle> const & rects )
  : m_unpackedRectangles( rects ),
    m_calledPack( false )
{
}

RectanglePacker::~RectanglePacker()
{
}


/** Find domain -- smallest 2^n x 2^m rectangle that contains all the
 *  rectangles.
*/
void
RectanglePacker::_ComputePackedDomain()
{
    for( vector<CVRectangle>::const_iterator iter = m_packedRectangles.begin();
         iter != m_packedRectangles.end();
         ++ iter )
    {
        assert( iter->m_lo.first >= 0 );
        assert( iter->m_lo.second >= 0 );
        m_packedDomain.m_hi.first  =
            max( iter->m_hi.first, m_packedDomain.m_hi.first );
        m_packedDomain.m_hi.second =
            max( iter->m_hi.second, m_packedDomain.m_hi.second );
    }

    int n = int(ceil( log(1+m_packedDomain.m_hi.first)/log(2.0) ));
    int m = int(ceil( log(1+m_packedDomain.m_hi.second)/log(2.0) ));
    m_packedDomain.m_hi.first  = int(pow(2,n)) - 1;
    m_packedDomain.m_hi.second = int(pow(2,m)) - 1;
}


void
RectanglePacker::PackedToPostscript( string outfileName ) const
{
    _ToPostscript( outfileName, m_packedRectangles );
}
void
RectanglePacker::UnpackedToPostscript( string outfileName ) const
{
    _ToPostscript( outfileName, m_unpackedRectangles );
}

void
RectanglePacker::_ToPostscript( string outfileName,
                                vector<CVRectangle> const & rects ) const
{
    ofstream outfile( outfileName.c_str() );
    double hsv[3]; //hue, saturation, value

    outfile << "/Times-Roman findfont" << endl
            << "6 scalefont" << endl
            << "setfont" << endl;

    for( unsigned int r=0; r<rects.size(); ++r )
    {
        hsv[0] = RandomRectangleGenerator::Urand();
        hsv[1] = 0.5 + 0.5*RandomRectangleGenerator::Urand();
        hsv[2] = RandomRectangleGenerator::Urand();
        rects[r].PrintPostscript( outfile, hsv );
        outfile << endl;
    }
    outfile << "showpage" << endl;
}


void
RectanglePacker::PackedToAscii( ostream & ost ) const
{
    _ToAscii( ost, m_packedRectangles );
}
void
RectanglePacker::UnpackedToAscii( ostream & ost ) const
{
    _ToAscii( ost, m_unpackedRectangles );
}

void
RectanglePacker::_ToAscii( std::ostream & ost,
                           vector<CVRectangle> const & rects ) const
{
    for( vector<CVRectangle>::const_iterator iter = rects.begin();
         iter != rects.end();
         ++ iter )
    {
        iter->PrintAscii( ost );
        ost << endl;
    }    
}


void
RectanglePacker::Stats( PackStats & packStats ) const
{
    assert( m_calledPack );

    packStats.nRectangles = m_packedRectangles.size();
    packStats.domainSize.first =
        1 + m_packedDomain.m_hi.first - m_packedDomain.m_lo.first;
    packStats.domainSize.second =
        1 + m_packedDomain.m_hi.second - m_packedDomain.m_lo.second;
    packStats.domainArea = packStats.domainSize.first 
                         * packStats.domainSize.second;

    int sum = 0;
    for( vector<CVRectangle>::const_iterator iter = m_packedRectangles.begin();
         iter != m_packedRectangles.end();
         ++ iter )
    {
        sum += (1 + iter->m_hi.first - iter->m_lo.first)
             * (1 + iter->m_hi.second - iter->m_lo.second);
    }
    packStats.usedDomainArea = sum;

}


/* Retrieve the packed location of the rectangle with the indicated ID.
 * ID numbers are assigned to rectangles at construction.
*/
CVRectangle
RectanglePacker::GetPackedRectangle( int id ) const
{
    // Build m_keyToPackedRectangles first time we get here.
    // key = id, value = rectangle's position in the m_packedRectangles vector.
    if( m_keyToPackedRectangles.empty() )
    {
        for( int i=0; i < int(m_packedRectangles.size()); ++i )
        {
            CVRectangle const & rect( m_packedRectangles[i] );
            m_keyToPackedRectangles.insert( pair<int,int>( rect.id, i ) );
        }
    }

    return m_packedRectangles[ m_keyToPackedRectangles[id] ];
}

/* Retrieve the unpacked, i.e. original, location of the rectangle with the
   indicated ID.
 * ID numbers are assigned to rectangles at construction.
*/
CVRectangle
RectanglePacker::GetUnpackedRectangle( int id ) const
{
    // Build m_keyToUnpackedRectangles first time we get here.
    // key = id, value = rectangle's position in the m_packedRectangles vector.
    if( m_keyToUnpackedRectangles.empty() )
    {
        for( int i=0; i < int(m_packedRectangles.size()); ++i )
        {
            CVRectangle const & rect( m_packedRectangles[i] );
            m_keyToUnpackedRectangles.insert( pair<int,int>( rect.id, i ) );
        }
    }

    return m_packedRectangles[ m_keyToUnpackedRectangles[id] ];
}


CVRectangle
RectanglePacker::GetPackedDomain() const
{
    return m_packedDomain;
}


//
//-------------------- NullRectanglePacker ----------------
//
NullRectanglePacker::NullRectanglePacker( vector<CVRectangle> const & rects )
  : RectanglePacker( rects )
{
}

NullRectanglePacker::~NullRectanglePacker()
{
}


/** Packed domain's lower LH corner is (0,0). */
void
NullRectanglePacker::Pack()
{
    m_packedRectangles = m_unpackedRectangles;

    _ComputePackedDomain();

    m_calledPack = true;
}


//
//-------------------- OKRectanglePacker ----------------
//
OKRectanglePacker::OKRectanglePacker( vector<CVRectangle> const & rects )
  : RectanglePacker( rects )
{
}

OKRectanglePacker::~OKRectanglePacker()
{
}


/** Find a compact 2^n x 2^m packing of all the rectangles of
 *  m_unpackedRectangles.
 *
 *  To find the packed location of a rectangle, indexed by its position in the
 *  unpacked list, call this->GetPackedRectangle().
 *
 *  The only role of m_packedDomain is so we can conveniently produce a display.
 *  Packed domain's lower LH corner is (0,0). 
*/
void
OKRectanglePacker::Pack()
{
    Trace t("OKRectanglePacker::Pack()");

    m_packedRectangles.reserve( m_unpackedRectangles.size() );
//  m_headroom.reserve( 16392 ); // Doesn't affect running time even on rings10.

    // Sort the rectangles lexicographically by height, then by width, copying
    // them to a temporary storage.
    // Actually, there's a pretty small gain to the sorting.  Sorted or not,
    // the dominant source of wasted space is the requirement that the bounding
    // rectangle be an even 2^m x 2^n.
    // Not sorting has the advantage of easier debugging.
    vector<CVRectangle> sortedUnpackedRectangles( m_unpackedRectangles );
    sort( sortedUnpackedRectangles.begin(), sortedUnpackedRectangles.end() );
    reverse( sortedUnpackedRectangles.begin(), sortedUnpackedRectangles.end() );

    //
    // Find m_packedDomainHeight -- least 2^n greater than tallest rectangle.
    //
    int maxRectHeight=0;
    int totalVolume=0;
    for( unsigned int r=0; r<sortedUnpackedRectangles.size(); ++r )
    {
        CVRectangle const & rect( sortedUnpackedRectangles[r] );
        maxRectHeight = max( maxRectHeight,
                             1 + rect.m_hi.second - rect.m_lo.second );
        totalVolume += (1+rect.m_hi.first-rect.m_lo.first) 
                     * (1+rect.m_hi.second-rect.m_lo.second);
    }
    m_packedDomainHeight =
        int(pow(2, int(ceil(log(max(maxRectHeight,
                                    int(pow(totalVolume,0.5))))/log(2)))));

    //
    // Now pack 'em in, one by one.
    //
    Headroom initialHeadroom( m_packedDomainHeight );
    m_headroom.push_back( initialHeadroom );
    for( unsigned int r=0; r<sortedUnpackedRectangles.size(); ++r )
    {
        //t.Info( "Working on rectangle # %d", r );
        CVRectangle rect( sortedUnpackedRectangles[r] );
        _FindASpot( rect );
        //t.Info( "...placed it at (%d,%d)", rect.m_lo.first,rect.m_lo.second );
        m_packedRectangles.push_back( rect );
        _UpdateHeadroom( rect );
    }        

    _ComputePackedDomain();

    m_calledPack = true;
}


/** Finds a spot to pack currRect.
 *  Modifies currRect: shifts it so its lower-left-hand corner is at the spot
 *    we want to pack it at.
*/
void
OKRectanglePacker::_FindASpot( CVRectangle & rect ) const
{
    // Go left until you hit a column with insufficient headroom.  And then,
    // put rect in the column just to its right.
    // We're guaranteed to have enough room at least at the point we start,
    // because we make sure m_headroom's right-most element is completely free.
    int rectHeight( 1 + rect.m_hi.second - rect.m_lo.second );
    int newRectLoY;
    int i=m_headroom.size() - 1;
    while( (i >= 0)
    &&     (-1 != m_headroom[i].FindRoom( rectHeight )))
    {
        --i;
    }

    newRectLoY = m_headroom[i+1].FindRoom( rectHeight );
    rect.ShiftTo( i+1, newRectLoY );
}


/** Arg placedRect has already been shifted so its lower-left-hand corner is
 *  where it's actually going to be packed.
 *
 *  Each Headroom object does its own availability updating.
 *
 *  Grow m_headroom so that its right-most element is totally empty of
 *  rectangles.
*/
void
OKRectanglePacker::_UpdateHeadroom( CVRectangle const & placedRect )
{
    // First backfill; mark as used even unused cells to the left of placedRect.
    // This is so _FindASpot() isn't tempted to use one of those spaces; it may
    // be tall enough, but it won't be wide enough for what we'll want to put
    // inside it.
    int i = placedRect.m_lo.first - 1;
    while( (i>=0)
    &&     (! m_headroom[i].IsFullOver( placedRect.m_lo.second,
                                        placedRect.m_hi.second )))
    {
        m_headroom[i].MarkUsed( placedRect.m_lo.second, placedRect.m_hi.second );
        -- i;
    }

    // Mark as used those cells now actually occupied by placedRect.
    for( int i=placedRect.m_lo.first; i<=placedRect.m_hi.first; ++i )
    {
        if( i >= int(m_headroom.size()) )
        {
            m_headroom.push_back( Headroom( m_packedDomainHeight ) );
        }
        m_headroom[i].MarkUsed( placedRect.m_lo.second, placedRect.m_hi.second );
    }

    // Make sure last element of m_headroom is completely empty of rectangles.
    if( ! m_headroom[m_headroom.size()-1].IsAllClear() )
    {
        m_headroom.push_back( Headroom( m_packedDomainHeight ) );
    }
}


/** It's assumed that the floor is zero. */
OKRectanglePacker::Headroom::Headroom( int maxHeight )
  : m_maxHeight( maxHeight )
{
    m_rep.insert( pair<int,int>( maxHeight, 0 ) );
}


/** Find a gap that will fit something of arg height.
 *  Return the y coord of the bottom of that gap, or if there's no room at all,
 *  return -1.
*/
int
OKRectanglePacker::Headroom::FindRoom( int height ) const
{
    HeadroomRep::const_iterator iter = m_rep.begin();
    while( (iter != m_rep.end()) && (iter->first < height) )
    {
        ++ iter;
    }
    if( iter == m_rep.end() )  // failure
    {
        return -1;
    } else                     // success
    {
        return iter->second;
    }
}


/** Return true iff all the space is unused. */
bool
OKRectanglePacker::Headroom::IsAllClear() const
{
    if( m_rep.count( m_maxHeight ) == 1 )
    {
        return true;
    } else
    {
        return false;
    }
}


/** Return true if there's no empty space within [lo,hi]. */
bool
OKRectanglePacker::Headroom::IsFullOver( int lo, int hi ) const
{
    for( HeadroomRep::const_iterator iter = m_rep.begin();
         iter != m_rep.end();
         ++ iter )
    {
        int gapBottom = iter->second;
        int gapTop    = iter->second + iter->first - 1;
        if( (gapBottom <= hi) && (gapTop >= lo ) )
        {
            return false;
        }
    }
    return true;
}


/** Mark as used all the cells from lo through m_hi.
 *  Called only if there actually are cells to mark as used, i.e. the placed
 *    rectangle intersects the column corresponding to this Headroom object.
*/
void
OKRectanglePacker::Headroom::MarkUsed( int lo, int hi )
{
    HeadroomRep::iterator iter = m_rep.begin();
    assert( iter != m_rep.end() );

    // Gotta go through all the elements of the map; if we're backfilling, there
    // could be several gaps that intersect [lo,hi].
    while( iter != m_rep.end() )
    {
        int gapLo = iter->second;
        int gapHi = iter->second + iter->first - 1;

        if( (gapLo <= hi) && (gapHi >= lo) ) // Else there's no overlap.
        {
            if( gapLo < lo )
            {
                if( gapHi <= hi )
                {   // Replace with one smaller gap.
                    int newGap = lo - gapLo;
                    m_rep.insert( pair<int,int>( newGap, gapLo ) );
                } else
                {   // Replace with two smaller gaps.
                    int newGap1 = lo - gapLo;
                    m_rep.insert( pair<int,int>( newGap1, gapLo ) );
                    int newGap2 = gapHi - hi;
                    m_rep.insert( pair<int,int>( newGap2, hi+1 ) );
                }
            } else // gapLo >= lo
            {
                if( gapHi <= hi )
                {    // Create no new gaps; we'll just erase this whole gap.
                } else
                {    // Create a smaller gap at the top.
                    int newGap = gapHi - hi;
                    m_rep.insert( pair<int,int>( newGap, hi+1 ) );
                }
            }

            HeadroomRep::iterator eraseme = iter;
            ++ iter;
            m_rep.erase( eraseme );
        } else
        {
            ++ iter;
        }
    }
}


//
//--------------------- RandomRectangleGenerator ----------------------
//
/** X and Y are rectangle width and height.  muX, muY are expected values,
 *  sdX, sdY are standard errors, correlXY is the correlation coefficient.
*/
RandomRectangleGenerator::RandomRectangleGenerator(
    int nRectangles,
    double muX, double muY,
    double sdX, double sdY, double correlXY,
    bool disjoint/*=true*/ )
{
    // FIXME: We're not doing the right thing with those moments; we'd need to
    // do an eigenvector decomposition.
    m_rep.reserve( nRectangles );
    double const sqrt12 = pow(12.0, 0.5);
    int xPos(0), yPos(0), xSize, ySize;

    for( int i=0; i<nRectangles; ++i )
    {
//      m_rep.push_back( CVRectangle( 20*i, 20*i, 20*i+10, 20*i+10 ) );
        double u1 = Urand();
        double u2 = Urand();
        xSize = int(muX + sqrt12*sdX*u1 + correlXY*sdX*sdY*u1*u2) + 1;
        ySize = int(muY + sqrt12*sdY*u2 + correlXY*sdX*sdY*u1*u2) + 1;


        if( disjoint == true )
        {
            // Places rectangles along the bottom line.
            m_rep.push_back(CVRectangle( Corner(xPos, 0),
                                       Corner( xPos + xSize,
                                               int(muY + sqrt12*sdY*u2 +
                                                 correlXY*sdX*sdY*u1*u2) + 1),
                                       i ));
            xPos += xSize;
            yPos += ySize;
        } else
        {
            // Places rectangles at random locations.
            xPos = int(muX + sqrt12*sdX*u1);
            yPos = int(muY + sqrt12*sdY*u2);
            m_rep.push_back(CVRectangle( Corner(xPos*2, yPos*2),
                                       Corner(xPos*2 + xSize, yPos*2 + ySize),
                                       i ));
        }

    }
}

std::vector<CVRectangle> const & 
RandomRectangleGenerator::GetRectangles() const
{
    return m_rep;
}


/** Returns a uniform[0,1] random variate. */
/*static*/double
RandomRectangleGenerator::Urand()
{
    return rand()/(RAND_MAX+0.0);
}
