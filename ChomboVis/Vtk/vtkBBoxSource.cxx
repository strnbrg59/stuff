/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkBBoxSource.cxx 
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
#include <math.h>
#include "vtkBBoxSource.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkObjectFactory.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkBBoxSource);

vtkBBoxSource::vtkBBoxSource(float xMin, float xMax,
                             float yMin, float yMax,
                             float zMin, float zMax)
{
   SetBounds(xMin, xMax,
             yMin, yMax,
             zMin, zMax);
}
void vtkBBoxSource::ExecuteInformation()
{
}

void vtkBBoxSource::Execute()
{
  float x[3];
  int pts[5];
  vtkPoints *newPts;
  vtkCellArray *newLines;
  vtkPolyData *output = this->GetOutput();

  vtkDebugMacro(<<"Creating bbox");
//
// Set things up; allocate memory
//
  newPts = vtkPoints::New();
  newPts->Allocate(8);
  newLines = vtkCellArray::New();
  newLines->Allocate(newLines->EstimateSize(12,2));

//
// Generate points and normals
//
  x[0] = bounds[0]; x[1] = bounds[2]; x[2] = bounds[4];
  newPts->InsertPoint(0,x);
  x[0] = bounds[1]; x[1] = bounds[2]; x[2] = bounds[4];
  newPts->InsertPoint(1,x);
  x[0] = bounds[0]; x[1] = bounds[3]; x[2] = bounds[4];
  newPts->InsertPoint(2,x);
  x[0] = bounds[1]; x[1] = bounds[3]; x[2] = bounds[4];
  newPts->InsertPoint(3,x);
  x[0] = bounds[0]; x[1] = bounds[2]; x[2] = bounds[5];
  newPts->InsertPoint(4,x);
  x[0] = bounds[1]; x[1] = bounds[2]; x[2] = bounds[5];
  newPts->InsertPoint(5,x);
  x[0] = bounds[0]; x[1] = bounds[3]; x[2] = bounds[5];
  newPts->InsertPoint(6,x);
  x[0] = bounds[1]; x[1] = bounds[3]; x[2] = bounds[5];
  newPts->InsertPoint(7,x);

  pts[0] = 0; pts[1] = 1; pts[2] = 3; pts[3] = 2; pts[4] = 0;
  newLines->InsertNextCell(5,pts);

  pts[0] = 4; pts[1] = 5; pts[2] = 7; pts[3] = 6; pts[4] = 4;
  newLines->InsertNextCell(5,pts);

  pts[0] = 0; pts[1] = 4;
  newLines->InsertNextCell(2,pts);
  pts[0] = 1; pts[1] = 5;
  newLines->InsertNextCell(2,pts);
  pts[0] = 2; pts[1] = 6;
  newLines->InsertNextCell(2,pts);
  pts[0] = 3; pts[1] = 7;
  newLines->InsertNextCell(2,pts);
  //
  // Update selves and release memory
  //
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetLines(newLines);
  newLines->Delete();
}

// Convenience method allows creation of cube by specifying bounding box.
void vtkBBoxSource::SetBounds(float xMin, float xMax,
                          float yMin, float yMax,
                      float zMin, float zMax)
{
  bounds[0] = xMin;
  bounds[1] = xMax;
  bounds[2] = yMin;
  bounds[3] = yMax;
  bounds[4] = zMin;
  bounds[5] = zMax;
  Modified();
}

void vtkBBoxSource::SetBounds(float inbounds[6])
{
  for(int i=0; i<6; i++) bounds[i]=inbounds[i];
  Modified();
}

void vtkBBoxSource::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataSource::PrintSelf(os,indent);

  os << indent  << "xmin "<<bounds[0]<<"\n";
  os << indent  << "xmax "<<bounds[1]<<"\n";
  os << indent  << "ymin "<<bounds[2]<<"\n";
  os << indent  << "ymax "<<bounds[3]<<"\n";
  os << indent  << "zmin "<<bounds[4]<<"\n";
  os << indent  << "zmax "<<bounds[5]<<"\n";

}

vtkPolyData * vtkBBoxSource::GetOutput()
{
    return this->vtkPolyDataSource::GetOutput();
}
