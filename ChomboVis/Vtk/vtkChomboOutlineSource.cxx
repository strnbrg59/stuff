/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkChomboOutlineSource.cxx 
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
#include "vtkChomboOutlineSource.h"
#include "vtkChomboReader.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include <cstdio>
#include <cassert>
#include "../utils/Consts.h"
#include "../utils/Trace.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkChomboOutlineSource);

vtkChomboOutlineSource::vtkChomboOutlineSource ()
  : DetailMode( BOXEDGES ),
    SliceCellsDelta(0.01)
{
}

vtkChomboOutlineSource::~vtkChomboOutlineSource ()
{}


/** Generate geometry for a particular piece, i.e. box.
 *  Args padded and real are logically bool.
*/
void vtkChomboOutlineSource::UpdateOutput(
  int level, int boxNum, int padded, int real )
{
    assert( (padded==0) || (padded==1) );
    assert( (real==0) || (real==1) );
    m_curLevel  = level;
    m_curBoxNum = boxNum;
    m_padded    = padded;
    m_real      = real;
    Execute();
}


void vtkChomboOutlineSource::Execute()
{
    Trace t("vtkChomboOutlineSource::Execute()");

    float x[3];
    int pts[2];
    vtkPoints *newPts = 0;
    vtkCellArray *newLines = 0;
    vtkPolyData *output = this->GetOutput();

    vtkDebugMacro(<< "Generating outline");
    //
    // Initialize
    //
    int * tInt = m_reader->GetDataCenteringPermuted();
    int dataCentering[3];
    memcpy( dataCentering, tInt, 3*sizeof(int) );

    int axisNum = 2;
    if( m_reader->IsSlicingMode() )
    {
        char axis = m_reader->GetSlicingDirection();
        axisNum = 0 * (axis=='x')  +  1 * (axis=='y')  +  2 * (axis=='z');
    }

    double spacing[3];
    memcpy( spacing,
            m_reader->GetSpacing( m_curLevel, axisNum ), 3*sizeof(double) );

    double bounds[6];
    memcpy( bounds,
            m_reader->GetBounds(
              m_curLevel, m_curBoxNum, axisNum, m_padded, m_real ),
            6*sizeof(double) );
    for( int i=0;i<3;++i )
    {
        bounds[2*i  ] -= spacing[i]/2.0;
        bounds[2*i+1] -= spacing[i]*( dataCentering[i] - 0.5 );
    }

    int dims[3]; // number of cells along each dimension
    memcpy( dims,
            m_reader->GetDimensions(m_curLevel,m_curBoxNum,axisNum,
                                    m_padded,m_real),
            3*sizeof(int) );
    for( int i=0;i<3;++i ) dims[i] -= dataCentering[i];

    //
    // Allocate storage and create outline
    //
    if( DetailMode == BOXEDGES )
    {
        // Make a cube.
        newPts = vtkPoints::New();
        newPts->Allocate(8);

        newLines = vtkCellArray::New();
        newLines->Allocate(newLines->EstimateSize(12,2));

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

        pts[0] = 0; pts[1] = 1; newLines->InsertNextCell(2,pts);
        pts[0] = 2; pts[1] = 3; newLines->InsertNextCell(2,pts);
        pts[0] = 4; pts[1] = 5; newLines->InsertNextCell(2,pts);
        pts[0] = 6; pts[1] = 7; newLines->InsertNextCell(2,pts);
        pts[0] = 0; pts[1] = 2; newLines->InsertNextCell(2,pts);
        pts[0] = 1; pts[1] = 3; newLines->InsertNextCell(2,pts);
        pts[0] = 4; pts[1] = 6; newLines->InsertNextCell(2,pts);
        pts[0] = 5; pts[1] = 7; newLines->InsertNextCell(2,pts);
        pts[0] = 0; pts[1] = 4; newLines->InsertNextCell(2,pts);
        pts[0] = 1; pts[1] = 5; newLines->InsertNextCell(2,pts);
        pts[0] = 2; pts[1] = 6; newLines->InsertNextCell(2,pts);
        pts[0] = 3; pts[1] = 7; newLines->InsertNextCell(2,pts);
    } else

    if( DetailMode == FACECELLS )
    {
        int size;
        int i,k,cnt;
        int u,v,w;

        size  = 4*(dims[0]+1);
        size += 4*(dims[1]+1);
        size += 4*(dims[2]+1);

        newPts = vtkPoints::New();
        newPts->Allocate(2*size);

        newLines = vtkCellArray::New();
        newLines->Allocate(newLines->EstimateSize(size,2));

        cnt = 0;
        u = 0; v = 1; w = 2;

        for (k = 0; k < 3; k++)
        {
            for (i = 0; i <= dims[u]; i++)
            {
                x[u] = bounds[2*u] + i*spacing[u];
                x[v] = bounds[2*v];
                x[w] = bounds[2*w  ];
                newPts->InsertPoint(cnt,x);

                pts[0] = cnt++;
                x[w] = bounds[2*w+1];
                newPts->InsertPoint(cnt,x);
                pts[1] = cnt++;
                newLines->InsertNextCell(2,pts);

                pts[0] = pts[1];
                x[v] = bounds[2*v+1];
                newPts->InsertPoint(cnt,x);
                pts[1] = cnt++;
                newLines->InsertNextCell(2,pts);

                pts[0] = pts[1];
                x[w] = bounds[2*w  ];
                newPts->InsertPoint(cnt,x);
                pts[1] = cnt++;
                newLines->InsertNextCell(2,pts);

                pts[0] = pts[1];
                pts[1] = pts[1]-3;
                newLines->InsertNextCell(2,pts);
            }
            i = u; u = v; v = w; w = i;
        }
    } else if( DetailMode == ALLCELLS )
    {
        int size;
        int i,j,k,cnt;
        int u,v,w;

        size  = (dims[0]+1)*(dims[1]+1);
        size += (dims[1]+1)*(dims[2]+1);
        size += (dims[2]+1)*(dims[0]+1);

        newPts = vtkPoints::New();
        newPts->Allocate(2*size);

        newLines = vtkCellArray::New();
        newLines->Allocate(newLines->EstimateSize(size,2));

        cnt = 0;
        u = 0; v = 1; w = 2;
        for (k = 0; k < 3; k++)
        {
            for (i = 0; i <= dims[u]; i++)
            {
                x[u] = bounds[2*u] + i*spacing[u];

                for (j = 0; j <= dims[v]; j++)
                {
                    x[v] = bounds[2*v] + j*spacing[v];
                    x[w] = bounds[2*w  ];
                    newPts->InsertPoint(cnt,x);

                    pts[0] = cnt++;
                    x[w] = bounds[2*w+1];
                    newPts->InsertPoint(cnt,x);

                    pts[1] = cnt++;
                    newLines->InsertNextCell(2,pts);
                }
            }
            i=u; // !
            u = v; v = w; w = i;
        }
    } else if( DetailMode == SLICECELLS )
    {
        SliceCellsDriver( output );
        return;
    } else
    {
        t.Error( "Invalid detail mode -- %d --." );
        return;
    }
  
    //
    // Update selves and release memory
    //
    output->SetPoints(newPts);
    newPts->Delete();
    output->SetLines(newLines);
    newLines->Delete();
}


