/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkChomboAccumulatePolyData.cxx 
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
#include "vtkChomboAccumulatePolyData.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

#include <assert.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkChomboAccumulatePolyData);

//----------------------------------------------------------------------------
vtkChomboAccumulatePolyData::vtkChomboAccumulatePolyData()
{
  this->AppendFilter=0;
}

//----------------------------------------------------------------------------
vtkChomboAccumulatePolyData::~vtkChomboAccumulatePolyData()
{
    // Odd that this test wasn't necessary in the Tcl version...
    if( this->AppendFilter )
    {
        this->AppendFilter->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkChomboAccumulatePolyData::StreamExecuteStart()
{
  //char const * const fn_name = "vtkChomboAccumulatePolyData::StreamExecuteStart()";
  if(this->AppendFilter) {
    this->AppendFilter->Delete();
  }
  this->AppendFilter = vtkAppendPolyData::New();
  if( ! this->AppendFilter ) fprintf( stderr, "this->AppendFilter is NULL\n" );
  //this->AppendFilter->GetOutput()->DebugOn();
}

//----------------------------------------------------------------------------
void vtkChomboAccumulatePolyData::StreamExecuteEnd()
{
  vtkAppendPolyData *appendfilter = this->AppendFilter;
  this->AppendFilter=NULL;
  vtkPolyData *results = appendfilter->GetOutput();
  vtkPolyData *output = this->GetOutput();
  
  results->Update();

  output->CopyStructure(results);
  output->GetPointData()->PassData(results->GetPointData());
  output->GetCellData()->PassData(results->GetCellData());
  appendfilter->Delete();
}

//----------------------------------------------------------------------------
// Append data sets into single PolyData
void vtkChomboAccumulatePolyData::Append()
{
  if(!this->AppendFilter) return; // nothing to do (maybe pass update to filter parent)
  
  vtkPolyData *input = this->GetInput();
  vtkPolyData *copy;

  copy = vtkPolyData::New();
  
  copy->CopyStructure(input);
  
  copy->GetPointData()->PassData(input->GetPointData());
  copy->GetCellData()->PassData(input->GetCellData());
  
  if(this->AppendFilter)
    this->AppendFilter->AddInput(copy);
  copy->Delete();  //drops refcount by one.

}

void vtkChomboAccumulatePolyData::Execute()
{
  //trust nobody for Execute commands, break pipelines here.
  // printf("vtkChomboAccumulatePolyData::Execute\n");
}

void vtkChomboAccumulatePolyData::Update()
{
  // printf("vtkChomboAccumulatePolyData::Update\n");
  //trust nobody for Update commands, break pipelines here.
}

void vtkChomboAccumulatePolyData::UpdateData(vtkDataObject* notUsed)
{
  // printf("vtkChomboAccumulatePolyData::UpdateData\n");
}

//----------------------------------------------------------------------------
void vtkChomboAccumulatePolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);
  fprintf(stderr,"++++\n");
  this->GetOutput()->PrintSelf(os,indent);
  fprintf(stderr,"---\n");
}
