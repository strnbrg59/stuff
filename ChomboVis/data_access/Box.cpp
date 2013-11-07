#ifndef INCLUDED_BOX_CPP
#define INCLUDED_BOX_CPP

#include "../utils/Trace.h"
#include <cassert>
#include <fstream>
#include "Box.h"

using std::vector;
using std::ostream;

hid_t Box::s_HDF5Datatype;

/** Tell HDF5 what it needs to know in order to load compound data from disk to
 *  memory.  Notice we're not registering the Box class, but rather the
 *  BoxSimple class.  This is to give us freedom to put whatever we want into
 *  class Box, without worrying about its compatibility with the offsetof
 *  (i.e. HOFFSET) macro.
*/
/*static*/ void
Box::InitializeHDF5Datatype( int dimensionality )
{
    s_HDF5Datatype = H5Tcreate( H5T_COMPOUND, sizeof( BoxSimple) );
    char loCornerComponentNames[3][5] = {"lo_i", "lo_j", "lo_k"};
    char hiCornerComponentNames[3][5] = {"hi_i", "hi_j", "hi_k"};

    BoxSimple bs;
    int offset = (char *)(&(bs.data))- (char *)&bs;

    for( int i=0; i<dimensionality; ++i )
    {
        H5Tinsert( s_HDF5Datatype, loCornerComponentNames[i],
                   offset + i*sizeof(int),
                   H5T_NATIVE_INT );
    }
    for( int i=0; i<dimensionality; ++i )
    {
        H5Tinsert( s_HDF5Datatype, hiCornerComponentNames[i],
                   offset + (THREE+i) * sizeof(int),
                   H5T_NATIVE_INT );
    }
}
/*static*/ void
Box::ReleaseHDF5Datatype()
{
    H5Tclose( s_HDF5Datatype );
}


BoxSimple::BoxSimple( Box const & box )
{
    for( int i=0;i<THREE;++i )
    {
        data[i] = box.GetLoCorner( i );
        data[THREE+i] = box.GetLoCorner( THREE+i );
    }
}


/**  We use this function to construct from the hyperslab read in as dataset
 *  "boxes".
*/
/* explicit */
Box::Box( BoxSimple const & boxSimple, int idNum )
{
    m_loCorner = Intvect( boxSimple.data );
    m_hiCorner = Intvect( boxSimple.data + THREE );
    m_idNum = idNum;
    m_realIdNum = m_idNum;
}


/** Construct a Box that's a slice of the one indicated in arg other.
 *  Regardless of arg axis, the resulting slice box's "thin" dimension is always
 *  z.
*/
Box::Box( Box const & other, char axis )
  : m_idNum( other.m_idNum ),
    m_realIdNum( other.m_realIdNum )
{
    Trace t("Box::Box( Box const &, char axis)");
    m_loCorner.m_data[2] = m_hiCorner.m_data[2] = 0;
    switch( axis )
    {
        case 'x':
            m_loCorner.m_data[0] = other.m_loCorner.m_data[1];
            m_loCorner.m_data[1] = other.m_loCorner.m_data[2];
            m_hiCorner.m_data[0] = other.m_hiCorner.m_data[1];
            m_hiCorner.m_data[1] = other.m_hiCorner.m_data[2];
            break;
        case 'y':
            m_loCorner.m_data[0] = other.m_loCorner.m_data[2];
            m_loCorner.m_data[1] = other.m_loCorner.m_data[0];
            m_hiCorner.m_data[0] = other.m_hiCorner.m_data[2];
            m_hiCorner.m_data[1] = other.m_hiCorner.m_data[0];
            break;
        case 'z':
            m_loCorner.m_data[0] = other.m_loCorner.m_data[0];
            m_loCorner.m_data[1] = other.m_loCorner.m_data[1];
            m_hiCorner.m_data[0] = other.m_hiCorner.m_data[0];
            m_hiCorner.m_data[1] = other.m_hiCorner.m_data[1];
            break;
        default:
            t.FatalError( "Illegal arg axis: %c", axis );
    }
}


/** Ordinary copy constructor. */
Box::Box( Box const & that )
  : m_loCorner( that.m_loCorner ),
    m_hiCorner( that.m_hiCorner ),
    m_idNum( that.m_idNum ),
    m_realIdNum( that.m_realIdNum )
{
}


Box::Box( int corners[6] )
{
    m_loCorner = Intvect(corners);
    m_hiCorner = Intvect(corners+3);
}