/** Draws cell projections onto a plane.  For meaningful output, you must
 *  call SetPlaneNormal() and SetPlanePosition() before calling this function.
*/
void
vtkChomboOutlineSource::SliceCellsDriver( vtkPolyData * output )
{
    //
    // Fill bounds, spacing and dims.
    //
    int axisNum = 2;
    if( m_reader->IsSlicingMode() )
    {
        char axis = m_reader->GetSlicingDirection();
        axisNum = 0 * (axis=='x')  +  1 * (axis=='y')  +  2 * (axis=='z');
    }

    double spacing[3];
    memcpy( spacing,
            m_reader->GetSpacing( m_curLevel, axisNum ), 3*sizeof(double) );

    int dataCentering[3];
    memcpy( dataCentering, m_reader->GetDataCentering(), 3*sizeof(int) );

    int dims[3];
    memcpy( dims,
            m_reader->GetDimensions(m_curLevel,m_curBoxNum,axisNum,
                                    m_padded,m_real),
            3*sizeof(int) );
    for(int i=0;i<3;i++) dims[i] -= dataCentering[i];

    double bounds[6];
    memcpy( bounds,
            m_reader->GetBounds(
              m_curLevel, m_curBoxNum, axisNum, m_padded, m_real ),
            6*sizeof(double) );

    double adjustedBounds[6];
    for(int i=0;i<3;i++)
    {
        adjustedBounds[i*2  ] = bounds[i*2]   
                              - (spacing[i]/2.0) * (dataCentering[i]==0);
        adjustedBounds[i*2+1] = bounds[i*2+1]
                              + (spacing[i]/2.0) * (dataCentering[i]==0);
    }


    vtkPoints * newPts = 0;
    vtkCellArray * newLines = 0;

    DrawSliceCells( dims, adjustedBounds, spacing, &newPts, &newLines, output );

    output->SetPoints(newPts);
    newPts->Delete();
    output->SetLines(newLines);
    newLines->Delete();
}


