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
** Author: Ted Sternberg
** Date:   August 1, 2003
*/

/**
 * This file exposes some of the functionality of the VisualizableDataset to
 *   the Python interpreter.  Compiled, it constitutes libVisDat.so.
 * The general rule for using these functions is, the first argument is
 *   the int handle (in a PointerHandleMap) to a pointer to a
 *   VisualizableDataset.
 * Only one .py module -- visualizable_dataset.py -- should ever call these
 *   functions directly.  visualizable_dataset.py then exposes an object-
 *   oriented view of this stuff.
*/

#include "module_templates.cpp"
#include "../VisualizableDataset.h"
#include "NewComponentGenerator.h"
#include "../PointerHandleMap.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
using std::cerr;
using std::endl;
using std::vector;
using std::pair;
using std::string;


/** Construct a VisualizableDataset from an hdf5 file.  Return an int handle
 *    to the address.
 *  Usage: >>> ptr = libVisDat.NewVisualizableDataset( hdf5filename)
*/
static PyObject *
visdat_NewVisualizableDataset( PyObject * self, PyObject * args )
{
    char * hdf5filename;
    if( ! PyArg_ParseTuple( args, "s", &hdf5filename ) )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "PyArg_ParseTuple failed." );
        return 0;
    }

    int floatPrecision;  // 1=>float, 2=>double.    
    int dimensionality;  // Not used here.
    ChomboHDF5FileType filetype;
    ChomboHDF5DiscoverMetaparameters(
        hdf5filename, &dimensionality, &floatPrecision, &filetype );

    boost::shared_ptr<VisualizableDatasetInterfaceForPython> pVisDat;
    if( floatPrecision == 1 )
    {
        pVisDat.reset( new VisualizableDataset<float>( hdf5filename ) );
    } else
    {
        pVisDat.reset( new VisualizableDataset<double>( hdf5filename ) );
    }

    PyObject * result = PyInt_FromLong( SharedPointerHandleMap<
        VisualizableDatasetInterfaceForPython>::GetHandle( pVisDat ));
    return result;
}


/** Usage: >>> libVisDat.Release( visdat_ptr )
 *  where visdat_ptr is a handle to a VisualizableDataset* (see "general rule"
 *  above.
 *
 *  Removes visdat_ptr (a shared_ptr) from the SharedPointerHandleMap, allowing
 *  it to be destructed (if it's actually not in use anywhere else).
 *
 *  WARNING: Never call this, except from the Python class VisualizableDataset's
 *  __del__() function.
*/
static PyObject *
visdat_Release( PyObject * self, PyObject *args )
{
    int intPtr;
    if( ! PyArg_ParseTuple( args, "i", &intPtr ) )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "PyArg_ParseTuple failed." );
        return 0;
    }

    boost::shared_ptr<VisualizableDatasetInterfaceForPython> p(
        SharedPointerHandleMap<
            VisualizableDatasetInterfaceForPython>::GetPointer( intPtr ) );
    if( p.use_count() == 3 ) // itself and the two in the map (a
                             // SemiSymmetricMap holds two copies of everything)
    {
        SharedPointerHandleMap<
            VisualizableDatasetInterfaceForPython>::RemoveByHandle( intPtr );
    }
    Py_INCREF( Py_None );
    return Py_None;
}


/** Usage: >>> libVisDat.SaveToHDF5( visdat_ptr, outfilename, 0 )
 *  where visdat_ptr is an int, and outfilename is a string.
*/
static PyObject *
visdat_SaveToHDF5( PyObject * self, PyObject * args )
{
    assert( args );
    return SharedDiadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::SaveToHDF5, args );
}


/** Create a sliced VisualizableDataset and return a handle to its address.
 *  Usage: >>> ptr = libVisDat.Slice( visdatPtr, axis, axisPosition )
 *  where visdatPtr is a handle to a VisualizableDatasetInterfaceForPython*.
 *  axis is 'x', 'y' or 'z'; and
 *  axisPosition, a real, is the point along the axis where you want the slice.
*/
static PyObject *
visdat_Slice( PyObject * self, PyObject * args )
{
    return SharedDiadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::Slice, args );
}


