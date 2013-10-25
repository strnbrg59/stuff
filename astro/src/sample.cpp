#include "astro.hpp"
#include "cmdline.hpp"
#include "myregexp.hpp"
#include "trace.hpp"
#include "utils.hpp"

#include <algorithm>
#include <fstream>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>
using std::endl;
#include <unistd.h>

using std::vector;

std::ostream&
operator<<( std::ostream& out, Sample const & sample)
{
    for( unsigned i=0;i<sample.Size();i++ )
    {
        out << sample[i] << endl;
    }
    return out;
}


/** Go through raw_data and collect the cmdline.SampleSize() brightest stars
 *  whose brightest pixel exceeds cmdline.Threshold().
*/
Sample::Sample( RawData const & raw_data,
                Cmdline const & cmdline )
  : _cmdline( cmdline )
{
    Trace t("Sample::Sample()");
    //t.Info( "raw_data.RowMin()=%d, raw_data.RowMax()=%d,"
    //        " raw_data.ColMin()=%d, raw_data.ColMax()=%d",
    //         raw_data.RowMin(), raw_data.RowMax(),
    //         raw_data.ColMin(), raw_data.ColMax() );

    _rawdata_size = std::make_pair( 1+raw_data.RowMax()-raw_data.RowMin(),
                                    1+raw_data.ColMax()-raw_data.ColMin() );


    // Bool matrix, parallels raw_data, to let us know if we've already
    // incorporated a pixel in an previous star's airy rectangle.  This way we
    // avoid collecting duplicate stars.
    vector<vector<bool> > pixels_used( 1+raw_data.RowMax()-raw_data.RowMin() );
    for( unsigned i=0;i<pixels_used.size();++i )
    {
        pixels_used[i].reserve( 1+raw_data.ColMax()-raw_data.ColMin() );
        fill( pixels_used[i].begin(), pixels_used[i].end(), false );
    }
    

    int i=0;
    while( i <= raw_data.RowMax()-raw_data.RowMin() )
    {
        int j=0;
        while( j <= raw_data.ColMax()-raw_data.ColMin() )
        {
            if( ( !pixels_used[i][j])
            &&  (raw_data.PixelAt( i,j ) > _cmdline.Threshold()) )
            {
                bool hit_domain_edge;
                Point<int> coord(i,j);
                AiryRectangle ar( coord, raw_data, &pixels_used,
                                  &hit_domain_edge, _cmdline );
                if( hit_domain_edge )
                {
                    t.Warning( "Hit domain edge on star at [%d,%d;%d].",
                                i, j, raw_data.PixelAt(i,j) );
                } else
                if( ar.IsInvalid() )
                {
                    t.Warning( "AiryRectangle invalid for some other reason." );
                } else
                {
                    _rep.push_back( Star( ar.TotalEnergy(), ar.Centroid() ));
                    // Use push_back, not not operator[] (preceded by resize())
                    // because in AlignWith() we take _rep.size().
                }
            }
            ++j;
        }
        ++i;
    }
    sort( _rep.begin(), _rep.end(), CompareStarEnergy);
    if( _rep.size() > unsigned(_cmdline.SampleSize()) )
    {
        _rep.resize( _cmdline.SampleSize() );
    }

    // Initialize _bad_star_count.  If you ever change the order or insert or
    // delete from _rep after this point, you'll need to make the corresponding
    // change to _bad_star_count.  In view of this programming danger, it would
    // have been safer to put the "bad" count right in class Star, or maybe
    // have _rep be a pair<Star,int>.  But I wanted this->MaybeBadStar() to be
    // const, even though it modifies _bad_star_count, so I had to make
    // something mutable and it seemed less invasive to create _bad_star_count
    // and make just it mutable, than to make _rep itself mutable.
    _bad_star_count.reserve( _rep.size() );
    _bad_star_count.insert( _bad_star_count.end(), _rep.size(), 0 );
}


