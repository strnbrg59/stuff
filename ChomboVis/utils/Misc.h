#ifndef INCLUDED_MISC_H
#define INCLUDED_MISC_H

#include <set>
#include <vector>
#include <iostream>

template<class T> std::ostream & operator<<( std::ostream &, std::set<T> const & );
template<class T> std::ostream & operator<<( std::ostream &, std::vector<T> const & );

#include "Misc.hImpl"

#endif // INCLUDED_MISC_H