/** Usage: >>> libVisDat.CropToGeneralBox( visdatPtr, box, cropping_levels,
 *                                                    cropping_components )
 *  where visdatPtr is a handle to a VisualizableDataset*,
 *  box is any rectangular prism, expressed as a box.Box,
 *  cropping_levels is a tuple of consecutive ints (the levels you want to
 *  retain, but the empty tuple '()' is interpreted as "save all the levels"),
 *  and cropping_components is a tuple of component names that follows the rules
 *  of cropping_levels.
 *  Returns a handle to a new VisualizableDataset.
*/
static PyObject *
visdat_CropToGeneralBox( PyObject * self, PyObject *args )
{
    return SharedTriadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::CropToGeneralBox, args );
}


/** Implementation details for visdat_DefineNewComponent and
 *  visdat_RedefineNewComponent.
 *  Arg redefine should be 0 if we want to call DefineNewComponent, 1 if we
 *  want to call RedefineNewComponent.
*/
static PyObject *
local_DefineNewComponent( PyObject * self, PyObject *args, int redefine )
{
    int visdatPtr;       // handle to VisualizableDataset *
    char * newComponentName;
    PyObject * function;
    PyObject * argNamesOrNums; // tuple of strings or integers
    if( ! PyArg_ParseTuple( args, "isOO",
                            &visdatPtr, &newComponentName,
                            &function, &argNamesOrNums ) )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "PyArg_ParseTuple failed." );
        return 0;
    }

    boost::shared_ptr<VisualizableDatasetInterfaceForPython> pVisDat = 
        SharedPointerHandleMap<
            VisualizableDatasetInterfaceForPython>::GetPointer( visdatPtr );

    //
    // Argument validity check.
    //
    if( !PyCallable_Check( function ) )
    {
        PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
        return 0;
    }
    if( ! PyTuple_Check( argNamesOrNums ) )
    {
        PyErr_SetString( PyExc_TypeError,
                        "Fourth argument should be, but isn't, a tuple." );
        return 0;
    }


    VectorFunctorBase * generator(0);
    if( pVisDat->GetPrecision() == 1 )
    {
        generator = new NewComponentGenerator<float>( function );
    } else
    {
        generator = new NewComponentGenerator<double>( function );
    }


    //
    // Figure out if argNamesOrNums is a tuple of strings or ints, and call
    // appropriate overloaded version of
    // VisualizableDataset::DefineNewComponent().
    //
    int retval;
    int nArgs = PyTuple_Size(argNamesOrNums);
    PyObject * arg0 = PyTuple_GetItem(argNamesOrNums,0);
    if( PyString_Check( arg0 ) )
    {
        vector<string> vArgNames;
        vArgNames.reserve( nArgs );

        for( int i=0; i<nArgs; ++i )
        {
            vArgNames.push_back(
                PyString_AsString(PyTuple_GetItem(argNamesOrNums,i)) );
        }
        if( redefine == 0 )
        {
            retval = pVisDat->DefineNewComponent(
                newComponentName,generator,vArgNames);
        } else
        {
            retval = pVisDat->RedefineNewComponent(
                newComponentName,generator,vArgNames);
        }
    } else
    if( PyInt_Check( arg0 ) )
    {
        vector<int> vArgNums;
        vArgNums.reserve( nArgs );

        for( int i=0; i<nArgs; ++i )
        {
            vArgNums.push_back(
                PyInt_AsLong(PyTuple_GetItem(argNamesOrNums,i)) );
        }
        if( redefine == 0 )
        {
            retval = pVisDat->DefineNewComponent(
                newComponentName,generator,vArgNums);
        } else
        {
            retval = pVisDat->RedefineNewComponent(
                newComponentName,generator,vArgNums);
        }
    } else
    {
        PyErr_SetString(PyExc_RuntimeError, "(Re)DefineNewComponent takes arg "
            "names or arg nums, but not other types." );
        return 0;
    }
            

    if( retval )
    {
        PyErr_SetString(PyExc_RuntimeError,
                      "VisualizableDataset::(Re)DefineNewComponent() failed." );
        return 0;
    }

    Py_INCREF( Py_None );
    return Py_None;
}


/** Usage: >>> libVisDat.DefineNewComponent( visdat_ptr,
                                             new_component_name,
 *                                           function, (argname1, argname2,...))
 *                                   [or]       "      (argnum1, argnum2,...))
 *  where visdat_ptr is a handle to a VisualizableDataset*;
 *  where function is a real function of as many variables as elements in the
 *      last argument;
 *  where the last argument is a tuple of names, or serial numbers of existing
 *      components of the VisualizableDataset, which are to be arguments of the
 *      function.  So you can specify the arguments by name, or by number (but
 *      not by a combination of names and numbers!)
*/
static PyObject *
visdat_DefineNewComponent( PyObject * self, PyObject *args )
{
    return local_DefineNewComponent( self, args, 0 );
}