/** Find transformation to align with another sample.
 *  Sample *this is an alignee, Sample that is the model.
 *  Return 1 on success, 0 on failure.
*/
int
Sample::AlignWith( StretchParams * transform,   // ultimate
                   Sample const & that )
{
    Trace t("Sample::AlignWith()");

    int strike_outs = 0;

    StarAccreter this_accreter, that_accreter;
    Triangle this_triangle( *this, _cmdline),
             that_triangle(  that, _cmdline);

    // True when no more stars left that map to that sample's domain:
    bool done_successfully(false);

    // True when we can't find any more matching triangles.
    bool no_more_triangles(false);
    
    // Mark off stars that we've tried to match up, whether we were successful
    // at that or not.
    vector<bool> used_stars( _rep.size() );
    fill( used_stars.begin(), used_stars.end(), false );

    no_more_triangles =
        _FindNextMatchingTriangles( &this_triangle, &that_triangle, that );
    while( (!no_more_triangles) && (!done_successfully) )
    {
        t.Info() << "Matching triangles:\n"
                 << "                    model:" << that_triangle << '\n'
                 << "                  alignee:" << this_triangle << '\n';
        this_accreter.SetTriangle( this_triangle );
        used_stars[this_triangle.i()] =
            used_stars[this_triangle.j()] =
            used_stars[this_triangle.k()] = true;
        int offset( this_triangle.GetMatchOffset(that_triangle) );
        that_accreter.SetTriangle( that_triangle, offset );
        
        // Find aligning transformation (but don't actually transform star
        // positions):
        this_accreter.AlignWith( that_accreter );
    
        // These are params of the ratio of corresponding star energies in
        // that sample, to *this sample:
        double brightness_ratio_mu, brightness_ratio_sd;
        FindBrightnessRatioParams( &brightness_ratio_mu, &brightness_ratio_sd,
            that_triangle, this_triangle, offset );

        int hits(0), misses(0);
        bool cant_find_match(false);  // Even though should be there.
        bool no_more_unused_stars(false);
        bool too_dim(false);
        bool too_bright(false);
        bool maybe_bad_star(false);
        pair<Star,Star> next_star_pair;

        // We give up when we can reject the hypothesis that the expected value
        // of misses/(hits+misses) is less than _cmdline.MaxMissRatio() at the
        // 90% confidence level.  Let m==misses, n==hits+misses, r=MaxMissRatio.
        // This is a binomial model, so we'll reject (ie quit on this triangle)
        // when m > n*r + 2*sqrt(n*r*(1-r)).
        double r = _cmdline.MaxMissRatio();
        double sd = 0.0;
        while( (misses <= (hits+misses)*r + 2*sd)
        &&     (!no_more_unused_stars) )
        {
            _FindNextStarPair( &next_star_pair,
                               &no_more_unused_stars,
                               &cant_find_match,
                               &too_dim,
                               &too_bright,
                               &maybe_bad_star,
                               &used_stars,
                               that, this_accreter,
                               brightness_ratio_mu, brightness_ratio_sd );

            if( cant_find_match )
            {
                if( (!no_more_unused_stars)
                && (!too_dim) && (!too_bright) && (!maybe_bad_star))
                {
                    ++misses;
                    t.Info( "Missed: no excuses." );
                }
            }
            else
            {
                ++hits;
                this_accreter.AddStar( next_star_pair.first  );
                that_accreter.AddStar( next_star_pair.second );
                this_accreter.AlignWith( that_accreter );
            }

            sd = pow( r * (1-r) * (hits+misses), 0.5 );
            t.Info( "hits=%d, misses=%d, sd=%f, "
                    "would reject above %f misses",
                    hits, misses, sd, (hits+misses)*r + 2*sd );
        }

        // Figure out why we exited that last loop.
        if( misses <= (hits+misses)*r + 2*sd )
        {
            done_successfully = true;
        } else
        {
            t.Info( "Struck out.  Trying a different triangle..." );
            strike_outs++;
            this_triangle.InchForward( _cmdline.TriangleSearchBandwidth() );
            no_more_triangles =
                _FindNextMatchingTriangles( &this_triangle, &that_triangle,
                                            that );
            fill( used_stars.begin(), used_stars.end(), false );
        }
    }

    // Figure out why we exited that last loop.
    if( done_successfully )
    {
        *transform = this_accreter.GetTransform();
        t.Info( "Alignment succeeded after %d strike-outs.", strike_outs );
        return 1;
    } else
    {
        t.Warning( "Ran out of triangles after %d strike-outs.", strike_outs );
        return 0;
    }
}


