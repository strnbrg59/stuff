/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/
// Author: Ted Sternberg

#ifndef INCLUDED_BOXLAYOUT_H
#define INCLUDED_BOXLAYOUT_H

#include "Box.h"
#include <fstream>
#include <list>
#include <vector>
using std::vector;

/** Looks like a std::vector<Box>, but has some extras. */
template<class REAL_T> class BoxLayout
{
  public:
    BoxLayout();
    BoxLayout( BoxLayout<REAL_T> const & );
    BoxLayout( std::vector<Box>, Triple<REAL_T> dx, Triple<REAL_T> origin );
    BoxLayout & operator=( BoxLayout const & );
    Triple<REAL_T> const & GetDx() const { return m_dx; }
    Triple<REAL_T> const & GetOrigin() const { return m_origin; }

    int GetDimensionality() const;
    void PrintBoxIDs( std::ostream& ) const;
    vector<vector<vector<int> > > GetAsNestedVectors() const;

    //
    // STL stuff to make this class look like a vector<Box>.
    //
    Box operator[]( int ) const;
    Box & operator[]( int );
    unsigned int size() const;
    std::vector<Box>::iterator begin();
    std::vector<Box>::iterator end();
    typedef std::vector<Box>::iterator iterator;
    void push_back( Box const & );
    void assign( std::list<Box>::iterator iBegin,
                 std::list<Box>::iterator iEnd,
                 Triple<REAL_T> dx, Triple<REAL_T> origin );
  private:
    std::vector<Box> m_rep;
    Triple<REAL_T> m_dx;
    Triple<REAL_T> m_origin;
};

#endif // INCLUDED_BOXLAYOUT_H