/** See comments for visdat_DefineNewComponent(). */
static PyObject *
visdat_RedefineNewComponent( PyObject * self, PyObject *args )
{
    return local_DefineNewComponent( self, args, 1 );
}


/** Usage: >>> libVisDat.SetDebugLevel( visdatPtr, level ) */
static PyObject *
visdat_SetDebugLevel( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::SetDebugLevel, args);
}


/** Usage: >>> libVisDat.GetBoxLayout( visdatPtr, level, real )
 *  Returns the box corners as a tuple of tuples of tuples.  Does not return
 *  any information on dx or origin.
*/
static PyObject *
visdat_GetBoxLayout( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetBoxLayoutAsNestedVectors,
        args);
}


/** Return a handle to the address of one of the BoxLayoutData objects.
 *  Usage: >>> ptr = libVisDat.GetBoxLayoutData( visdatPtr, level, component,
                                                 real, contrapad )
 *  where visdatPtr is a handle to a VisualizableDatasetInterfaceForPython*.
 *  and level and component are ints.
 *  For an explanation of args real and contrapad, see the comments under
 *  GetBoxLayoutData() in data_access/VisualizableDataset.cpp.
*/
static PyObject *
visdat_GetBoxLayoutData( PyObject * self, PyObject * args )
{
    int visdatPtr;       // handle to VisualizableDataset *
    int level, component, real, contrapad;
    if( ! PyArg_ParseTuple( args, "iiiii", &visdatPtr, &level, &component,
                                           &real, &contrapad ) )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "PyArg_ParseTuple failed." );
        return 0;
    }

    boost::shared_ptr<VisualizableDatasetInterfaceForPython> pVisDat = 
        SharedPointerHandleMap<
            VisualizableDatasetInterfaceForPython>::GetPointer(visdatPtr);
    boost::shared_ptr<BoxLayoutDataInterfaceForPython> pBLD(
        pVisDat->GetBoxLayoutData( level, component, real, contrapad ) );

    PyObject * result = PyInt_FromLong( SharedPointerHandleMap<BoxLayoutDataInterfaceForPython>::GetHandle(pBLD) );
    return result;
}


/** Return a vector<int> that gives the depth of user-supplied ghost cells. */
static PyObject *
visdat_GetOutputGhost( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetOutputGhost, args);
}


/** E.g. (0,0,0)=cell, (1,0,1)=x-z-edge, (1,0)=2D x-face, (1,1,1)=node. */
static PyObject *
visdat_GetDataCentering( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetDataCentering, args);
}

/** Includes components from hdf5 as well as generated ones. */
static PyObject *
visdat_GetComponentNames( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetComponentNames, args);
}


/** Lower-left-hand corner of domain, in distance units. */
static PyObject *
visdat_GetOrigin( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetOriginAsVector, args);
}


/** Usage: >>> (dx,dy,dz) = libVisDat.GetDx( level ) */
static PyObject *
visdat_GetDx( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetDxAsVector, args);
}


/** Usage: >>> dt = libVisDat.GetDt( level ) */
static PyObject *
visdat_GetDt( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetDt, args);
}


/** Usage: >>> libVisDat.FabIsEmpty( level,boxNum,component,padded ) */
static PyObject *
visdat_FabIsEmpty( PyObject * self, PyObject *args )
{
    return SharedTetradicPythonFunc(
        &VisualizableDatasetInterfaceForPython::FabIsEmpty, args);
}

/** Usage: >>> libVisDat.RealFabIsEmpty( level,boxNum,component,padded ) */
static PyObject *
visdat_RealFabIsEmpty( PyObject * self, PyObject *args )
{
    return SharedTetradicPythonFunc(
        &VisualizableDatasetInterfaceForPython::RealFabIsEmpty, args);
}


/** Usage: >>> time = libVisDat.GetTime( level ) */
static PyObject *
visdat_GetTime( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetTime, args);
}


/** Indices of lo and hi corner cells. */
static PyObject *
visdat_GetProblemDomain( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetProbDomain, args);
}