/** Fill arg this_triangle (already allocated) with three Stars that match up
 *  with three in another Sample (arg that).  Fill arg that_triangle (again,
 *  pre-allocated by client using Triangle(Sample&) ctor) with the matching
 *  triangle from the other sample.
 *  A match is defined by Triangle::Matches(); there are several criteria.
 *  Returns true on success (found a pair of matching triangles) or false on
 *  failure.
*/
bool
Sample::_FindNextMatchingTriangles(
    Triangle * this_triangle,
    Triangle * that_triangle,
    Sample const & that )
{
    Trace t("Sample::_FindMatchingTriangle()");

    while( !this_triangle->Matches( *that_triangle )
    &&     !this_triangle->NoMore() )
    {
        double dim_limit( 
            std::max_element( this_triangle->GetStars().begin(),
                              this_triangle->GetStars().end(),
                              CompareStarEnergy )->TotalEnergy()
            / _cmdline.MaxBrightnessRatio() );
        double bright_limit( 
            std::min_element( this_triangle->GetStars().begin(),
                              this_triangle->GetStars().end(),
                              CompareStarEnergy)->TotalEnergy()
            * _cmdline.MaxBrightnessRatio() );
            // Yep, dim is max_element, bright is min_element.  That's because
            // of how we've defined CompareStarEnergy(), and because the
            // compiler rejects std::not2(std::ptr_fun(CompareStarEnergy))
            // unless we rewrite CompareStarEnergy to take instances, rather
            // than const references, for its arguments.
    
        //t.Info() << "bright_limit=" << bright_limit << '\n';
        //t.Info() << "dim_limit=" << dim_limit << '\n';

        that_triangle->Reset( bright_limit );
        //t.Info()<<"that_triangle reset to i()=" << that_triangle->i() << '\n';
        do {
            that_triangle->Advance( dim_limit );
        } while( !this_triangle->Matches( *that_triangle )
          &&     !that_triangle->NoMore() );

        /*
        if( that_triangle->NoMore() )
        {
            t.Info() << "that_triangle advanced to i()=" << that_triangle->i()
                << " before declaring 'no_more'" << '\n';
        }
        */

        if( !this_triangle->Matches( *that_triangle ) )
        {
            this_triangle->InchForward( _cmdline.TriangleSearchBandwidth() );
            /*
            t.Info() << "this_triangle advanced to ("
                << this_triangle->i() << "," << this_triangle->j()
                << "," << this_triangle->k() << ")\n";
            */
        }
        //t.Info() <<"this_triangle->NoMore()="<< this_triangle->NoMore()<<'\n';
    }

    if( this_triangle->NoMore() )
    {
        return true;
    } else
    {
        return false;
    }
}


void
Sample::_Transform( StretchParams const & transforms )
{
    for( vector<Star>::iterator s = _rep.begin();
         s != _rep.end();
         ++s )
    {
        s->Transform(transforms);
    }
}


Star const &
Sample::operator[](unsigned i) const
{
    assert( i >= 0 && i<_rep.size() );
    return _rep[i];
}


/** Find the unmarked star -- its corresponding element in arg marked_stars is
 *  false -- that is closest to the centroid of the existing stars in arg
 *  accreter.  If there are no more such stars to be had, set arg no_more to
 *  true.
*/
vector<Star>::const_iterator
Sample::_FindUnmarkedCentroidStar(
    StarAccreter const & accreter,
    vector<bool> * marked_stars ) const
{
    Point<double> centroid( accreter.Centroid() );
    double min_distance( 1.0E100 );
    vector<Star>::const_iterator result( _rep.end() );
    vector<Star>::const_iterator iter = _rep.begin();
    for( ; iter != _rep.end(); ++iter )
    {
        double d( centroid.DistanceTo( iter->Coords() ) );
        if( ( d < min_distance )
        &&  ( (*marked_stars)[iter - _rep.begin()] == false ) )
        {
            min_distance = d;
            result = iter;
        }
    }

    if( result != _rep.end() )
    {
        (*marked_stars)[result - _rep.begin()] = true;
    }
    return result;
}
  

