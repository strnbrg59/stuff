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

#include "vtkChomboPlaneSource.h"
#include "vtkPlaneSource.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"

#include "vtkObjectFactory.h"
#include "../utils/Trace.h"
#include <iostream>
#include <cassert>

/**
 * Just like in superclass, except omits annoying (and bogus) error message.
*/
int vtkChomboPlaneSource::UpdatePlane(double v1[3], double v2[3])
{
  // std::cerr << "UpdatePlane()\n";
  // set plane center
  for ( int i=0; i < 3; i++ )
    {
    this->Center[i] = this->Origin[i] + 0.5*(v1[i] + v2[i]);
    }

  // set plane normal
  vtkMath::Cross(v1,v2,this->Normal);
  if ( vtkMath::Normalize(this->Normal) == 0.0 )
    {
    //  vtkErrorMacro(<<"Bad plane coordinate system");
    return 0;
    }
  else
    {
    return 1;
    }
}


/** Exactly the same as in superclass.  But if we don't explicitly define this
 *  function here, then the calls to "this->UpdatePlane()" go to the superclass.
*/
void vtkChomboPlaneSource::Execute()
{
  //std::cerr << "Execute()\n";
  double x[3], tc[2], v1[3], v2[3];
  vtkIdType pts[4];
  int i, j, ii;
  int numPts;
  int numPolys;
  vtkPoints *newPoints; 
  vtkFloatArray *newNormals;
  vtkFloatArray *newTCoords;
  vtkCellArray *newPolys;
  vtkPolyData *output = this->GetOutput();
  
  // Check input
  for ( i=0; i < 3; i++ )
    {
    v1[i] = this->Point1[i] - this->Origin[i];
    v2[i] = this->Point2[i] - this->Origin[i];
    }
  if ( !this->UpdatePlane(v1,v2) )
    {
    return;
    }

  // Set things up; allocate memory
  //
  numPts = (this->XResolution+1) * (this->YResolution+1);
  numPolys = this->XResolution * this->YResolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);
  newNormals = vtkFloatArray::New();
  newNormals->SetNumberOfComponents(3);
  newNormals->Allocate(3*numPts);
  newTCoords = vtkFloatArray::New();
  newTCoords->SetNumberOfComponents(2);
  newTCoords->Allocate(2*numPts);

  newPolys = vtkCellArray::New();
  newPolys->Allocate(newPolys->EstimateSize(numPolys,4));

  // Generate points and point data
  //
  for (numPts=0, i=0; i<(this->YResolution+1); i++)
    {
    tc[1] = (double) i / this->YResolution;
    for (j=0; j<(this->XResolution+1); j++)
      {
      tc[0] = (double) j / this->XResolution;

      for ( ii=0; ii < 3; ii++)
        {
        x[ii] = this->Origin[ii] + tc[0]*v1[ii] + tc[1]*v2[ii];
        }

      newPoints->InsertPoint(numPts,x);
      newTCoords->InsertTuple(numPts,tc);
      newNormals->InsertTuple(numPts++,this->Normal);
      }
    }

  // Generate polygon connectivity
  //
  for (i=0; i<this->YResolution; i++)
    {
    for (j=0; j<this->XResolution; j++)
      {
      pts[0] = j + i*(this->XResolution+1);
      pts[1] = pts[0] + 1;
      pts[2] = pts[0] + this->XResolution + 2;
      pts[3] = pts[0] + this->XResolution + 1;
      newPolys->InsertNextCell(4,pts);
      }
    }

  // Update ourselves and release memory
  //
  output->SetPoints(newPoints);
  newPoints->Delete();

  output->GetPointData()->SetNormals(newNormals);
  newNormals->Delete();

  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();
}


// modifies the normal and origin
/** Exactly the same as in superclass.  But if we don't explicitly define this
 *  function here, then the calls to "this->UpdatePlane()" go to the superclass.
*/
void vtkChomboPlaneSource::SetPoint1(double pnt[3])
{
  if ( this->Point1[0] == pnt[0] && this->Point1[1] == pnt[1] &&
       this->Point1[2] == pnt[2] )
    {
    return; //no change
    }
  else
    {
    int i;
    double v1[3], v2[3];

    for ( i=0; i < 3; i++ )
      {
      this->Point1[i] = pnt[i];
      v1[i] = this->Point1[i] - this->Origin[i];
      v2[i] = this->Point2[i] - this->Origin[i];
      }

    // set plane normal
    this->UpdatePlane(v1,v2);
    this->Modified();
    }
}


/** Exactly the same as in superclass.  But if we don't explicitly define this
 *  function here, then the calls to "this->UpdatePlane()" go to the superclass.
*/
void vtkChomboPlaneSource::SetPoint2(double pnt[3])
{
  if ( this->Point2[0] == pnt[0] && this->Point2[1] == pnt[1] &&
       this->Point2[2] == pnt[2] )
    {
    return; //no change
    }
  else
    {
    int i;
    double v1[3], v2[3];

    for ( i=0; i < 3; i++ )
      {
      this->Point2[i] = pnt[i];
      v1[i] = this->Point1[i] - this->Origin[i];
      v2[i] = this->Point2[i] - this->Origin[i];
      }
    // set plane normal
    this->UpdatePlane(v1,v2);
    this->Modified();
    }
}

void vtkChomboPlaneSource::SetPoint1(double x, double y, double z )
{
    double buf[3];
    buf[0] = x;
    buf[1] = y;
    buf[2] = z;
    SetPoint1( buf );
}
void vtkChomboPlaneSource::SetPoint2(double x, double y, double z )
{
    double buf[3];
    buf[0] = x;
    buf[1] = y;
    buf[2] = z;
    SetPoint2( buf );
}

vtkChomboPlaneSource::vtkChomboPlaneSource() { }
vtkChomboPlaneSource::~vtkChomboPlaneSource() { }

vtkStandardNewMacro(vtkChomboPlaneSource);