/** Grow the box by the corresponding coordinates of arg iv.  If
 *  atBothEnds==true, then grow the box's lo and hi corners; this is appropriate
 *  if we're growing to accomodate ghost cells.  If atBothEnds==false, grow
 *  at the hi corner only; this is appropriate for adapting to the hdf5
 *  convention for non-cell-centered data.
*/
void
Box::Grow( Intvect const & iv, bool atBothEnds )
{
    if( atBothEnds == true )
    {
        m_loCorner -= iv;
    }
    m_hiCorner += iv;
}

/** Shrink the box.  See Box::Grow() for more details. */
void
Box::Shrink( Intvect const & iv, bool atBothEnds )
{
    if( atBothEnds == true )
    {
        m_loCorner += iv;
    }
    m_hiCorner -= iv;
}


Box &
Box::operator=( Box const & that )
{
    m_loCorner = that.m_loCorner;
    m_hiCorner = that.m_hiCorner;
    m_idNum = that.m_idNum;
    m_realIdNum = that.m_realIdNum;

    return *this;
}    


/** Multiply the coordinates of the Box's corners by 1/refinementFactor, so they
 *  correspond to the scale of Boxes one level coarser.
 *  We truncate, rather than round, fractions.  This is so, on the high corners,
 *  we don't overshoot to the next coarse box.
*/
void
Box::operator*=( double x )
{
    for( int i=0;i<THREE;++i )
    {
        m_loCorner.m_data[i] = int( m_loCorner.m_data[i] * x + 1E-10 );
        m_hiCorner.m_data[i] = int( m_hiCorner.m_data[i] * x + 1E-10 );
        // The 1E-10 is to ensure we don't end up with, e.g., 5 if
        // the floating point result should be 5.0 but ends up represented
        // internally as 4.9999999999.  But we *don't* want to
        // round to the nearest integer; we do want to round down -- just not
        // down by two integers!
    }
}

void
Box::operator*=( double x[THREE] )
{
    for( int i=0;i<THREE;++i )
    {
        m_loCorner.m_data[i] = int( m_loCorner.m_data[i] * x[i] + 1E-10 );
        m_hiCorner.m_data[i] = int( m_hiCorner.m_data[i] * x[i] + 1E-10 );
    }
}


/** Truncates, doesn't round. See operator*=() comments. */
Box
Box::operator*( double x ) const
{
    Box result( *this );
    result *= x;
    return result;
}

/** Truncates, doesn't round. See operator*=() comments. */
Box
Box::operator*( double x[THREE] ) const
{
    Box result( *this );
    result *= x;
    return result;
}


/** Grow this box so it includes arg that, which lies to the right, above or
 *  behind.
*/
void
Box::operator+=( Box const & that )
{
    m_hiCorner = that.m_hiCorner;
}


/** Checks if *this and that are adjacent and would form a rectangular prism if
 *  combined with *this to the left, below, or in front of that.
 *  Note that to really check if the boxes can be combined, you need to call
 *  this function twice -- once on one Box, once on the other.
 *
 *  If the two Boxes can't be joined, returns false.
 *
 *  See also operator+=.
*/ 
bool
Box::CanCombineWith( Box const & that ) const
{
    if( (Intvect(GetHiCorner(0)+1,GetLoCorner(1),GetLoCorner(2)) ==
         that.GetLoCorner())
    &&  (GetHiCorner() ==
         Intvect(that.GetLoCorner(0)-1,that.GetHiCorner(1),that.GetHiCorner(2)))
    )
    {
        return true;
    }

    if( (Intvect(GetLoCorner(0),GetHiCorner(1)+1,GetLoCorner(2)) ==
         that.GetLoCorner())
    &&  (GetHiCorner() ==
         Intvect(that.GetHiCorner(0),that.GetLoCorner(1)-1,that.GetHiCorner(2)))
    )
    {
        return true;
    }

    if( (Intvect(GetLoCorner(0),GetLoCorner(1),GetHiCorner(2)+1) ==
         that.GetLoCorner())
    &&  (GetHiCorner() ==
         Intvect(GetHiCorner(0),GetHiCorner(1),GetLoCorner(2)-1)))
    {
        return true;
    }

    // If got here, then *this and that can't be combined.
    return false;
}


/** Tests for equality of the corners. */
bool
Box::operator==( Box const & that ) const
{
    if( (m_loCorner == that.m_loCorner)
    &&  (m_hiCorner == that.m_hiCorner) )
    {
        return true;
    } else
    {
        return false;
    }
}

bool
Box::operator!=( Box const & that ) const
{
    return ! operator==(that);
}