/** Find the unmarked star that's closest to the centroid of the stars already
 *  in arg this_accreter, and which, when transformed by this_accreter, should
 *  lie inside the domain of arg that.
 *  Then find its counterpart in arg that, setting arg next_pair to the two
 *  matching stars -- the one from *this, and the one from arg that.
 *
 *  Set arg no_more to true if there are no more unmarked stars that should lie
 *  inside arg that's domain.
 *  Set arg no_match to true if, despite no_more==false, we just can't find, in
 *  arg that, a star close (quite close!) to where we expect the counterpart to
 *  be.
 *  Set arg too_dim if we can't find a good match but the centroid star is dim
 *  enough that, considering the relative brightnesses of the two photographs,
 *  there's a good chance its counterpart simply wasn't imaged.
 *  Arg too_bright kicks in because we exclude stars whose airy rectangle is too
 *  big.
 *  Arg maybe_bad_star gets set to True if there's no match and the star's
 *  neither too dim nor too bright but it's failed to match for a number
 *  (cmd_line.bad_star_max) of previous triangles.  In that case, the "star"
 *  could be a hot pixel that survived the noise-reduction, or a fairly bright
 *  star -- we've seen one with energy 163 -- that just didn't register in the
 *  alignee photograph.
*/
void
Sample::_FindNextStarPair( pair<Star,Star> * next_pair,
                           bool * no_more,
                           bool * no_match,
                           bool * too_dim,
                           bool * too_bright,
                           bool * maybe_bad_star,
                           vector<bool> * used_stars,
                           Sample const & that,
                           StarAccreter const & this_accreter,
                           double brightness_ratio_mu,
                           double brightness_ratio_sd ) const
{
    Trace t("Sample::_FindNextStarPair()");

    *no_more = false;
    *too_dim = false;
    *too_bright = false;
    *maybe_bad_star = false;

    Star centroid_star, transformed_centroid_star;

    // Look until you find a star that, transformed, should lie within the
    // domain of the other sample.
    vector<Star>::const_iterator centroid_star_iter;
    do
    {
        centroid_star_iter =
            _FindUnmarkedCentroidStar( this_accreter, used_stars );
        if( centroid_star_iter != _rep.end() )
        {
            centroid_star = *(centroid_star_iter);
            transformed_centroid_star = centroid_star;
            transformed_centroid_star.Transform(
                this_accreter.GetTransform() );
        } else
        {
            *no_more = true;
        }
    } while(   (!(*no_more))
            && (!that.Contains( transformed_centroid_star.Coords())) );

    if( *no_more )
    {
        *no_match = true;
        return;                           // Early return
    }

    // If we're here, then we have a star that should match up with one in that.
    double distance;
    Star nearest_in_that( that._FindNearest( &distance,
                                             transformed_centroid_star ));
    if( distance < _cmdline.MatchingStarMaxDistance() )
    {
        *no_match = false;
        t.Info() << "Found match at distance " << distance << '\n'
                 << "  centroid_star = " << centroid_star << '\n'
                 << "  transformed   = " << transformed_centroid_star << '\n'
                 << "  nearest       = " << nearest_in_that << '\n';

        *next_pair = std::make_pair( centroid_star, nearest_in_that );
    } else
    {
        t.Info() << "No match: distance = " << distance << '\n'
                 << "  centroid_star = " << centroid_star << '\n'
                 << "  transformed   = " << transformed_centroid_star << '\n'
                 << "  nearest       = " << nearest_in_that << '\n';
        *no_match = true;

        // Maybe the corresponding star is there but it's too dim (correcting
        // for the relative brightness of the two photographs) to have made the
        // cutoff into the stars in the other sample.  Or it's too bright;
        // extremely bright objects get excluded on grounds of overly-large
        // airy rectangles (see Cmdline.airy_rectangle_max_size).
        if( TooDim( centroid_star.TotalEnergy(),
                    that._rep.back().TotalEnergy(),
                    brightness_ratio_mu, brightness_ratio_sd ) )
        {
            *too_dim = true;
            t.Info() << "  (" << centroid_star.TotalEnergy() << " too dim)\n";
        }
        if( TooBright( centroid_star.TotalEnergy(),
                       that._rep.begin()->TotalEnergy(),
                       brightness_ratio_mu, brightness_ratio_sd ) )
        {
            *too_bright = true;
            t.Info() << "  (" << centroid_star.TotalEnergy()<< " too bright)\n";
        }
        if( (*no_match) && (!*too_dim) && (!*too_bright) )
        {
            if( MaybeBadStar( centroid_star_iter ) )
            {
                *maybe_bad_star = true;
                t.Info() << "  (" << centroid_star << " maybe bad star)\n";
            }
        }
    }
}


/** Keeps track of how many times a star failed to match, despite being neither
 *  too dim nor too bright nor outside the domain.  If a star's "bad-count"
 *  gets past cmd_line.bad_star_max, then that star is considered "bad".
 *  Const, but modifies mutable _bad_star_count.
*/
bool
Sample::MaybeBadStar( vector<Star>::const_iterator star_iter ) const
{
    Trace t("Sample::MaybeBadStar()");
    int count = ++ _bad_star_count[ star_iter - _rep.begin() ];
    //t.Info( "count=%d", count );
    if( count >= _cmdline.BadStarMax() )
    {
        return true;
    } else
    {
        return false;
    }
}


/** Return true if coordinates fall within the domain, i.e. within this sample's
 *  associated RawData.
*/
bool
Sample::Contains( Point<double> const & star_coords ) const
{
    Trace t("Sample::Contains()");

    if( (star_coords.x() > 0)
    &&  (star_coords.x() < _rawdata_size.first)
    &&  (star_coords.y() > 0)
    &&  (star_coords.y() < _rawdata_size.second) )
    {
/*
        t.Info( "Found star at [%f,%f] to be inside rawdata with dimensions "
                "[%d,%d]", star_coords.x(), star_coords.y(),
                           _rawdata_size.first, _rawdata_size.second );
*/
        return true;
    } else
    {
        t.Info( "Star at [%f,%f] is OUTSIDE rawdata with dimensions "
                "[%d,%d]", star_coords.x(), star_coords.y(),
                           _rawdata_size.first, _rawdata_size.second );
        return false;
    }
}


