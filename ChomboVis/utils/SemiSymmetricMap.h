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
//
// Author: Ted Sternberg
// Created: March 19, 2004
//

#ifndef INCLUDED_SEMI_SYMMETRIC_MAP_H
#define INCLUDED_SEMI_SYMMETRIC_MAP_H

#include <map>

/** In various places in ChomboVis, we need to "pass" a pointer through an
 *  interface that does not understand pointers.  One case comes up because
 *  VTK's Python wrapping mechanism does not let us return a pointer (except
 *  to a VTK class).  The other case regards the need to give the Python
 *  classes in visualizable_dataset.py and box_layout_data.py a way to identify
 *  the instances of the C++ classes VisualizableDataset and BoxLayoutData
 *  which which they are associated; Python doesn't understand pointers.
 *
 *  Typically, the type T here will be a pointer type, very often a void*.
 *
 *  We call this "symmetric" because you can look up by key and by value
 *  just as conveniently.  The "semi" part refers to the fact that while when
 *  we can't find a value we insert it into the map, we don't do that if we
 *  can't find a key.
*/
template<typename T> class SemiSymmetricMap
{
  public:
    SemiSymmetricMap() : m_nextKey(0) { }
    int GetKey( T );
    T GetValue( int key ) const;
    int RemoveByKey( int key );
  private:
    // Implemented as two maps, for speedy lookup by either int or T.
    std::map<int, T> m_intKeyMap;
    std::map<T, int> m_valueKeyMap;
    int m_nextKey;
};

#include "SemiSymmetricMap.hImpl"
#endif // INCLUDED_SEMI_SYMMETRIC_MAP_H
