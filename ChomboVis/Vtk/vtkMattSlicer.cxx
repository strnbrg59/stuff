/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkMattSlicer.cxx
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
#include "vtkMattSlicer.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkDataArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkStructuredPoints.h"
#include "vtkObjectFactory.h"
#include <cassert>
#include "../utils/Trace.h"
#ifndef floorf
#define floorf(val) val < 0 ? (int)(val-1) : (int)val
#endif

vtkStandardNewMacro(vtkMattSlicer);

void vtkMattSlicer::Execute() {
    Trace t("vtkMattSlicer::Execute()"); t.NoOp();
    if( m_usingTriangleStrips == 0 )
    {
        ExecuteWithoutTriangleStrips();
    } else
    {
        ExecuteWithTriangleStrips();
    }
}


void vtkMattSlicer::ExecuteWithTriangleStrips()
{
    Trace t("vtkMattSlicer::ExecuteWithTriangleStrips()");
    vtkPoints *newPts;
    vtkPointData *pd,*outpd;
    vtkDataArray *inScalars;
    vtkDataArray *newScalars;
    vtkCellArray *newpolys;
    int precision; // 1=float, 2=double

    //G.Wind changed this from this->Input and this->Output for 
    // vtk 3.1 compliance
    vtkStructuredPoints *input = (vtkStructuredPoints *)(this->Inputs[0]);
    vtkPolyData *output = (vtkPolyData *)(this->Outputs[0]);
    //end changes

    int dim[3],numPts,numCells;
    double ptorig[3],ptspace[3];
    
    pd = input->GetPointData();
    if( pd->GetScalars()->IsA("vtkFloatArray") )
    {
        precision = 1;
    } else
    {
        precision = 2;
    }
    outpd = output->GetPointData();

    if (!pd->GetScalars()) {
      vtkErrorMacro(<<"No input data");
      return;
    }
    
    inScalars = pd->GetScalars();

    input->GetOrigin(ptorig[0],ptorig[1],ptorig[2]);
        // Subtract 0.5 * ptspace[i] from ptorig[i].
    input->GetSpacing(ptspace[0],ptspace[1],ptspace[2]);
    input->GetDimensions(dim);

    int ijk[3];
    float xyz[3];

    xyz[Axis] = Origin[Axis] + Offset; // Artifical offset of the slice.

    float ht;

    if (ptspace[Axis] == 0) {
      ht = 0;
    } else {
      // Normalized height.
      ht = (Origin[Axis] - ptorig[Axis] + 0.5*ptspace[Axis]) / ptspace[Axis];
    }

    ijk[Axis] = (int)(floor(ht)); // nearest grid slice

    // the other two axes
    int Uidx = (Axis + 1) % 3;
    int Vidx = (Axis + 2) % 3; // the other two axes

    float lowscale;
    int ptId;

    if (ht == dim[Axis]) {
      ijk[Axis]--;
    }

    if (ijk[Axis] >= 0 && ijk[Axis] < dim[Axis]) {
      numCells = dim[Vidx] *    dim[Uidx];
      numPts   = dim[Vidx] * 2*(dim[Uidx]+1);

      newPts = vtkPoints::New();
      newPts->SetNumberOfPoints(numPts);

      if( precision == 1 )
      {
          newScalars = vtkFloatArray::New();
          ((vtkFloatArray*)newScalars)->SetNumberOfValues(numPts);
      } else
      {
          newScalars = vtkDoubleArray::New();
          ((vtkDoubleArray*)newScalars)->SetNumberOfValues(numPts);
      }    


      int index;
      
      index = 0;

      xyz[Vidx] = ptorig[Vidx] - 0.5 * ptspace[Vidx];

      for (int ii = 0; ii < dim[Vidx]; ii++) {
        ijk[Vidx] = ii;

        xyz[Uidx] = ptorig[Uidx] - 0.5 * ptspace[Uidx];

        for (int jj = 0; jj <= dim[Uidx]; jj++) {
          if (jj > 0) {
            ijk[Uidx] = jj-1;
          } else {
            ijk[Uidx] = jj;
          }

          ptId = input->ComputePointId(ijk);
          if( precision == 1 )
          {
              lowscale = ((vtkFloatArray*)inScalars)->GetValue(ptId);
              ((vtkFloatArray*)newScalars)->SetValue(index  ,lowscale);
              ((vtkFloatArray*)newScalars)->SetValue(index+1,lowscale);
          } else
          {
              lowscale = ((vtkDoubleArray*)inScalars)->GetValue(ptId);
              ((vtkDoubleArray*)newScalars)->SetValue(index  ,lowscale);
              ((vtkDoubleArray*)newScalars)->SetValue(index+1,lowscale);
          }


          newPts->SetPoint(index, xyz);

          xyz[Vidx] += ptspace[Vidx];
          newPts->SetPoint(index+1,xyz);

          xyz[Vidx] -= ptspace[Vidx];
          xyz[Uidx] += ptspace[Uidx];
          index += 2;
        }

        xyz[Vidx] += ptspace[Vidx];
      }

      // need to set points for one cell above that.
      // and then each needs to be shifted down by a half-cell
      newpolys = vtkCellArray::New();
      newpolys->Allocate(newpolys->EstimateSize(dim[Vidx],2*(dim[Uidx]+1)),
                         newpolys->EstimateSize(dim[Vidx],2*(dim[Uidx]+1))/2);

      int size = 2*(dim[Uidx]+1);

      index = 0;

      for (int ii = 0; ii < dim[Vidx]; ii++) {
        newpolys->InsertNextCell(size);

        for (int jj = 0; jj <= dim[Uidx]; jj++) {  
          newpolys->InsertCellPoint(index++);
          newpolys->InsertCellPoint(index++);
        }
      }

      output->SetStrips(newpolys);
      newpolys->Delete();
      
      output->SetPoints(newPts);
      newPts->Delete();
      
      outpd->SetScalars(newScalars);
      newScalars->Delete();
    }
}


