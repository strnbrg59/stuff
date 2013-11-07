/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkChomboOutlineSource.h  
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
// .NAME vtkChomboOutlineSource - create wireframe outline for arbitrary data set
// .SECTION Description
// vtkChomboOutlineSource generates a wireframe outline of any 
// data set. The outline consists of the twelve edges of the dataset 
// bounding box.  This has been modified from the original vtkOutlineFilter.

#ifndef __vtkChomboOutlineSource_h
#define __vtkChomboOutlineSource_h

#include "vtkPolyDataSource.h"
//#include "vtkStructuredPoints.h"
//#include "vtkStructuredPointsToPolyDataFilter.h"
#include "VTKChomboConfigure.h"

class vtkPoints;
class vtkCellArray;
class vtkChomboReader;

class VTK_VTKChombo_EXPORT vtkChomboOutlineSource : public vtkPolyDataSource
{
public:
  static vtkChomboOutlineSource *New();
  void SetChomboReader( vtkChomboReader * r ) { m_reader = r; }
  vtkTypeMacro(vtkChomboOutlineSource,vtkPolyDataSource);
  
  vtkSetMacro(DetailMode,int);
  vtkSetMacro(SliceCellsDelta,double);
  vtkGetMacro(SliceCellsDelta,double);
  void UpdateOutput( int level, int boxNum, int padded, int real );

  float GetPlanePosition() const { return m_planePosition; }
  void  SetPlanePosition( float x ) { m_planePosition = x; }
  char  GetPlaneNormal() const { return m_planeNormal; }
  void  SetPlaneNormal( char c ) { m_planeNormal = c; }

  void SliceCellsDriver( vtkPolyData * output );

private:
  vtkChomboOutlineSource();
  ~vtkChomboOutlineSource();
  vtkChomboOutlineSource(const vtkChomboOutlineSource&) {};
  void operator=(const vtkChomboOutlineSource&) {};

  void Execute();

  // These are for the gridded plane:
  float m_planePosition;
  char  m_planeNormal;    // 'x', 'y' or 'z'
  void DrawSliceCells( int /*const*/ * dims,
                       double /*const*/ * bounds,
                       double /*const*/ * spacing,
                       vtkPoints * * newPts,
                       vtkCellArray * * newLines,
                       vtkPolyData * output );

  int DetailMode;
  double SliceCellsDelta;
  vtkChomboReader * m_reader;
  int m_curLevel;
  int m_curBoxNum;
  int m_padded;
  int m_real;
};

// VTK's Python wrapper thinks enums are a syntax error.
#define BOXEDGES   1
#define FACECELLS  2
#define ALLCELLS   3
#define SLICECELLS 4
#define SOLIDBOXES 5

#endif