/** 1=float, 2=double */
static PyObject *
visdat_GetPrecision( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetPrecision, args);
}


static PyObject *
visdat_GetDimensionality( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetDimensionality, args);
}


/** Number of levels of refinement. */
static PyObject *
visdat_GetNumLevels( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetNumLevels, args);
}


/** Returns vector<pair<double,double>>.
 *  Usage: >>> p0 = (1.1,2.5,8.2)
 *         >>> p1 = (3.1,0.6,0.9)
 *         >>> libvisdat.GetLinePlot( visdat_ptr, p0,p1, n, component, max_lev )
 *    ... and it returns an n-tuple of 2-tuples.
*/
static PyObject *
visdat_GetLinePlot( PyObject * self, PyObject *args )
{
    return SharedPentadicPythonFunc(
        &VisualizableDatasetInterfaceForPython::GetLinePlot, args);
}


static PyMethodDef VisdatMethods[] =
{
    {"NewVisualizableDataset", visdat_NewVisualizableDataset, METH_VARARGS,
        "Returns the int handle to ptr." },
    {"DefineNewComponent", visdat_DefineNewComponent, METH_VARARGS,
        "Registers a new component generator with a VisualizableDataset." },
    {"RedefineNewComponent", visdat_RedefineNewComponent, METH_VARARGS,
        "Redefines a new component generator." },
    {"Release", visdat_Release,
        METH_VARARGS,
        "Decrements refcount of a VisualizableDataset*." },
    {"SaveToHDF5", visdat_SaveToHDF5, METH_VARARGS,
        "Save a VisualizableDataset to file in Chombo HDF5 format."},
    {"SetDebugLevel", visdat_SetDebugLevel, METH_VARARGS,
        "Same scale as in anag_utils.py -- 0=nothing, 1=Fatal, ..., 5=trace."},
    {"GetBoxLayoutData", visdat_GetBoxLayoutData, METH_VARARGS,
        "Returns int handle into PointerHandleMap."},
    {"GetBoxLayout", visdat_GetBoxLayout, METH_VARARGS,
        "Returns a tuple(tuple(tuple()))"},
    {"Slice", visdat_Slice, METH_VARARGS, "Returns sliced data."},
    {"CropToGeneralBox", visdat_CropToGeneralBox, METH_VARARGS,
        "Trims away cells outside the indicated rectangular prism."},
    {"GetOutputGhost", visdat_GetOutputGhost, METH_VARARGS, "Returns a vector "
       "giving the depth of user-supplied ghost cells, e.g. (1,2,1) or (0,0)."},
    {"GetDataCentering", visdat_GetDataCentering, METH_VARARGS,
        "(0,0,0)=cell, (1,0,1)=x-z-edge, (1,0)=2D x-face, (1,1,1)=node."},
    {"GetComponentNames", visdat_GetComponentNames, METH_VARARGS,
        "Includes components from hdf5 as well as generated ones."},
    {"GetOrigin", visdat_GetOrigin, METH_VARARGS,
        "Lower-left-hand corner of domain, in distance units."},
    {"GetDx", visdat_GetDx, METH_VARARGS, "returns a vector"},
    {"GetDt", visdat_GetDt, METH_VARARGS, "returns a double"},
    {"GetTime", visdat_GetTime, METH_VARARGS, "returns a double"},
    {"GetProblemDomain", visdat_GetProblemDomain, METH_VARARGS,
        "Indices of lo and hi corner cells."},
    {"GetNumLevels", visdat_GetNumLevels, METH_VARARGS,
        "Number of levels of refinement."},
    {"GetPrecision", visdat_GetPrecision, METH_VARARGS,
        "1=float, 2=double."},
    {"GetDimensionality", visdat_GetDimensionality, METH_VARARGS, "2 or 3"},
    {"GetLinePlot", visdat_GetLinePlot, METH_VARARGS,
        "Returns vector<pair<double,double> > --> tuple(tuples) "},
    {"FabIsEmpty", visdat_FabIsEmpty, METH_VARARGS, "returns bool"},
    {"RealFabIsEmpty", visdat_RealFabIsEmpty, METH_VARARGS, "returns bool"},
    {0,0,0,0}
};


extern "C"
void
initlibvisdat()
{
    Py_InitModule( "libvisdat", VisdatMethods );
}
