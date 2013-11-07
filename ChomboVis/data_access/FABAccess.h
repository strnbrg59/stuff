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
#ifndef DEFINED_FAB_ACCESS_H
#define DEFINED_FAB_ACCESS_H

/** Really fast access to a FAB's data, viewed as a 3D array (but in fact
 *  stored as a Fortran-order 1D array).
 *  Args i, j and k are the coordinates.
 *  Args I and J are the top of the ranges of i and j.
 *  Arg arrayData is the Fortran-order 1D array.
 *
 *  We expose this function to ChomboVtk-level classes like AMRStreamSource
 *  that need really fast access to the data.  They first obtain the arrayData
 *  pointer from vtkChomboReader::GetFAB().
 *  When speed is not of the essence,
 *  use vtkChomboReader::GetDatum().
*/

namespace FabAccess
{

template<class REAL_T> REAL_T
GetArrayItemFast( int i, int j, int k, int I, int J, REAL_T const * arrayData )
{
    return arrayData[ i +
                      j * I +
                      k * I * J ];
}


template<class REAL_T> REAL_T &
GetArrayItemFast( int i, int j, int k, int I, int J, REAL_T * arrayData )
{
    return arrayData[ i +
                      j * I +
                      k * I * J ];
}


/* Macro version:  Found to be no faster than inline function.
#define GetArrayItemFast( i, j, k, I, J, arrayData ) \
    arrayData[ i +  j * I +   k * I * J ]
*/    

template<class REAL_T> inline void
SetArrayItemFast( int i, int j, int k, int I, int J, REAL_T * arrayData,
                  REAL_T newVal )
{
    arrayData[ i +  j * I +  k * I * J ] = newVal;
}

} // namespace

#endif // DEFINED_FAB_ACCESS_H
