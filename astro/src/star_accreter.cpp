#include "astro.hpp"
#include "trace.hpp"

/** Initialize *this (throwing away any stars previously accreted and
 *  transformations saved.
*/
void
StarAccreter::SetTriangle( Triangle const & triangle, int offset/*=0*/ )
{
    Trace t("StarAccreter::SetTriangle()");
    _rep.clear();
    _rep.reserve(3);
    for( int i=0;i<3;++i )
    {
        _rep.push_back( triangle.GetStars()[(i+offset)%3] );
    }
}


void
StarAccreter::AlignWith( StarAccreter const & that )
{
    // We want to estimate transformation that will align the stars in *this
    // with those in that.
    vector<Star> rep_copy( _rep );
    int polynomial_degree;
    if( _rep.size() < 10 )
    {
        polynomial_degree = 1;
    } else
    {
        polynomial_degree = 2;
    }
    _transform.Estimate( that._rep, rep_copy, polynomial_degree );
/*
    for( unsigned i=0; i<_rep.size(); ++i )
    {
        cerr << "this star[" << i << "]=" << _rep[i] << endl;
        Star s( _rep[i] );
        s.Transform( _transform );
        cerr << "  transformed: " << s << endl;
        cerr << "  that star = " << that._rep[i] << endl;
    }
*/
}


void
StarAccreter::AddStar( Star const & star )
{
    _rep.push_back( star );
}


/** Does not weight by star energy.  Just straight average coordinate of the
 *  stars.
*/
Point<double>
StarAccreter::Centroid() const
{
    double sum_x(0), sum_y(0);
    for( unsigned i=0; i<_rep.size(); ++i )
    {
        sum_x += _rep[i].x();
        sum_y += _rep[i].y();
    }
    return Point<double>( sum_x/_rep.size(), sum_y/_rep.size() );
}
