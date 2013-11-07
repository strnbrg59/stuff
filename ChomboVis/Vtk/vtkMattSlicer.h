
/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkMattSlicer.h  
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

  Thanks:    Thanks to Matt Hall (mahall    math.uiuc.edu) who developed this class.
             http://zeus.ncsa.uiuc.edu/~mahall

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
// .NAME vtkMattSlicer - Matt Hall's quicker slicer
// .SECTION Description
// vtkMattSlicer is an orthogonal slicer specialized to StructuredPoints
// datasets with implicitly cell-centered data.
//        http://zeus.ncsa.uiuc.edu/~mahall
// .SECTION See Also
// vtkStructuredPointsToPolyDataFilter

#ifndef __vtkMattSlicer_h
#define __vtkMattSlicer_h

#include <vtkStructuredPointsToPolyDataFilter.h>
#include "VTKChomboConfigure.h"

class VTK_VTKChombo_EXPORT vtkMattSlicer :
    public vtkStructuredPointsToPolyDataFilter
{
  float Origin[3];
  int Axis;
  float Offset;
public: 
  vtkMattSlicer() {
    Origin[0]=Origin[1]=Origin[2]=0;
    Axis=1;
    Offset=0.0;
    m_usingTriangleStrips = 1;
  };
  static vtkMattSlicer *New();
  vtkTypeMacro(vtkMattSlicer,vtkStructuredPointsToPolyDataFilter);
  const char *GetClassName(){return "vtkMattSlicer";}
  void PrintSelf(ostream &os, vtkIndent indent){};
  vtkSetVector3Macro(Origin, float);
  vtkSetMacro(Axis, int);
  vtkSetMacro(Offset, float);
  void SetX(float ff){Origin[0]=ff;Modified();}
  void SetY(float ff){Origin[1]=ff;Modified();}
  void SetZ(float ff){Origin[2]=ff;Modified();}
  void SetAxis(int ii, float ff){Origin[ii]=ff;Modified();}

  void UseTriangleStrips( int yes_no );
  void Execute();

protected:
  void ExecuteWithTriangleStrips();
  void ExecuteWithoutTriangleStrips();

private:
  int m_usingTriangleStrips;
    // Triangle strips are much more efficient, but come out looking wrong
    // when subjected to a vtkClipPolyData.  So vtk_slice sets this variable
    // to 0, when it requests clipping.
};

#endif
