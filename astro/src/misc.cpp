#include "astro.hpp"

std::ostream&
operator<<( std::ostream& out, Star const & star )
{
    out << "(" << star.x() << ", " << star.y() << "; "
        << star.TotalEnergy() << ") ";
    return out;
}


Star::Star()
  : _total_energy(0.0),
    _coords(Point<double>(0.0,0.0))
{
}

Star::Star( double energy, Point<double> xy )
  : _total_energy(energy),
    _coords(xy)
{
}


Star::Star( Star const & that )
  : _total_energy( that._total_energy ),
    _coords( that._coords )
{
}

Star&
Star::operator=( Star const & that )
{
    _total_energy = that._total_energy;
    _coords = that._coords;
    return *this;
}


void
Star::Transform( StretchParams const & params )
{
    _coords.Transform( params );
}

double
Star::x() const
{
    return _coords.x();
}
double
Star::y() const
{
    return _coords.y();
}

/** Functor for std::sort() */
bool
CompareStarEnergy( Star const & star1, Star const & star2 )
{
    return star1.TotalEnergy() > star2.TotalEnergy();
}