/** Return the star in this sample whose coordinates are closest to those of
 *  arg star.   Set arg distance to the distance between arg star and that
 *  closest star.
*/
Star
Sample::_FindNearest( double * distance, Star star ) const
{
    assert( _rep.size() > 0 );
    vector<Star>::const_iterator iter = _rep.begin();
    vector<Star>::const_iterator result = _rep.end();
    *distance = 1.0E100;
    for( ; iter != _rep.end(); ++iter )
    {
        double d( iter->Coords().DistanceTo( star.Coords() ) );
        if(  d < *distance )
        {
            *distance = d;
            result = iter;
        }
    }
            
    assert( result != _rep.end() );
    return *result;
}


/** We used to return true if dimmest_energy/this_energy was more than two sd's
 *  above the mean, but that caused problems.  Sometimes the sd was large, and
 *  so we never ruled anything "too dim", even stars with energies above 150!
 *  The simpler rule we now use seems to work more reliably.  We have to be
 *  quite tolerant about failing to find matching stars that are dim, as dim
 *  stars can easily be lost to the dark-noise-removal.
*/
bool
Sample::TooDim( 
    double this_energy, double dimmest_energy,
    double mu, double sd ) const
{
    Trace t("Sample::TooDim()");
    return (this_energy < 3 * dimmest_energy / mu) || (this_energy < 40);
}
bool
Sample::TooBright( 
    double this_energy, double brightest_energy,
    double mu, double sd ) const
{
    return brightest_energy/this_energy < mu + 2*sd;
}


/** Produce ChomboVis .dat file that shows two samples on top of each other.
 *  Particles component "discrim" can be used to discriminate between the
 *  samples.
*/
void
Display( string outfile_name, Sample const & s1, Sample const & s2 )
{
    std::ofstream outfile( outfile_name.c_str() );

    // ChomboVis boilerplate.
    string boilerplate1("space_dim: 2\n"
                        "num_levels: 1\n"
                        "num_components: 1\n"
                        "component_names: phi\n"
                        "data_centering: 0\n"
                        "prob_domain: ");
    outfile << boilerplate1;
    outfile << "0 0 0 0" << endl;
    outfile << "origin: " << "0 " << -s1.RawdataSize().second << endl
            << "anisotropic: " << s1.RawdataSize().second << " "
                               << s1.RawdataSize().first  << endl;


    outfile << "level_0:\n";
    outfile << "    dx: " << 1 << endl;  // But see "anisotropic", above.

    outfile << "    output_ghost: 0 0\n"
            << "    boxes:\n";
    outfile << "        0 0 0 0" << endl;
    outfile << "    data:\n 1" << endl;

    outfile << "particles:" << endl
            << "    num_components: 3" << endl
            << "    num_particles: " << s1._rep.size() +s2._rep.size() << endl;


    // Assemble particles data.
    vector<double> s1_x(s1._rep.size()), s1_y(s1._rep.size()),
                   s2_x(s2._rep.size()), s2_y(s2._rep.size()),
                   s1_discrim(s1._rep.size()),
                   s2_discrim(s2._rep.size());

    int i=0;
    for( vector<Star>::const_iterator s = s1._rep.begin();
         s != s1._rep.end();
         ++s )
    {
        s1_x[i] = -s->x();
        s1_y[i] = s->y();
        s1_discrim[i] = 1;
        ++i;
    }

    i=0;
    for( vector<Star>::const_iterator s = s2._rep.begin();
         s != s2._rep.end();
         ++s )
    {
        s2_x[i] = -s->x();
        s2_y[i] = s->y();
        s2_discrim[i] = 2;
        ++i;
    }


    outfile << "    component_0: " << endl
            << "        name: position_x" << endl
            << "        values: " << s1_y << s2_y << endl;
        // ChomboVis orientation is rotated 90 and flipped vertically.
    outfile << "    component_1: " << endl
            << "        name: position_y" << endl
            << "        values: " << s1_x << s2_x << endl;
    outfile << "    component_2: " << endl
            << "        name: discrim" << endl
            << "        values: " << s1_discrim << s2_discrim << endl;
}


void Display( string outfile_name,
              Sample const & sample1, Triangle const & triangle1,
              Sample const & sample2, Triangle const & triangle2 )
{
    assert(0);
}