/** Returns the intersection of this box and another one.
 *  Arg status is set to STATUS_OK if the intersection is nonempty, or
 *  STATUS_EMPTY otherwise.  Arg status should be a pointer to a Status object
 *  preallocated by the client.
*/
Box
Box::Intersect( Box const & that, Status * status ) const
{
    Box result;
    *status = STATUS_OK;
    for( int i=0;i<3;++i )
    {
        result.m_loCorner[i] = std::max(m_loCorner[i], that.m_loCorner[i] );
        result.m_hiCorner[i] = std::min(m_hiCorner[i], that.m_hiCorner[i] );
        if( result.m_loCorner[i] > result.m_hiCorner[i] )
        {
            *status = STATUS_EMPTY;
        }
    }
    return result;
}


/** Return a collection of disjoint Boxes (rectangular prisms) that cover the
 *  set difference of *this and arg that.
*/
vector<Box>
Box::operator-( Box const & that ) const
{
//  cout << (*this) << " minus " << that << ":" << endl;

    Intvect c0( m_loCorner),
            c1( m_hiCorner.i(), m_loCorner.j(), m_loCorner.k() ),
            c2( m_hiCorner.i(), m_hiCorner.j(), m_loCorner.k() ),
            c3( m_loCorner.i(), m_hiCorner.j(), m_loCorner.k() ),
            c4( m_loCorner.i(), m_loCorner.j(), m_hiCorner.k() ),
            c5( m_hiCorner.i(), m_loCorner.j(), m_hiCorner.k() ),
            c6( m_hiCorner ),
            c7( m_loCorner.i(), m_hiCorner.j(), m_hiCorner.k() ),
            f0( that.m_loCorner),
            f1( that.m_hiCorner.i(), that.m_loCorner.j(), that.m_loCorner.k() ),
            f2( that.m_hiCorner.i(), that.m_hiCorner.j(), that.m_loCorner.k() ),
            f3( that.m_loCorner.i(), that.m_hiCorner.j(), that.m_loCorner.k() ),
            f4( that.m_loCorner.i(), that.m_loCorner.j(), that.m_hiCorner.k() ),
            f5( that.m_hiCorner.i(), that.m_loCorner.j(), that.m_hiCorner.k() ),
            f6( that.m_hiCorner ),
            f7( that.m_loCorner.i(), that.m_hiCorner.j(), that.m_hiCorner.k() );

    vector<Box> result;
    Box b0,b1, // front & back subboxes
        b2,b3, // left & right
        b4,b5; // bottom & top

    // If *this and *that disjoint, difference is *this.
    if( (f2.j() < c1.j()) || (f2.i() < c0.i()) || (f4.k() < c0.k())
    ||  (f0.j() > c3.j()) || (f0.i() > c1.i()) || (f0.k() > c4.k()) )
    {
        result.push_back( *this );
    } else
    {
        bool existsBox[6] = {false,false,false,false,false,false};

        // b0
        if( c4.k() > f4.k() )
        {
            existsBox[0] = true;
            b0.m_loCorner = Intvect(c4,c4,f4) + Intvect(0,0,1);
            b0.m_hiCorner = c6;
            // The "+Intvect(1,0,0)" is to make sure b0's hiCorner is inside
            // b0, rather than one cell beyond it.
            result.push_back( b0 );
//          cout << "b0=" << b0 << endl;
        }

        // b1
        if( f0.k() > c0.k() )
        {
            existsBox[1] = true;
            b1.m_loCorner = c0;
            b1.m_hiCorner = Intvect(c2,c2,f2) - Intvect(0,0,1);
            result.push_back( b1 );
//          cout << "b1=" << b1 << endl;
        } 

        // b2
        if( f0.i() > c0.i() )
        {
            existsBox[2] = true;

            if( existsBox[1] )
            {
                b2.m_loCorner = Intvect(c0,c0,f0);
            } else
            {
                b2.m_loCorner = c0;
            }

            if( existsBox[0] )
            {
                b2.m_hiCorner = Intvect(f7,c7,f7) - Intvect(1,0,0);
            } else
            {
                b2.m_hiCorner = Intvect(f7,c7,c7) - Intvect(1,0,0);
            }
            result.push_back( b2 );
//          cout << "b2=" << b2 << endl;
        }

        // b3
        if( c1.i() > f1.i() )
        {
            existsBox[3] = true;
            
            if( existsBox[1] )
            {
                b3.m_loCorner = Intvect(f1,c1,f1) + Intvect(1,0,0);
            } else
            {
                b3.m_loCorner = Intvect(f1,c1,c1) + Intvect(1,0,0);
            }

            if( existsBox[0] )
            {
                b3.m_hiCorner = Intvect(c6,c6,f6);
            } else
            {
                b3.m_hiCorner = c6;
            }

            result.push_back( b3 );
//          cout << "b3=" << b3 << endl;
        }

        // b4
        if( f0.j() > c0.j() )
        {
            existsBox[4] = true;

            if( existsBox[1] && existsBox[2] )
            {
                b4.m_loCorner = Intvect(f0,c0,f0);
            } else
            if( existsBox[1] )
            {
                b4.m_loCorner = Intvect(c0,c0,f0);
            } else
            if( existsBox[2] )
            {
                b4.m_loCorner = Intvect(f0,c0,c0);
            } else
            {
                b4.m_loCorner = c0;
            }

            if( existsBox[0] && existsBox[3] )
            {
                b4.m_hiCorner = f5;
            } else
            if( existsBox[0] )
            {
                b4.m_hiCorner = Intvect(c5,f5,f5);
            } else
            if( existsBox[3] )
            {
                b4.m_hiCorner = Intvect(f5,f5,c5);
            } else
            {
                b4.m_hiCorner = Intvect(c5,f5,c5);
            }
            b4.m_hiCorner -= Intvect(0,1,0);

            result.push_back( b4 );
//          cout << "b4=" << b4 << endl;
        }

        // b5
        if( c2.j() > f2.j() )
        {
            existsBox[5] = true;

            if( existsBox[1] && existsBox[2] )
            {
                b5.m_loCorner = f3;
            } else
            if( existsBox[1] )
            {
                b5.m_loCorner = Intvect(c3,f3,f3);
            } else
            if( existsBox[2] )
            {
                b5.m_loCorner = Intvect(f3,f3,c3);
            } else
            {
                b5.m_loCorner = Intvect(c3,f3,c3);
            }
            b5.m_loCorner += Intvect(0,1,0);

            if( existsBox[0] && existsBox[3] )
            {
                b5.m_hiCorner = Intvect(f6,c6,f6);
            } else
            if( existsBox[0] )
            {
                b5.m_hiCorner = Intvect(c6,c6,f6);
            } else
            if( existsBox[3] )
            {
                b5.m_hiCorner = Intvect(f6,c6,c6);
            } else
            {
                b5.m_hiCorner = c6;
            }

            result.push_back( b5 );
//          cout << "b5=" << b5 << endl;
        }
    }

    for( unsigned int i=0;i<result.size();++i )
    {
        result[i].m_realIdNum = m_realIdNum;
        // We'll assign m_idNum's after we've assembled all the subboxes.
    }

    return result;
}


