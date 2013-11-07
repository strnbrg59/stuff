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
** Author: Ted Sternberg
*/

#ifndef INCLUDED_NEWCOMPONENTGENERATOR_H
#define INCLUDED_NEWCOMPONENTGENERATOR_H

#include <Python.h>
#include <boost/shared_array.hpp>

//
// NewComponentGenerator.  See ../../utils/VectorFunctor.h for parent class.
//
template<class REAL_T> class NewComponentGenerator
  : public VectorFunctor<REAL_T>
{
  public:
    NewComponentGenerator( PyObject * callable );
    ~NewComponentGenerator() { }

    // Client takes ownership of returned pointer.
    // Returns NULL on error.
    boost::shared_array<REAL_T> operator()(
        vector<boost::shared_array<REAL_T> > args, int length ) const;

  private:
    PyObject * m_callable;
};

#include "NewComponentGenerator.hImpl"

#endif // INCLUDED_NEWCOMPONENTGENERATOR_H