/** Produce ChomboVis .dat file that shows a sample and a triangle chosen from
 *  it.  Particles component "discrim" can be used to discriminate between the
 *  triangle stars and the rest of the stars in the sample.
*/
void Display( string outfile_name,
              Sample const & s1,  Triangle const & t1 )
{
    std::ofstream outfile( outfile_name.c_str() );

    // ChomboVis boilerplate.
    string boilerplate1("space_dim: 2\n"
                        "num_levels: 1\n"
                        "num_components: 1\n"
                        "component_names: phi\n"
                        "data_centering: 0\n"
                        "prob_domain: ");
    outfile << boilerplate1;
    outfile << "0 0 0 0" << endl;
    outfile << "origin: " << "0 " << -s1.RawdataSize().second << endl
            << "anisotropic: " << s1.RawdataSize().second << " " 
                               << s1.RawdataSize().first  << endl;

    outfile << "level_0:\n";
    outfile << "    dx: " << 1 << endl;

    outfile << "    output_ghost: 0 0\n"
            << "    boxes:\n";
    outfile << "        0 0 0 0" << endl;
    outfile << "    data:\n 1" << endl;

    outfile << "particles:" << endl
            << "    num_components: 3" << endl
            << "    num_particles: " << s1._rep.size() << endl;

    // Assemble particles data.
    vector<double> s1_x(s1._rep.size()), s1_y(s1._rep.size()),
                   discrim(s1._rep.size());

    int i=0;
    for( vector<Star>::const_iterator s = s1._rep.begin();
         s != s1._rep.end();
         ++s )
    {
        s1_x[i] = -s->x();
        s1_y[i] = s->y();
        if( (int(t1._i) == i) || (int(t1._j) == i) || (int(t1._k) == i) )
        {
            discrim[i] = 1.5;
        } else
        {
            discrim[i] = 1;
        }
        ++i;
    }

    outfile << "    component_0: " << endl
            << "        name: position_x" << endl
            << "        values: " << s1_y << endl;
        // ChomboVis orientation is rotated 90 and flipped vertically.
    outfile << "    component_1: " << endl
            << "        name: position_y" << endl
            << "        values: " << s1_x  << endl;
    outfile << "    component_2: " << endl
            << "        name: discrim" << endl
            << "        values: " << discrim << endl;

}


/** Produce ChomboVis .dat file that shows a sample and a set of triangles
 *  chosen from it.
 *  Particles component "discrim" can be used to discriminate between the
 *  triangle stars and the rest of the stars in the sample.
*/
void Display( string outfile_name,
              Sample const & s1,  vector<Triangle> const & vt1 )
{
    Trace t("Display( string, Sample, vector<Triangle>)");
    t.Info( "vt1.size()=%d", vt1.size() );
    std::ofstream outfile( outfile_name.c_str() );

    // ChomboVis boilerplate.
    string boilerplate1("space_dim: 2\n"
                        "num_levels: 1\n"
                        "num_components: 1\n"
                        "component_names: phi\n"
                        "data_centering: 0\n"
                        "prob_domain: ");
    outfile << boilerplate1;
    outfile << "0 0 0 0" << endl;
    outfile << "origin: " << "0 " << -s1.RawdataSize().second << endl
            << "anisotropic: " << s1.RawdataSize().second << " " 
                               << s1.RawdataSize().first  << endl;

    outfile << "level_0:\n";
    outfile << "    dx: " << 1 << endl;

    outfile << "    output_ghost: 0 0\n"
            << "    boxes:\n";
    outfile << "        0 0 0 0" << endl;
    outfile << "    data:\n 1" << endl;

    outfile << "particles:" << endl
            << "    num_components: 3" << endl
            << "    num_particles: " << s1._rep.size() << endl;

    // Assemble particles data.
    vector<double> s1_x(s1._rep.size()), s1_y(s1._rep.size()),
                   discrim(s1._rep.size());

    for( unsigned i=0; i<s1._rep.size(); ++i )
    {
        s1_x[i] = -s1._rep[i].x();
        s1_y[i] =  s1._rep[i].y();

        // Set discrim so it discriminates between stars of the various
        // triangles.  For the i-th triangle, discrim=i+1.0.  Stars not in any
        // triangle get discrim=1.0.
        discrim[i] = 1.0;
        for( unsigned t=0; t<vt1.size(); ++t )
        {
            if( (vt1[t]._i == i)
            ||  (vt1[t]._j == i)
            ||  (vt1[t]._k == i) )
            {
                discrim[i] = t + 2.0;
            }
        }
    }

    outfile << "    component_0: " << endl
            << "        name: position_x" << endl
            << "        values: " << s1_y << endl;
        // ChomboVis orientation is rotated 90 and flipped vertically.
    outfile << "    component_1: " << endl
            << "        name: position_y" << endl
            << "        values: " << s1_x  << endl;
    outfile << "    component_2: " << endl
            << "        name: discrim" << endl
            << "        values: " << discrim << endl;

}