void vtkMattSlicer::ExecuteWithoutTriangleStrips()
{
    Trace t("vtkMattSlicer::ExecuteWithoutTriangleStrips()");
    vtkPoints *newPts;
    vtkPointData *pd, *outpd;
    vtkDataArray *inScalars;
    vtkDataArray *newScalars;
    vtkCellArray *newpolys;
    //G.Wind changed this from this->Input and this->Output for 
    // vtk 3.1 compliance
    vtkStructuredPoints *input=(vtkStructuredPoints *)this->Inputs[0];
    vtkPolyData *output=(vtkPolyData *)this->Outputs[0];
    //end changes

    int dim[3], numPts, numCells;
    int precision; // 1=float, 2=double
    double ptorig[3], ptspace[3];
    
    pd=input->GetPointData();
    outpd=output->GetPointData();
    if (!pd->GetScalars()){
      vtkErrorMacro(<<"No input data");
      return;
    }
    
    if( pd->GetScalars()->IsA("vtkFloatArray") )
    {
        precision = 1;
    } else
    {
        precision = 2;
    }

    inScalars=pd->GetScalars();

    input->GetOrigin(ptorig[0],ptorig[1],ptorig[2]);
        // Subtract 0.5 * ptspace[i] from ptorig[i].
    input->GetSpacing(ptspace[0],ptspace[1],ptspace[2]);
    input->GetDimensions(dim);
    // And extend dimension of slice by 1 each planar axis.
    //t.Info( "dim=[%d,%d,%d]", dim[0], dim[1], dim[2] );

    int ijk[3];
    float xyz[3];

    xyz[Axis]=Origin[Axis]+Offset; // Artifical offset of the slice.

    float ht;

    if (ptspace[Axis] == 0) {
      ht=0;
    } else {
      ht=(Origin[Axis]-ptorig[Axis]+0.5*ptspace[Axis])/ptspace[Axis];
      // Normalized height.
    }

    ijk[Axis]=(int)(floor(ht));          // nearest grid slice

    int Uidx=(Axis+1)%3,  Vidx=(Axis+2)%3; // the other two axes

    float lowscale;
    int ptId;

    if (ht == dim[Axis]) {
      ijk[Axis]--;
    }

    if (ijk[Axis] >= 0 && ijk[Axis] < dim[Axis]) {
      numCells = dim[Uidx]*dim[Vidx];
      numPts   = 4*numCells;

      newPts=vtkPoints::New();
      newPts->SetNumberOfPoints(numPts);

      if( precision == 1 )
      {
          newScalars = vtkFloatArray::New();
          ((vtkFloatArray*)newScalars)->SetNumberOfValues(numPts);
      } else
      {
          newScalars = vtkDoubleArray::New();
          ((vtkDoubleArray*)newScalars)->SetNumberOfValues(numPts);
      }    
      
      for (ijk[Uidx] = 0; ijk[Uidx] < dim[Uidx]; ijk[Uidx]++)
      {
        for (ijk[Vidx] = 0; ijk[Vidx] < dim[Vidx]; ijk[Vidx]++)
        {
          ptId=input->ComputePointId(ijk);

          if( precision == 1 )
          {
              lowscale=((vtkFloatArray*)inScalars)->GetValue(ptId);
              ((vtkFloatArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+0,lowscale);
              ((vtkFloatArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+1,lowscale);
              ((vtkFloatArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+2,lowscale);
              ((vtkFloatArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+3,lowscale);
          } else
          {
              lowscale=((vtkDoubleArray*)inScalars)->GetValue(ptId);
              ((vtkDoubleArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+0,lowscale);
              ((vtkDoubleArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+1,lowscale);
              ((vtkDoubleArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+2,lowscale);
              ((vtkDoubleArray*)newScalars)->SetValue(
                4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+3,lowscale);
          }
        
          xyz[Uidx]=ptorig[Uidx]+ptspace[Uidx]*ijk[Uidx]-0.5*ptspace[Uidx];
          xyz[Vidx]=ptorig[Vidx]+ptspace[Vidx]*ijk[Vidx]-0.5*ptspace[Vidx];
          newPts->SetPoint(4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+0, xyz);
          xyz[Uidx]+=ptspace[Uidx];
          newPts->SetPoint(4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+1, xyz);
          xyz[Vidx]+=ptspace[Vidx];
          newPts->SetPoint(4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+2, xyz);
          xyz[Uidx]-=ptspace[Uidx];
          newPts->SetPoint(4*(ijk[Uidx]+ijk[Vidx]*dim[Uidx])+3, xyz);
        }
      }
      // need to set points for one cell above that.
      // and then each needs to be shifted down by a half-cell
      newpolys=vtkCellArray::New();
      newpolys->Allocate(newpolys->EstimateSize(numCells,4),
                         newpolys->EstimateSize(numCells,4)/2 );
      for (int ii = 0; ii < dim[Vidx]; ii++)
      {
        for (int jj = 0; jj < dim[Uidx]; jj++)
        {
          newpolys->InsertNextCell(4);
          newpolys->InsertCellPoint(4*(jj+ii*dim[Uidx])+0);
          newpolys->InsertCellPoint(4*(jj+ii*dim[Uidx])+1);
          newpolys->InsertCellPoint(4*(jj+ii*dim[Uidx])+2);
          newpolys->InsertCellPoint(4*(jj+ii*dim[Uidx])+3);
        }
      }
      output->SetPolys(newpolys);
      newpolys->Delete();
      
      output->SetPoints(newPts);
      newPts->Delete();
      
      outpd->SetScalars(newScalars);
      newScalars->Delete();
    }
}


void vtkMattSlicer::UseTriangleStrips( int yes_no )
{
    assert( (yes_no==0) || (yes_no==1) );
    m_usingTriangleStrips = yes_no;
}
