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
** Date:   August 4, 2003
*/

/**
 * Utilities for the other *module.cpp files in this directory.  Generic Python
 * parsing and function-calling.
*/

#include <Python.h>
#include "module_templates.hImpl"
#include "../../utils/StatusCodes.h"
#include "../Intvect.h"
#include "../Box.h"
#include <string>
#include <vector>
#include <map>
class VisualizableDatasetInterfaceForPython;
class BoxLayoutDataInterfaceForPython;
using std::string;
using std::vector;
using std::pair;


//template<> struct ParseTraits<void> : NOT!  Never return void from the classes
//we wrap here; it presents problems with the RetVal() method.  Instead of void,
//return a Status (enum).

template<> struct ParseTraits<Status>
{
    typedef int ParseType;
    string ScanCode() { return "i"; }
    PyObject * RetVal(Status s) {
        return PyInt_FromLong(s);
    }
    Status Convert(ParseType a) { return Status(a); }
};
template<> struct ParseTraits<bool>
{
    typedef int ParseType;
    string ScanCode() { return "i"; }
    PyObject * RetVal( bool b ) {
        return PyInt_FromLong( long(b) );
    }
    bool Convert(ParseType a) { return (a!=0); }
};
template<> struct ParseTraits<char>
{
    typedef char ParseType;
    string ScanCode() { return "c"; }
    char Convert(ParseType a) { return a; }
};
template<> struct ParseTraits<int>
{
    typedef int ParseType;
    string ScanCode() { return "i"; }
    PyObject * RetVal( int i ) {
        return PyInt_FromLong(i);
    }
    int Convert(ParseType a) { return a; }
};
template<> struct ParseTraits<float>
{
    typedef float ParseType;
    string ScanCode() { return "f"; }
    PyObject * RetVal( double x ) {
        return PyFloat_FromDouble(x);
    }
    float Convert(ParseType a) { return a; }
};
template<> struct ParseTraits<double>
{
    typedef double ParseType;
    string ScanCode() { return "d"; }
    PyObject * RetVal( double x ) {
        return PyFloat_FromDouble(x);
    }
    double Convert(ParseType a) { return a; }
};
template<> struct ParseTraits<string>
{
    typedef char * ParseType;
    string ScanCode() { return "s"; }
    string Convert(ParseType a) { return string(a); }
    PyObject * RetVal( string s ) {
        return PyString_FromString( s.c_str() );
    }
};

template<> struct ParseTraits<boost::shared_ptr<
    VisualizableDatasetInterfaceForPython> >
{
    PyObject * RetVal(
        boost::shared_ptr<VisualizableDatasetInterfaceForPython> x )
    {
        int intPtr = SharedPointerHandleMap<
            VisualizableDatasetInterfaceForPython>::GetHandle(x);
        return PyInt_FromLong(intPtr);
    }
};


template<> struct ParseTraits<boost::shared_ptr<
    BoxLayoutDataInterfaceForPython> >
{
    PyObject * RetVal(
        boost::shared_ptr<BoxLayoutDataInterfaceForPython> x )
    {
        int intPtr = SharedPointerHandleMap<
            BoxLayoutDataInterfaceForPython>::GetHandle(x);
        return PyInt_FromLong(intPtr);
    }
};


template<> struct ParseTraits<vector<int> >
{
    typedef PyObject * ParseType;
    string ScanCode() { return "O"; }
    vector<int> Convert(ParseType a)    // Convert from tuple of ints
    {
        assert( PyTuple_Check(a) );
        vector<int> result;
        unsigned n( PyTuple_Size(a) );
        result.reserve( n );
        for( unsigned i=0;i<n;++i )
        {
            result.push_back( PyInt_AsLong(PyTuple_GetItem(a,i)) );
        }
        return result;
    }
    PyObject * RetVal( vector<int> vi )
    {
        PyObject * result;
        if( vi.size() > 1 )
        {
            result = PyTuple_New( vi.size() );
            for( unsigned i=0;i<vi.size();++i )
            {
                PyTuple_SetItem( result, i, PyInt_FromLong(vi[i]) );
            }
        } else
        {
            result = PyInt_FromLong(vi[0]);
        }
        return result;
    }
};