/** Arg hit_domain_edge gets set to true if the AiryRectangle reaches the edge
 *  of arg raw_data and it's not clear we've reached the AiryRectangle's true
 *  boundary.  If we set *hit_domain_edge, the AiryRectangle can be presumed to
 *  be unusable (and indeed we return immediately).
*/
AiryRectangle::AiryRectangle( Point<int> const & presumed_center,
                              RawData const & raw_data,
                              vector<vector<bool> > * pixels_used,
                              bool * hit_domain_edge,
                              Cmdline const & cmdline )
  : _presumed_center( presumed_center ),
    _raw_data( raw_data ),

    _lo_corner(1,1),   // Initialize to impossible values, so we can detect
    _hi_corner(-1,-1), // if these things have been computed yet.
    _centroid(-2,-2),
    _total_energy(-1),

    _is_invalid( false ),
    _cmdline( cmdline )
{
    Trace t("AiryRectangle::AiryRectangle()");

    // Do what you can to ensure that hit_domain_edge is not 0 or a bad pointer.
    assert( hit_domain_edge );
    bool dummy = *hit_domain_edge;
    dummy += 1;

    _FindCorners( presumed_center, raw_data, pixels_used, hit_domain_edge );
    
    if( *hit_domain_edge )
    {
        _is_invalid = true;
        t.Info( "Hit domain edge (presumed center=[%d,%d])",
                presumed_center.i(), presumed_center.j() );
        return;                                          // Early return.
    }
    _FindCentroid();
/*
    t.Info( "Size=%dx%d, center=[%d,%d].", _hi_corner.i()-_lo_corner.i(),
                             _hi_corner.j()-_lo_corner.j(),
                             presumed_center.i(), presumed_center.j() );
*/
}


/** Establish _lo_corner and _hi_corner of the AiryRectangle.  Policy regarding
 *  arg hit_domain_edge is same as in AiryRectangle ctor.
*/
void
AiryRectangle::_FindCorners( Point<int> const & presumed_center,
                             RawData const & raw_data,
                             vector<vector<bool> > * pixels_used,
                             bool * hit_domain_edge )
{
    Trace t("AiryRectangle::_FindCorners()");

    *hit_domain_edge = false; // Until evidence to the contrary

    // Look across rows of increasing number.
    int i=presumed_center.i();
    int j=presumed_center.j();
    while( (i<=raw_data.RowMax()) 
    &&     (raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark()) )
    {
        ++i;
    }
    if( i > raw_data.RowMax()-1 )  // -1 because in _EdgesNotDark() we're going
    {                              // to look one pixel beyond.
        *hit_domain_edge = true;
        // We could return here, but we won't because we want to get all four
        // corners so we can mark as "visited" the corresponding elements of
        // raw_data.
    }
    _hi_corner.i( i-1 );

    // Look across rows of decreasing number.
    i=presumed_center.i();
    j=presumed_center.j();
    while( (i>=raw_data.RowMin()) 
    &&     (raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark()) )
    {
        --i;
    }
    if( i < raw_data.RowMin()+1 )
    {
        *hit_domain_edge = true;
    }
    _lo_corner.i( i+1 );

    // Look across columns of increasing number.
    i=presumed_center.i();
    j=presumed_center.j();
    while( (j<=raw_data.ColMax()) 
    &&     (raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark()) )
    {
        ++j;
    }
    if( j > raw_data.ColMax()-1 )
    {
        *hit_domain_edge = true;
    }
    _hi_corner.j( j-1 );

    // Look across columns of decreasing number.
    i=presumed_center.i();
    j=presumed_center.j();
    while( (j>=raw_data.ColMin()) 
    &&     (raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark()) )
    {
        --j;
    }
    if( j < raw_data.ColMin()+1 )
    {
        *hit_domain_edge = true;
    }
    _lo_corner.j( j+1 );


    // Make sure it's dark (i.e. within _cmdline.AiryRectangleDark()) all the
    // way around.  If not, grow the AiryRectangle by a pixel in the appropriate
    // direction -- 'n','s','e' or 'w'.
    char direction;
    while( (! (*hit_domain_edge))
    &&     (! _TooBig())
    &&     _EdgesNotDark( &direction, raw_data ) )
    {
        _Expand( direction, hit_domain_edge, raw_data );
    }

    // Reject rectangles that are too big.
    if( _TooBig() || (*hit_domain_edge) )
    {
        if( _TooBig() )
        {
            t.Info( "Airy rectangle is too big (presumed center=[%d,%d])",
                     presumed_center.i(), presumed_center.j() );
        }
        _is_invalid = true;
    }


    // Mark as used all the pixels in the rectangle.
    for( int i=_lo_corner.i(); i<=_hi_corner.i(); ++i )
    {
        for( int j=_lo_corner.j(); j<=_hi_corner.j(); ++j )
        {
            (*pixels_used)[i][j] = true;
        }
    }
}


