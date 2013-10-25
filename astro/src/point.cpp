#include "astro.hpp"

template<> int Point<int>::i() const { return _rep.first; }
template<> int Point<int>::j() const { return _rep.second; }
template<> void Point<int>::i( int _i ) { _rep.first = _i; }
template<> void Point<int>::j( int _j ) { _rep.second = _j; }

template<> double Point<double>::x() const { return _rep.first; }
template<> double Point<double>::y() const { return _rep.second; }
template<> void Point<double>::x( double _x ) { _rep.first = _x; }
template<> void Point<double>::y( double _y ) { _rep.second = _y; }

template<> void
Point<double>::Transform( StretchParams const & transforms )
{
    double xx( _rep.first );
    double yy( _rep.second );
    transforms.Transform( &xx, &yy );
    _rep.first = xx;
    _rep.second = yy;
}
