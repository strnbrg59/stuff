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
#ifndef INCLUDED_HETEROMAP_H
#define INCLUDED_HETEROMAP_H

#include "StatusCodes.h"
#include <map>
using std::map;
class TypeWrapperBase;

/* Typesafe map container of heterogeneous objects.  Holds pointers to copies
 * it makes of the objects.  Getter uses dynamic_cast to verify type
 * correctness.
*/
template<class KEY_T> class HeteroMap
{
  public:
    HeteroMap();
    ~HeteroMap();
    template<class T> void Put( KEY_T key, T const & value, Status * status=0 );
    template<class T> T const & Get( KEY_T key, T const * dummy,
                                     Status * status = 0 ) const;
    void UnitTest();

  private:
    HeteroMap( HeteroMap const & );             // deliberately undefined.
    HeteroMap & operator=( HeteroMap const & ); // deliberately undefined.

    map<KEY_T, TypeWrapperBase const *> m_rep;
};

#include "HeteroMap.hImpl"

#endif