/** Return the total number of cells -- x size times y size times z size. */
int
Box::GetNumCells() const
{
    int result = 1;
    for( int i=0; i<THREE; ++i )
    {
        result *= 1 + m_hiCorner.m_data[i] - m_loCorner.m_data[i];
    }
    return result;
}


/** Fill arg result with the size, in cells, of the indicated box along each
 *  of its three dimensions.
 *
 *  Client is responsible for allocating arg result (as an int[3]).
*/
void
Box::GetDims( int * result ) const
{
    for( int i=0; i<THREE; ++i )
    {
        result[i] = 1 + GetHiCorner(i) - GetLoCorner(i);
    }
}


bool
Box::ContainsCell( int cell[3] ) const
{
    for( int i=0;i<3;++i )
    {
        if( (cell[i] < m_loCorner[i]) || (cell[i] > m_hiCorner[i]) )
        {
            return false;
        }
    }
    return true;
}


/** Return true if the indicated position on the indicated axis lies within this
 *  box.
*/
bool
Box::ContainsPlane( char axis, double axisPosition,
                    Triple<double> dx, Triple<double> origin ) const
{
    Trace t("Box::ContainsPlane()");
    assert( (axis=='x') || (axis=='y') || (axis=='z') );
    int axisNum( axis - 'x' );

    double loCornerXYZ = GetLoCorner( axisNum ) * dx[axisNum]
                         + origin[axisNum];
    double hiCornerXYZ = (GetHiCorner( axisNum ) + 1 ) * dx[axisNum]
                         + origin[axisNum];
    if( (axisPosition >= loCornerXYZ ) && (axisPosition <= hiCornerXYZ ) )
    {
        return true;
    } else
    {
        return false;
    }
}


/** Converts lo and hi corner to what they would be if dx were x times smaller
 *  Think of arg x is the inverse of a refinement ratio.
*/
void
Box::Scale( int r )
{
    Intvect newLoCorner( m_loCorner );
    for( int dim=0;dim<3;++dim )
    {
        newLoCorner[dim] *= r;
        m_hiCorner[dim] = newLoCorner[dim]
                        + (1+m_hiCorner[dim]-m_loCorner[dim])*r - 1;
        m_loCorner = newLoCorner;
    }
}


/*static*/ Box
Box::WackyBox()
{
    int const unlikely( 999 );
    int corners[6] = { unlikely, -unlikely, unlikely, -unlikely,
                       unlikely, -unlikely };
    return Box( corners );
}

#endif // INCLUDED_BOX_CPP
