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

#ifndef _INCLUDED_VTKFLOATARRAY_H
#define _INCLUDED_VTKFLOATARRAY_H

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

/** In a class templatized on REAL_T (i.e. float/double), instead of saying
 *  things like
 *      if( sizeof(REAL_T)==sizeof(float) ) array = vtkFloatArray::New();
 *      else                                array = vtkDoubleArray::New();
 *  you can say
 *      array = vtkArray<REAL_T>::New();
*/
template<class REAL_T> class vtkArray;

template<> class vtkArray<float> : public vtkFloatArray
{
};

template<> class vtkArray<double> : public vtkDoubleArray
{
};

#endif
