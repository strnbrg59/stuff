/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkRayBuilder.cxx
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

/* provided as a modification from source material derived from
   the VTK distribution.  In accordance with VTK software agreement
   the following also holds:
=========================================================================



Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/


#include "vtkRayBuilder.h"
#include "vtkObjectFactory.h"
#include "vtkWindow.h"
#include "../utils/Trace.h"
#include <cassert>

vtkStandardNewMacro(vtkRayBuilder);

vtkRayBuilder::~vtkRayBuilder()
{}

vtkRayBuilder::vtkRayBuilder()
{
  Viewport = NULL;
  float* r;
  for(r=ray; r<ray+6; r++) *r=0;
  ray[5] = -1;
}

/** Returns two (3D) points on the ray that runs, as it were, perpendicular to
 *  the computer monitor and through the point, in the Vtk display, given by
 *  coordinates x and y.
*/
float* vtkRayBuilder::GetRay( int x, int y )
{
    Trace t("vtkRayBuilder::GetRay()");
    if(Viewport == NULL)
    {
        vtkErrorMacro(<<"vtkRayBuilder needs SetViewport to be called");
        for( float * r = ray; r < ray+6; ++r) *r=0;
        ray[5] = -1;
        return ray;
    }
    vtkWindow* window = Viewport->GetVTKWindow(); 
    int* size = window->GetSize();

    double * PointNear = new double[3];
    double * PointFar = new double[3];
    double * temp;

    PointNear[0] = x;              PointFar[0] = x;     
    PointNear[1] = size[1] - y;    PointFar[1] = size[1] - y;
    PointNear[2] = 0.0;            PointFar[2] = 1.0;
  
    Viewport->SetDisplayPoint(PointNear[0],PointNear[1],PointNear[2]);
    Viewport->DisplayToWorld();
    temp = Viewport->GetWorldPoint();
    memcpy( PointNear, temp, 3*sizeof(double) );

    Viewport->SetDisplayPoint(PointFar[0],PointFar[1],PointFar[2]);
    Viewport->DisplayToWorld();
    temp = Viewport->GetWorldPoint();
    memcpy( PointFar, temp, 3*sizeof(double) );

    for( int i=0;i<3;i++ )
    {
        ray[i] = PointNear[i];
        ray[i+3] = PointFar[i];
    }
  
    delete [] PointNear;
    delete [] PointFar;

  return ray;
}