template<> struct ParseTraits<vector<int> const * >
{
    typedef PyObject * ParseType;
    string ScanCode() { return "O"; }  // Python should pass in a tuple.
    vector<int> * Convert( ParseType a )
    {
        ParseTraits<vector<int> > pt;
        vector<int> * vi = new vector<int>(pt.Convert( a ));
          // FIXME: that's a memory leak.  You could delete croppedLevels in
          // VisualizableDataset::CropToGeneralBox and that would solve the
          // immediate problem...
          // An even better solution: return a std::auto_ptr.
        return vi;
    }
};


template<> struct ParseTraits<vector<string> >
{
    typedef PyObject * ParseType;
    string ScanCode() { return "O"; }
    vector<string> Convert(ParseType a)
    {
        assert( PyTuple_Check(a) );
        vector<string> result;
        unsigned n( PyTuple_Size(a) );
        result.reserve( n );
        for( unsigned i=0;i<n;++i )
        {
            result.push_back( PyString_AsString(PyTuple_GetItem(a,i)) );
        }
        return result;
    }
    PyObject * RetVal( vector<string> vi )
    {
        PyObject * result;

        result = PyTuple_New( vi.size() );
        for( unsigned i=0;i<vi.size();++i )
        {
            PyTuple_SetItem( result, i, PyString_FromString(vi[i].c_str()));
        }
        return result;
    }
};


template<> struct ParseTraits<vector<string> const * >
{
    typedef PyObject * ParseType;
    string ScanCode() { return "O"; }  // Python should pass in a tuple.
    vector<string> * Convert( ParseType a )
    {
        ParseTraits<vector<string> > pt;
        vector<string> * vi = new vector<string>(pt.Convert( a ));
          // FIXME: that's a memory leak.  You could delete croppedComponents in
          // VisualizableDataset::CropToGeneralBox and that would solve the
          // immediate problem...
          // An even better solution: return a std::auto_ptr.
        return vi;
    }
};


template<> struct ParseTraits<vector<float> >
{
    PyObject * RetVal( vector<float> vpo )
    {
        ParseTraits<float> pt;
        PyObject * result;
        if( vpo.size() > 1 )
        {
            result = PyTuple_New( vpo.size() );
            for( unsigned i=0;i<vpo.size();++i )
            {
                PyTuple_SetItem( result, i, pt.RetVal(vpo[i]) );
            }
        } else
        {
            result = pt.RetVal(vpo[0]);
        }
        return result;
    }
};
template<> struct ParseTraits<vector<double> >
{
    PyObject * RetVal( vector<double> vpo )
    {
        ParseTraits<double> pt;
        PyObject * result;
        if( vpo.size() > 1 )
        {
            result = PyTuple_New( vpo.size() );
            for( unsigned i=0;i<vpo.size();++i )
            {
                PyTuple_SetItem( result, i, pt.RetVal(vpo[i]) );
            }
        } else
        {
            result = pt.RetVal(vpo[0]);
        }
        return result;
    }
};


template<> struct ParseTraits<Box const &>
{
    typedef PyObject * ParseType;
    string ScanCode() { return "O"; }
    Box Convert( ParseType a )
    {
        assert( PyTuple_Check(a) );
        assert( PyTuple_Size(a) == 2 );
        PyObject * loCorner = PyTuple_GetItem(a,0);
        PyObject * hiCorner = PyTuple_GetItem(a,1);
        assert( PyTuple_Check( loCorner ) );
        assert( PyTuple_Check( hiCorner ) );
        assert( PyTuple_Size( loCorner ) == 3 );
        assert( PyTuple_Size( hiCorner ) == 3 );

        BoxSimple boxSimple;
        for( int i=0;i<3;++i )
        {
            boxSimple.data[i]   = PyInt_AsLong(PyTuple_GetItem(loCorner,i));
            boxSimple.data[i+3] = PyInt_AsLong(PyTuple_GetItem(hiCorner,i));
        }
        Box result( boxSimple, -1 );
        return result;
    }
    PyObject * RetVal( Box const & box )
    {
        vector<int> loCorner( box.GetLoCorner() );
        vector<int> hiCorner( box.GetHiCorner() );
        vector<vector<int> > corners(2);
        corners[0] = loCorner;
        corners[1] = hiCorner;
        ParseTraits<vector<vector<int> > > pt;
        return pt.RetVal( corners );
    }
};