/** Return true if, one pixel past any of the outer edges (top, bottom, right
 *  or left -- direction 'n','s','e','w') all the pixels are not dark, i.e.
 *  within _cmdline.AiryRectangleDark().  And if so, set arg direction to
 *  indicate which edge we will need to expand from.
 *  Also return true if, for any direction, we can't look one pixel out because
 *  that would take us past the domain edge.
*/
bool
AiryRectangle::_EdgesNotDark( char * direction, RawData const & raw_data )
  const
{
    Trace t("AiryRectangle::_EdgesNotDark()");

    // Top (direction 'n').
    int i( _hi_corner.i()+1 );
    for( int j=_lo_corner.j()-1; j<=_hi_corner.j()+1;++j )
    {
        if( raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark() )
        {
            *direction = 'n';
            return true;
        }
    }

    // Bottom (direction 's').
    i = _lo_corner.i()-1;
    for( int j=_lo_corner.j()-1; j<=_hi_corner.j()+1;++j )
    {
        if( raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark() )
        {
            *direction = 's';
            return true;
        }
    }

    // Right (direction 'e').
    int j( _hi_corner.j()+1 );
    for( int i=_lo_corner.i(); i<=_hi_corner.i();++i )
    {
        if( raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark() )
        {
            *direction = 'e';
            return true;
        }
    }

    // Left (direction 'w').
    j = _lo_corner.j()-1;
    for( int i=_lo_corner.i(); i<=_hi_corner.i();++i )
    {
        if( raw_data.PixelAt(i,j) > _cmdline.AiryRectangleDark() )
        {
            *direction = 'w';
            return true;
        }
    }

    return false;
}


/** Grow the AiryRectangle in the indicated direction.  Check if that takes us
 *  past the edge of the domain.
*/
void
AiryRectangle::_Expand( char direction, bool * hit_domain_edge,
                        RawData const & raw_data )
{
    Trace t("AiryRectangle::_Expand()");
    //t.Info("direction=%c", direction);
    switch( direction )
    {
        case 'n' : _hi_corner.i( _hi_corner.i() + 1 ); break;
        case 's' : _lo_corner.i( _lo_corner.i() - 1 ); break;
        case 'e' : _hi_corner.j( _hi_corner.j() + 1 ); break;
        case 'w' : _lo_corner.j( _lo_corner.j() - 1 ); break;
    }

    if( (_hi_corner.i()+1 > raw_data.RowMax())
    ||  (_lo_corner.i()-1 < raw_data.RowMin())
    ||  (_hi_corner.j()+1 > raw_data.ColMax())
    ||  (_lo_corner.j()-1 < raw_data.ColMin()) )
    {
        *hit_domain_edge = true;
    }
    // Checked *_corner.*()+1 because next thing that's going to be called is
    // _EdgesNotDark(), and that's going to want to look one more layer out.
}


bool
AiryRectangle::_TooBig() const
{
    return
           (_hi_corner.i() - _lo_corner.i() > _cmdline.AiryRectangleMaxSide())
         ||(_hi_corner.j() - _lo_corner.j() > _cmdline.AiryRectangleMaxSide());
}


/** Calls _FindCentroid() on first time. */
Point<double>
AiryRectangle::Centroid()
{
    if( _centroid.x() < _lo_corner.i() )
    {
        _FindCentroid();
    }
    return _centroid;
}


/** Sets _centroid to average of i & j coords weighted by pixel energies.  */
void
AiryRectangle::_FindCentroid()
{
    // Find centroid's x: weighted average over rows.
    int weighted_sum_i(0), weighted_sum_j(0);
    _total_energy = 0;
    int energy;
    for( int i=_lo_corner.i(); i<=_hi_corner.i(); ++i )
    {
        for( int j=_lo_corner.j(); j<=_hi_corner.j(); ++j )
        {
            energy = _raw_data.PixelAt(i,j);
            weighted_sum_i += energy * i;
            weighted_sum_j += energy * j;
            _total_energy += energy;
        }
    }
    _centroid.x( (weighted_sum_i + 0.0)/_total_energy );
    _centroid.y( (weighted_sum_j + 0.0)/_total_energy );
}


/** Calculates _total_energy on first time. */
int
AiryRectangle::TotalEnergy()
{
    if( _total_energy < 0 )
    {
        _FindCentroid();  // Sets _total_energy as a side-effect.
    }
    return _total_energy;
}


/** Find the mean and sd of the three ratios of energies of corresponding stars
 *  in the triangles.
*/
void FindBrightnessRatioParams( double * mu, double * sd,
    Triangle const & t_numerator, Triangle const & t_denominator,
    int offset_for_numerator )
{
    vector<Star> const & numerator_stars( t_numerator.GetStars() );
    vector<Star> const & denominator_stars( t_denominator.GetStars() );
    double s(0.0), ss(0.0);
    for( int v=0;v<3;++v )
    {
        double term( numerator_stars[(v+offset_for_numerator)%3].TotalEnergy()
                   / denominator_stars[v].TotalEnergy() );
        s += term;
        ss += term*term;
    }

    *mu = s/3;
    *sd = pow( (ss - s*s/3)/2, 0.5 );
}
