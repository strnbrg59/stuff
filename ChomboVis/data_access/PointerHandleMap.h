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

#ifndef INCLUDED_POINTER_HANDLE_MAP_H
#define INCLUDED_POINTER_HANDLE_MAP_H

#include "../utils/SemiSymmetricMap.h"
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

template<typename T> class SharedPointerHandleMap
{
  public:
    static int GetHandle( boost::shared_ptr<T> p );
    static boost::shared_ptr<T> GetPointer( int k );
    static int RemoveByHandle( int k );

//  private:
    static SemiSymmetricMap<boost::shared_ptr<T> > * s_map;
        // The const versions aren't used.
};


template<typename T> class SharedArrayHandleMap
{
  public:
    static int GetHandle( boost::shared_array<T> p );
    static boost::shared_array<T> GetPointer( int k );
    static int RemoveByHandle( int k );

//  private:
    static SemiSymmetricMap<boost::shared_array<T> > * s_map;
        // The const versions aren't used.
};

//
// A template for getting the const'ed and unconst'ed version of a type.
//
template<typename T> struct TypeOp
{
    typedef T          ArgT;
    typedef T *       pArgT;

    typedef T          BareT;
    typedef T *       pBareT;

    typedef T const    ConstT;
    typedef T const * pConstT;
};

template<typename T> struct TypeOp<T const>
{
    typedef T const    ArgT;
    typedef T const * pArgT;

    typedef T          BareT;
    typedef T *       pBareT;

    typedef T const    ConstT;
    typedef T const * pConstT;
};    

#include "PointerHandleMap.hImpl"

#endif // INCLUDED_POINTER_HANDLE_MAP_H
