#include "astro.hpp"
#include "cmdline.hpp"
#include "myregexp.hpp"
#include "trace.hpp"

#include <algorithm>
#include <fstream>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>
using std::cerr;
using std::endl;
#include <unistd.h>

class Cmdline;

Triangle::Triangle( Sample const & sample, Cmdline const & cmdline )
  : _cmdline( cmdline ),
    _sample( &sample ),
    _i(0), _j(1), _k(2),  // indices into _sample.
    _no_more( false )
{
    _rep.push_back( (*_sample)[0] );
    _rep.push_back( (*_sample)[1] );
    _rep.push_back( (*_sample)[2] );
}
 

Triangle::Triangle( Triangle const & that )
  : _cmdline( that._cmdline ),
    _rep( that._rep ),
    _sample( that._sample ),
    _i(that._i), _j(that._j), _k(that._k),
    _no_more( that._no_more )
{
}

Triangle &
Triangle::operator=( Triangle const & that )
{
    _rep = that._rep;
    _sample = that._sample;
    _i = that._i;
    _j = that._j;
    _k = that._k;
    _no_more = that._no_more;
    return *this;
}


/** Reset triangle stars to first three in Sample of which the brightest's
 *  TotalEnergy() does not exceed arg bright_limit.
*/
void
Triangle::Reset( double bright_limit /*=1E123*/ )
{
    Trace t("Triangle::Reset()");
    Star dummy_star( bright_limit, Point<double>(0,0) );
    vector<Star>::const_iterator iter = std::lower_bound(
        _sample->GetStars().begin(), _sample->GetStars().end(),
        dummy_star, CompareStarEnergy );
    if( (iter == _sample->GetStars().end())
    ||  (iter -  _sample->GetStars().begin() + 3 > _sample->GetStars().size()) )
    {
        _no_more = true;
    } else
    {
        _i = iter - _sample->GetStars().begin();
        _j = _i+1;
        _k = _j+1;
        _rep[0] = (*_sample)[_i];
        _rep[1] = (*_sample)[_j];
        _rep[2] = (*_sample)[_k];
        _no_more = false;
    }
}


/** Load *this up with the next triangle of stars from _sample.
 *  Set _no_more to true when we've exhausted all the possible triangles
 *  in _sample.
 *
 *  Assumes stars in _sample are sorted in decreasing order of energy.
 *  Don't advance beyond a star whose TotalEnergy() falls below dim_limit.
*/
void
Triangle::Advance( double dim_limit/*=0.0*/ )
{
    Trace t("Triangle::Advance()");

    unsigned size = _sample->Size();

    if( (_k < size-1) && ( (*_sample)[_k+1].TotalEnergy() >= dim_limit) )
    {
        ++ _k;
        _rep[2] = (*_sample)[_k];
        _no_more = false;
    } else
    {
        if( (_j < size-2) && ( (*_sample)[_j+1].TotalEnergy() >= dim_limit) )
        {
            ++ _j;
            _k = _j+1;
            _rep[1] = (*_sample)[_j];
            _rep[2] = (*_sample)[_k];
            _no_more = false;
        } else
        {
            if( (_i < size-3) && ((*_sample)[_i+1].TotalEnergy() >= dim_limit))
            {
                ++ _i;
                _j = _i+1;
                _k = _j+1;
                _rep[0] = (*_sample)[_i];
                _rep[1] = (*_sample)[_j];
                _rep[2] = (*_sample)[_k];
                _no_more = false;
            } else
            {
                _no_more = true;
            }
        }
    }
}


/** Advance, but never consider stars more than arg bandwidth places apart in
 *  the vector of sample stars.
 *  A small bandwidth number greatly reduces the number of triangles we search,
 *  thus making practical fairly large samples, at negligible risk of causing
 *  us to miss finding any matching triangles (if there are any to be found).
*/
void Triangle::InchForward( int bandwidth )
{
    assert( bandwidth >= 3 );  // Need at least that much room for 3 stars!

    assert( !_no_more );
    // Advance() might set _no_more inside this function, but if that happens,
    // we check if it means we've really reached the end of the sample stars or
    // we've simply reached the boundary of the bandwidth.  If the latter, we
    // should have slid the band forward and unset _no_more.

    double dim_limit;
    if( unsigned(_i + bandwidth) < _sample->Size() )
    {
        dim_limit = (*_sample)[_i + bandwidth].TotalEnergy();
    } else
    {
        dim_limit = 0.0;
    }

    Advance( dim_limit );

    if( _no_more )
    {
        if( _i < _sample->Size()-3-1 )
        {
            Reset( (*_sample)[_i+1].TotalEnergy() );
            _no_more = false;
        }
        return;
    }
}



/** Return the three angles in the triangle of stars. */
vector<double>
Triangle::Angles() const
{
    Trace t("Triangle::Angles()");

    vector<double> result;
    result.resize(3);
    double origin1, origin2, x1, y1, x2, y2, dot_product, norm;

    for( int vertex=0; vertex<2; vertex++ )
    {
        origin1 = _rep[vertex].x();
        origin2 = _rep[vertex].y();
        x1 = _rep[vertex+1].x() - origin1;
        y1 = _rep[vertex+1].y() - origin2;
        x2 = _rep[(vertex+2) % 3].x() - origin1;
        y2 = _rep[(vertex+2) % 3].y() - origin2;
        dot_product = x1*x2 + y1*y2;
        norm = pow( (x1*x1 + y1*y1) * (x2*x2 + y2*y2 ), 0.5 );
        result[vertex] = acos( dot_product/norm );
    }
    result[2] = M_PI - result[0] - result[1];

    return result;
}


