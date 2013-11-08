
/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkAMRVolume.h  
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
// .NAME vtkAMRVolume
// .SECTION Description
// vtkAMRVolume is a 
// .SECTION See Also
// vtkChomboReader

#ifndef __vtkAMRVolume_h
#define __vtkAMRVolume_h

#include <vtkObject.h>
#include "VTKChomboConfigure.h" // Include configuration header.

class vtkVolumeRayCastFunction;
class vtkChomboReader;
class vtkVolumeProperty;
class vtkRenderer;

class VTK_VTKChombo_EXPORT vtkAMRVolume : public vtkObject 
{
public:
    vtkAMRVolume();
    vtkAMRVolume(int);
    virtual ~vtkAMRVolume();

    int * foo( int m, int n, int * i );

    vtkTypeMacro(vtkAMRVolume, vtkObject);
    virtual void PrintSelf(ostream& os, vtkIndent indent)
        { m_pimpl->PrintSelf( os, indent ); }
    static vtkAMRVolume *New();

    void SetChomboReader( vtkChomboReader * );

    virtual vtkChomboReader * GetChomboReader() const
        { return m_pimpl->GetChomboReader(); }

    virtual void SetRenderer( vtkRenderer * r )
        { m_pimpl->SetRenderer( r ); }
    virtual vtkRenderer * GetRenderer() const
        { return m_pimpl->GetRenderer(); }

    virtual void SetRayCastFunction( vtkVolumeRayCastFunction * r )
        { m_pimpl->SetRayCastFunction( r ); }
    virtual vtkVolumeRayCastFunction * GetRayCastFunction() const
        { return m_pimpl->GetRayCastFunction(); }

    virtual void SetProperty( vtkVolumeProperty * r )
        { m_pimpl->SetProperty( r ); }
    virtual vtkVolumeProperty * GetProperty() const
        { return m_pimpl->GetProperty(); }

    virtual void SetComponentNum( int s ) { m_pimpl->SetComponentNum( s ); }
    virtual int GetComponentNum() const { return m_pimpl->GetComponentNum(); }  

    virtual void SetMinScalarValue( float s )
        { m_pimpl->SetMinScalarValue( s ); }
    virtual float GetMinScalarValue() const
        { return m_pimpl->GetMinScalarValue(); }

    virtual void SetMaxScalarValue( float s )
        { m_pimpl->SetMaxScalarValue( s ); }
    virtual float GetMaxScalarValue() const
        { return m_pimpl->GetMaxScalarValue(); }

    virtual void SetMaxLevel( int s ) { m_pimpl->SetMaxLevel( s ); }
    virtual int GetMaxLevel() const { return m_pimpl->GetMaxLevel(); }    

    virtual void SetMinLevel( int s ) { m_pimpl->SetMinLevel( s ); }
    virtual int GetMinLevel() const { return m_pimpl->GetMinLevel(); }    

    virtual void SetSampleDistance( float s )
        { m_pimpl->SetSampleDistance( s ); }
    virtual float GetSampleDistance() const
        { return m_pimpl->GetSampleDistance(); }

    virtual void ClearVolumes() { m_pimpl->ClearVolumes(); }
    virtual void BuildVolumes() { m_pimpl->BuildVolumes(); }
    virtual void AddVolumesToRenderer() { m_pimpl->AddVolumesToRenderer(); }
    virtual void RemoveVolumesFromRenderer()
        { m_pimpl->RemoveVolumesFromRenderer(); }
    virtual void Modified() { m_pimpl->Modified(); }

  private:
    vtkAMRVolume * m_pimpl;

    // Deliberately unimplemented:
    vtkAMRVolume(const vtkAMRVolume&);
    vtkAMRVolume&  operator=(const vtkAMRVolume&);

};

#endif