void
vtkChomboOutlineSource::DrawSliceCells(
    int /*const*/ * dims,
    double /*const*/ * bounds,
    double /*const*/ * spacing,
    vtkPoints * * newPts,
    vtkCellArray * * newLines,
    vtkPolyData * output )
{
    int pts[2];
    float x[3];

    int axisNum = (m_planeNormal=='y') + 2 * (m_planeNormal=='z');
    int otherAxes[2] = { (axisNum + 1) % 3, (axisNum + 2) % 3 };

    int nPoints = 4 * (dims[otherAxes[0]] + 1
                     + dims[otherAxes[1]] + 1 );
    *newPts = vtkPoints::New();
    (*newPts)->Allocate(nPoints);
    
    *newLines = vtkCellArray::New();
    (*newLines)->Allocate((*newLines)->EstimateSize(nPoints,2));

    int cnt=0;

    // Distance to either side of m_planePosition:
    double delta = GetSliceCellsDelta();

    // We put one grid on either side of the slice at m_planePosition, really
    // close to it.
    for( int side=-1; side<2; side+=2 )
    {
       for( int i=0; i<=dims[otherAxes[0]]; ++i )
       {
           x[otherAxes[1]] = bounds[2*otherAxes[1]];
           x[axisNum] = m_planePosition + side*delta;
           x[otherAxes[0]] = bounds[2*otherAxes[0]] + i*spacing[otherAxes[0]];
           (*newPts)->InsertPoint(cnt,x); pts[0] = cnt++;
           
           x[otherAxes[1]] = bounds[2*otherAxes[1]+1];
           x[otherAxes[0]] = bounds[2*otherAxes[0]] + i*spacing[otherAxes[0]];
           (*newPts)->InsertPoint(cnt,x); pts[1] = cnt++;
   
           (*newLines)->InsertNextCell(2,pts);
       }
   
       for( int k=0; k<=dims[otherAxes[1]]; ++k )
       {
           x[otherAxes[1]] = bounds[2*otherAxes[1]] + k*spacing[otherAxes[1]];
           x[axisNum] = m_planePosition + side*delta;
           x[otherAxes[0]] = bounds[2*otherAxes[0]];
           (*newPts)->InsertPoint(cnt,x); pts[0] = cnt++;
           
           x[otherAxes[1]] = bounds[2*otherAxes[1]] + k*spacing[otherAxes[1]];
           x[otherAxes[0]] = bounds[2*otherAxes[0]+1];
           (*newPts)->InsertPoint(cnt,x); pts[1] = cnt++;
   
           (*newLines)->InsertNextCell(2,pts);
       }
    }
}
