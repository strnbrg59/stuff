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

#include <cassert>
#include "vtkChomboRawGL.h"
#include "vtkObjectFactory.h"

#include <GL/gl.h>
#include <GL/glu.h>
//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkChomboRawGL);

vtkChomboRawGL::vtkChomboRawGL()
{
}

void
vtkChomboRawGL::SetFog( int on_off )
{
    assert( (on_off==0) || (on_off==1) );
    if( on_off == 0 )
    {
        glDisable(GL_FOG);
    } else
    {
        glEnable(GL_FOG);
        glHint(GL_FOG_HINT, GL_DONT_CARE);
    //  glFogi(GL_FOG_MODE, GL_EXP);
    //  glFogf(GL_FOG_DENSITY, 0.55);
        glFogi(GL_FOG_MODE, GL_LINEAR);
    //  static float fogColor[] = {1.0, 1.0, 1.0, 1.0};
    //  glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_START, 1.0 );
        glFogf(GL_FOG_END, 30.0 );
    }
}


void
vtkChomboRawGL::SetFogColor( float r, float g, float b, float alpha )
{
    static float rgba[4];
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = alpha;
    glFogfv(GL_FOG_COLOR, rgba);
}

void
vtkChomboRawGL::SetFogStart( double x )
{
    glFogf(GL_FOG_START, x );
}

void
vtkChomboRawGL::SetFogEnd( double x )
{
    glFogf(GL_FOG_END, x );
}
