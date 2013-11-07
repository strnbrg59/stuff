/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkChomboTexture.h  
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

#ifndef __vtkChomboTexture_h
#define __vtkChomboTexture_h

#include "vtkObject.h"
#include "VTKChomboConfigure.h" // Include configuration header.

class vtkStructuredPoints;
class vtkChomboReader;

class VTK_VTKChombo_EXPORT vtkChomboTexture : public vtkObject
{
  public: 
    vtkChomboTexture();
    static vtkChomboTexture * New();
    vtkTypeMacro(vtkChomboTexture,vtkObject);
    ~vtkChomboTexture();

    const char * GetClassName() { return "vtkChomboTexture"; }
    void PrintSelf( ostream & os, vtkIndent indent ) { };

    void SetChomboReader( vtkChomboReader * );
    vtkStructuredPoints * MakeTextureMemory( char direction, double position );
    void UpdateMemoryPartition( char direction );
    double * GetMemoryPartition( int boxNum ) const; // const

  private:
    const vtkChomboReader * m_reader;
    vtkStructuredPoints * m_textureMemory;
    int /*RectanglePacker*/ * m_rectanglePacker;
    double * m_dblBuf; // return value for GetMemoryPartition() and others.
};

#endif
