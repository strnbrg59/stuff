/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkChomboReader.cxx  
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

/**
 * Structure of this file:
 * #include "vtkChomboReader.h", the public interface.
 * #include "vtkChomboReaderImpl_h", the internal (to this file) interface.
 * Implementation of vtkChomboReader, a pimpl class.
 * #include "vtkChomboReaderImpl_cxx", the implementation.
*/

#include "../data_access/Intvect.h"
#include "../data_access/FAB.h"
#include "vtkChomboReader.h"
#include "../VtkImpl/vtkChomboReaderImpl_h"
#include "vtkObjectFactory.h"

/** Nothing much happens until the first call to LoadFile().
 *  A single vtkChomboReader persists throughout a ChomboVis session.  But we
 *  construct a new m_pimpl every time we load a new HDF5 file.
 */
vtkChomboReader::vtkChomboReader()
  : m_numDims(0),
    m_precision(0),
    m_pimpl(0)
{
    Trace t("vtkChomboReader::vtkChomboReader()"); t.NoOp();
}


vtkStandardNewMacro(vtkChomboReader);

vtkChomboReader::~vtkChomboReader()
{
    if( m_pimpl )
    {
        // m_pimpl is NULL when it's a field of the parent-class part of a
        // vtkChomboReaderImpl.  Huh?  OK.  Every vtkChomboReader has a member
        // of type vtkChomboReaderImpl*.  If it's a vtkChomboReader we want to
        // use through Python, then its Impl member gets allocated and, as we
        // know, then does all the heavy lifting.  But the Impl object has
        // vtkChomboReader for its parent class, but that Impl object has no use
        // for an Impl* member attached to its parent part and indeed the
        // Impl* member of that parent section is left at its default, which is
        // NULL (see the vtkChomboReader ctor).
        m_pimpl->Delete();
        // If, instead, we say "delete m_pimpl", then we get the error message
        // "Trying to delete object with non-zero reference count."
    }
/*
    else
    {
        cerr << "m_pimpl == NULL" << endl;
        // This prints every time.  See comments immediately above for why that
        // happens.
    }
*/
}


/** Load a new HDF5 file.
 *  Init m_pimpl (the private vtkChomboReaderImpl<> member), according to the
 *  precision and dimensionality of the indicated hdf5 file.
 *
 *  Returns 0 if ok, 1 on error.
*/
int
vtkChomboReader::LoadFile( char const * filename )
{
    Trace t("vtkChomboReader::LoadFile()");

    Status status;
    ChomboHDF5FileType filetype;
    status = ChomboHDF5DiscoverMetaparameters(
                    filename, &m_numDims, &m_precision, &filetype );
    if( status != STATUS_OK )
    {
        t.Error( "%s", StatusName( status ).c_str() );
        return 1;
    }

    if( m_precision == 1 )
    {
        m_pimpl = new vtkChomboReaderImpl<float>(filename, m_numDims, &status);
        // Note about object destruction.
        // vtkChomboReaderImpl is, through vtkChomboReader, a subclass of
        // vtkObject.  vtkObject is reference-counted and vtkChomboReaderImpl
        // is destructed by vtkObject::Delete().  That happens when, in
        // vtk_data.py, we say "self.reader = 0".
    } else
    {
        m_pimpl = new vtkChomboReaderImpl<double>(filename, m_numDims, &status);
    }

    if( status != STATUS_OK )
    {
        t.Error( "%s", StatusName( status ).c_str() );
        return 1;
    }

    return 0;
}


/** Analogous to LoadFile(), but for sharing a VisualizableDataset that another
 *  vtkChomboReader has already loaded.  Used when initializing LocalVtkData
 *  objects in vtk_data.py.
*/
int
vtkChomboReader::ShareFile( vtkChomboReader * that )
{
    Trace t("vtkChomboReader::ShareFile()");

    Status status;

    m_precision = that->GetPrecision();
    m_numDims = that->GetNumDims();

    if( m_precision == 1 )
    {
        m_pimpl = new vtkChomboReaderImpl<float>(
                that->m_pimpl, m_numDims, &status );
    } else
    {
        m_pimpl = new vtkChomboReaderImpl<double>(
                that->m_pimpl, m_numDims, &status );
    }

    if( status != STATUS_OK )
    {
        t.Error( "%s", StatusName( status ).c_str() );
        return 1;
    }

    return 0;
}

#include "../VtkImpl/vtkChomboReaderImpl_cxx"
