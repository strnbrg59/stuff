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
// .NAME vtkAMRStreamSource
// .SECTION Description
// vtkAMRStreamSource is a vector feild  trace generation class that takes
//   an AMR data structure represented by a ChomboReader object, and
//   a set of points that are originating points.
//
//  IntegrationMode refers to the manner of discretely advancing a
//  fiedline along a seed point.  0 fixed steps
//                                1 fixed steps clipped for vector
//                                  magnitude change
//
//                                2 Richardson extrapolation (3 point)
//
// .SECTION See Also
// vtkPolyDataSource vtkChomboReader vtkPoints

#ifndef __vtkAMRStreamSource_h
#define __vtkAMRStreamSource_h

#include <vtkPolyDataSource.h>
#include "VTKChomboConfigure.h" // Include configuration header.

class vtkPoints;
class vtkChomboReader;
class vtkCellArray;

class VTK_VTKChombo_EXPORT vtkAMRStreamSource : public vtkPolyDataSource 
{
public:

    //
    // Ctors, dtor
    //
    vtkAMRStreamSource();
    vtkAMRStreamSource(int);
    static vtkAMRStreamSource *New();
    virtual ~vtkAMRStreamSource();


    //
    // Overridden VTK methods.
    //
    vtkTypeMacro(vtkAMRStreamSource,vtkPolyDataSource);
    virtual void PrintSelf(ostream& os, vtkIndent indent)
        { m_pimpl->PrintSelf( os, indent ); }

    virtual vtkPolyData * GetOutput() 
        { return m_pimpl->GetOutput(); }

    virtual void Execute() { m_pimpl->Execute(); }
    virtual void Modified() { m_pimpl->Modified(); }
    virtual unsigned long GetMTime() { return m_pimpl->GetMTime(); }


    //
    // Interface for Python layer (vtk_stream.py).
    //  
    void SetChomboReader( vtkChomboReader * );
    virtual void SetUComponent( int b ) { m_pimpl->SetUComponent(b); }    
    virtual void SetVComponent( int b ) { m_pimpl->SetVComponent(b); }    
    virtual void SetWComponent( int b ) { m_pimpl->SetWComponent(b); }    

    virtual void SetMaxLevel( int i) { m_pimpl->SetMaxLevel(i); }
    virtual void SetMaxPointsPerLine( int i) { m_pimpl->SetMaxPointsPerLine(i);}
    virtual void SetFieldThreshold( double a )
        { m_pimpl->SetFieldThreshold(a); }
    virtual void SetRelativeStepSize( double a )
        { m_pimpl->SetRelativeStepSize(a); }
  
    // Valid value to this are 0, 1 or 2.  FIXME: use an enum.
    virtual void SetIntegrationMode( int m ) { m_pimpl->SetIntegrationMode(m); }

    virtual void SetForward( int m ) { m_pimpl->SetForward(m); }
    virtual void SetBackward( int m ) { m_pimpl->SetBackward(m); }
    virtual void SetSeedPoints( vtkPoints * vp ) { m_pimpl->SetSeedPoints(vp); }
  
  private:
  
    virtual void BuildFieldLine(double current[3], int dim, int gain, 
                                vtkPoints* points,
                                vtkCellArray* lines)
        { m_pimpl->BuildFieldLine( current, dim, gain, points, lines ); }

    vtkAMRStreamSource * m_pimpl;

    // Deliberately unimplemented:
    vtkAMRStreamSource(const vtkAMRStreamSource&);
    vtkAMRStreamSource&  operator=(const vtkAMRStreamSource&);
};

#endif
