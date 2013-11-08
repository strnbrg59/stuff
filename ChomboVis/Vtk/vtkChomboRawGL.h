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

#ifndef __vtkChomboRawGL_h
#define __vtkChomboRawGL_h

#include "VTKChomboConfigure.h"
#include "vtkObject.h"

class VTK_VTKChombo_EXPORT vtkChomboRawGL : public vtkObject
{
  public:
    static vtkChomboRawGL *New();
    vtkTypeMacro(vtkChomboRawGL,vtkObject);
//  void PrintSelf(ostream& os, vtkIndent indent);

    void SetFog( int on_off );
    void SetFogStart( double );
    void SetFogEnd( double );
    void SetFogColor( float r, float g, float b, float alpha );

  protected:
    ~vtkChomboRawGL() {};
    vtkChomboRawGL();
    vtkChomboRawGL(const vtkChomboRawGL&) {};
    void operator=(const vtkChomboRawGL&) {};
};

#endif