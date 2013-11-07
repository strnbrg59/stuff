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

//
// Python's interface to information and control over particle data.
//

#ifndef INCLUDED_VTKCHOMBOPARTICLES_H
#define INCLUDED_VTKCHOMBOPARTICLES_H

class vtkChomboReader;
class vtkPolyData;
#include "vtkObject.h"
#include "VTKChomboConfigure.h" // Include configuration header.

class VTK_VTKChombo_EXPORT vtkChomboParticles : public vtkObject
{
  public:

    //
    // Ctors, dtor
    //
    vtkChomboParticles();
    vtkChomboParticles(int);
    static vtkChomboParticles *New();
    virtual ~vtkChomboParticles();
    vtkTypeMacro(vtkChomboParticles,vtkObject);

    //
    // Overridden VTK methods.
    //
    virtual void PrintSelf(ostream& os, vtkIndent indent)
        { m_pimpl->PrintSelf( os, indent ); }
    const char * GetClassName() { return "vtkChomboParticles"; }


    //
    // Interface for Python layer (vtk_stream.py).
    //  
    void SetChomboReader( vtkChomboReader * );
    virtual vtkPolyData * GetParticleOutput()
        { return m_pimpl->GetParticleOutput(); }

    virtual const char * GetXComponent() const
        { return m_pimpl->GetXComponent(); }
    virtual const char * GetYComponent() const
        { return m_pimpl->GetYComponent(); }
    virtual const char * GetZComponent() const
        { return m_pimpl->GetZComponent(); }
    virtual void SetXComponent( const char * name )
        { m_pimpl->SetXComponent( name ); }
    virtual void SetYComponent( const char * name )
        { m_pimpl->SetYComponent( name ); }
    virtual void SetZComponent( const char * name )
        { m_pimpl->SetZComponent( name ); }

    virtual const char * GetXGlyphOrientationComponent() const
        { return m_pimpl->GetXGlyphOrientationComponent(); }
    virtual const char * GetYGlyphOrientationComponent() const
        { return m_pimpl->GetYGlyphOrientationComponent(); }
    virtual const char * GetZGlyphOrientationComponent() const
        { return m_pimpl->GetZGlyphOrientationComponent(); }
    virtual void SetXGlyphOrientationComponent( const char * name )
        { m_pimpl->SetXGlyphOrientationComponent( name ); }
    virtual void SetYGlyphOrientationComponent( const char * name )
        { m_pimpl->SetYGlyphOrientationComponent( name ); }
    virtual void SetZGlyphOrientationComponent( const char * name )
        { m_pimpl->SetZGlyphOrientationComponent( name ); }

    virtual void SetGlyphScalingComponent( const char * name )
        { m_pimpl->SetGlyphScalingComponent( name ); }
    virtual void SetDoScaleGlyphs( int yes_no )
        { m_pimpl->SetDoScaleGlyphs( yes_no ); }
    virtual double GetGlyphScalingComponentMin() const
        { return m_pimpl->GetGlyphScalingComponentMin(); }
    virtual double GetGlyphScalingComponentMax() const
        { return m_pimpl->GetGlyphScalingComponentMax(); }

    virtual void SetDecimationFactor( double x )
        { m_pimpl->SetDecimationFactor( x ); }

    virtual const char * GetFilteringComponent() const
        { return m_pimpl->GetFilteringComponent(); }
    virtual double GetFilterMin() const
        { return m_pimpl->GetFilterMin(); }
    virtual double GetFilterMax() const
        { return m_pimpl->GetFilterMax(); }
    virtual void SetFilteringComponent( const char * name )
        { m_pimpl->SetFilteringComponent( name ); }
    virtual void SetFilterMinMax( double lo, double hi )
        { m_pimpl->SetFilterMinMax( lo, hi ); }

    virtual const char * GetOffsetFilteringComponent() const
        { return m_pimpl->GetOffsetFilteringComponent(); }
    virtual double GetOffsetFilterMin() const
        { return m_pimpl->GetOffsetFilterMin(); }
    virtual double GetOffsetFilterMax() const
        { return m_pimpl->GetOffsetFilterMax(); }
    virtual void SetOffsetFilteringComponent( const char * name )
        { m_pimpl->SetOffsetFilteringComponent( name ); }
    virtual void SetOffsetFilterMinMax( double lo, double hi )
        { m_pimpl->SetOffsetFilterMinMax( lo, hi ); }
    virtual void SetOffsetFilterOffset( double x )
        { m_pimpl->SetOffsetFilterOffset( x ); }

    virtual void SetMarkersArePoints( int yesno )
        { m_pimpl->SetMarkersArePoints( yesno ); }
    virtual void SetDoOrientGlyphs( int yesno )
        { m_pimpl->SetDoOrientGlyphs( yesno ); }

    virtual int GetNumberOfSelectedParticles() const
        { return m_pimpl->GetNumberOfSelectedParticles(); }
    virtual double * GetParticleXYZCoordinates( int particle_num ) const
        { return m_pimpl->GetParticleXYZCoordinates( particle_num ); }
    virtual double GetScalingScalarValue( int particle_num ) const
        { return m_pimpl->GetScalingScalarValue( particle_num ); }
    virtual int GetSelectedParticleNumber( int n ) const
        { return m_pimpl->GetSelectedParticleNumber( n ); }
  private:
  
    vtkChomboParticles * m_pimpl;

    // Deliberately unimplemented:
    vtkChomboParticles(const vtkChomboParticles&);
    vtkChomboParticles&  operator=(const vtkChomboParticles&);
};

#endif // INCLUDED_VTKCHOMBOPARTICLES_H