/** Return the sum of the lengths of the sides. */
double
Triangle::Perimeter() const
{
    double result(0.0);
    for( int i=0;i<3;++i )
    {
        result += pow(
            pow( _rep[i%3].x() - _rep[(i+1)%3].x(), 2 )
          + pow( _rep[i%3].y() - _rep[(i+1)%3].y(), 2 ),
                      0.5 );
    }
    return result;
}


/** Offset is on arg that. */
int
Triangle::GetMatchOffset( Triangle const & that ) const
{
    int offset;
    DistanceTo( that, &offset );
    return offset;
}


bool
Triangle::Matches( Triangle const & that ) const
{
    int offset;
    if( DistanceTo( that, &offset ) < _cmdline.TriangleTolerance() )
    {
        return true;
    } else
    {
        return false;
    }
}


/** Return sum of squares of differences (in radians) between angles, plus
 *  1E-6*sum of squares of differences between star energy ratios.
 *  Set arg *offset to 0, 1 or 2 according to how the angles of the two
 *  triangles correspond to one another (and the offsetting is on arg that).
*/
double
Triangle::DistanceTo( Triangle const & that, int * offset ) const
{
    Trace t("Triangle::DistanceTo()");

    vector<double>this_angles(  Angles() );
    vector<double>that_angles( that.Angles() );
    double result;

    // There are three ways for the two sets of angles to line up.  (Not six;
    // we exclude flip transformations!)  So we'll measure the distance in each
    // of the three ways and return the minimum.
    vector<double> distances(3);
    for( int i=0;i<3;i++ )          // i indexes the offset.
    {
        distances[i] = 0;
        for( int j=0;j<3;j++ )
        {
            distances[i] += pow( this_angles[j] - that_angles[(j+i)%3],
                                 2 );
            if( this_angles[j] < 0.24 ) // 0.24 radians = 15 degrees.
            {  // Hard to align on extremely small angles: reject such triangles
                distances[i] += 100;
            }
        }
    }
    vector<double>::const_iterator min_dist =
        min_element( distances.begin(), distances.end() );

    if( min_dist == distances.begin() )
    {
        *offset = 0;
    } else
    {
        if( min_dist == distances.begin() + 1 )
        {
            *offset = 1;
        } else
        {
            *offset = 2;
        }
    }


    // Exclude pairs of triangles with two (or three!) identical angles.
    // We could in principle work with isosceles triangles, but it would be
    // a drag and surely we can find other matching triangles.
    double const tolerance = 1E-2;
    if( (fabs(this_angles[0] - this_angles[1]) < tolerance)
    ||  (fabs(this_angles[1] - this_angles[2]) < tolerance)
    ||  (fabs(this_angles[0] - this_angles[2]) < tolerance) )
    {
        return 133;                                 // Early return
            // Nothing special about this number, just need something
            // large and distinct from the large numbers we assign
            // elsewhere in this function, when we intend to ensure
            // we violate any reasonable closeness tolerance.
    }

    // Exclude pairs of triangles that, even though their angles might match,
    // are of very different scales.
    if( (Perimeter()/that.Perimeter() > 1.33)
    ||  (Perimeter()/that.Perimeter() < 0.75) )
    {
        return 188;                                 // Early return
    }

    // Now add 1E-6 times the sum of the squared differences of the total
    // energy ratios.
    vector<double> this_energy_ratios(3), that_energy_ratios(3);
    double tot_energy_ratio_squared_diffs(0);
    for( int i=0;i<3;++i )
    {
        this_energy_ratios[i] = _rep[i].TotalEnergy()
                              / _rep[(i+1)%3].TotalEnergy();
        that_energy_ratios[i] = that._rep[(i+*offset)%3].TotalEnergy()
                              / that._rep[(i+*offset+1)%3].TotalEnergy();
        double energy_ratio_diff = this_energy_ratios[i]
                                 - that_energy_ratios[i];
        tot_energy_ratio_squared_diffs += pow(energy_ratio_diff,2);
    }
    //t.Info( "tot_energy_ratio_squared_diffs=%f.",
    //        tot_energy_ratio_squared_diffs);
    result = *min_dist + tot_energy_ratio_squared_diffs*5E-7;

    //t.Info( "distance=%f", result );
    return result;
}


void
Triangle::Display() const
{
    Trace t("Triangle::Display()");
    //::Display( *_sample, *this );
}


vector<Star> const &
Triangle::GetStars() const
{
    return _rep;
}


std::ostream&
operator<<( std::ostream& out, Triangle const & triangle)
{
    out << "[";

    out << triangle._i << ", " << triangle._j << ", " << triangle._k;
    out << "] ";
    
    vector<double> angles = triangle.Angles();
    out << " angles=(";
    for( int i=0;i<3;i++ ) out << angles[i] << " ";
    out << "), energies: (" << triangle._rep[0].TotalEnergy()<<", "
                            << triangle._rep[1].TotalEnergy()<<", "
                            << triangle._rep[2].TotalEnergy();
    out << ") perimeter=" << triangle.Perimeter() << endl;
    return out;
}
