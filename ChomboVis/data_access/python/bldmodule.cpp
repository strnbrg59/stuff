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
 * This file exposes some of the functionality of the BoxLayoutData to
 * the Python interpreter.  Compiled, it constitutes libBLD.so.
 *
 * The general rule for using these functions is, the first argument is
 *   the int handle (in a PointerHandleMap) to a pointer to a BoxLayoutData.
 * Only one .py module -- box_layout_data.py -- should ever call these
 *   functions directly.  box_layout_data.py then exposes an object-
 *   oriented view of this stuff.
*/

#include "module_templates.cpp"
#include "BoxLayoutDataInterfaceForPython.h"
#include <iostream>
#include <vector>
#include <string>
using std::cerr;
using std::endl;
using std::vector;
using std::string;



/** Usage: >>> libBLD.Clone( bld_ptr )
 *  Returns ptr to a BoxLayoutData that's an exact replica of this one, except
 *  for the refcount.
*/
static PyObject *
bld_Clone( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Clone, args);
}


/** Usage: >>> libBLD.UnitTest( bld_ptr, level, component )
*/
static PyObject *
bld_UnitTest( PyObject * self, PyObject *args )
{
// FIRST COMPILATION ERROR HERE

    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::UnitTest, args );
}


/** Usage: >>> libBLD.GetNumBoxes( bld_ptr ) */
static PyObject *
bld_GetNumBoxes( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetNumBoxes, args );
}
/** Usage: >>> libBLD.GetNumCells( bld_ptr ) */
static PyObject *
bld_GetNumCells( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetNumCells, args );
}

/** Usage: >>> libBLD.GetFArrayAsTupleMatrix( bld_ptr, box_num ) */
static PyObject *
bld_GetFArrayAsTupleMatrix( PyObject * self, PyObject * args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetFArrayAsVectorMatrix, args );
}

/** Usage: >>> libBLD.GetFArray( bld_ptr, box_num )
 *  Returns handle into table of pointers.
*/
static PyObject *
bld_GetFArray( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetFArray, args);
}


/** Usage: >>> libBLD.Clamp( bld_ptr, boxNums )
 *  where boxNums is a tuple of the boxes you want to restrict all pointwise
 *  operators and summary statistics to.
*/
static PyObject *
bld_Clamp( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Clamp, args );
}


/** Usage: >>> libBLD.ClampToGeneralBox( bld_ptr, box )
 *  where box is any rectangular prism, expressed as a 2-tuple of 3-tuples
 *  (loCorner, hiCorner).
 *  Compare to bld_Clamp.
*/
static PyObject *
bld_ClampToGeneralBox( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::ClampToGeneralBox, args );
}


/** Usage: >>> libBLD.CropToGeneralBox( bld_ptr, box )
 *  where box is any rectangular prism, expressed as a box.Box.
*/
static PyObject *
bld_CropToGeneralBox( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::CropToGeneralBox, args );
}


/** Usage: >>> libBLD.UnClamp( bld_ptr )
*/
static PyObject *
bld_UnClamp( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &BoxLayoutDataInterfaceForPython::UnClamp, args);
}


/** Usage: >>> libBLD.GetDatum( bld_ptr, box_num, i, j, k )
*/
static PyObject *
bld_GetDatum( PyObject * self, PyObject *args )
{
    return SharedTetradicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetDatum, args);
}


/** Usage: >>> libBLD.SetDatum( bld_ptr, x, box_num, i, j, k )
*/
static PyObject *
bld_SetDatum( PyObject * self, PyObject *args )
{
    return SharedPentadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::SetDatum, args);
}


/** Usage: >>> libBLD.AbsoluteValue(bld_ptr) */
static PyObject *
bld_AbsoluteValue( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_fabs, args);
}

//
// Trigonometric functions
//
/** Usage: >>> libBLD.Sin(bld_ptr) */
static PyObject *
bld_Sine( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_sin, args);
}
/** Usage: >>> libBLD.Cos(bld_ptr) */
static PyObject *
bld_Cosine( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_cos, args);
}
/** Usage: >>> libBLD.Tan(bld_ptr) */
static PyObject *
bld_Tangent( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_tan, args);
}
/** Usage: >>> libBLD.Asin(bld_ptr) */
static PyObject *
bld_ArcSine( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_asin, args);
}
/** Usage: >>> libBLD.Acos(bld_ptr) */
static PyObject *
bld_ArcCosine( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_acos, args);
}
/** Usage: >>> libBLD.Atan(bld_ptr) */
static PyObject *
bld_ArcTangent( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_atan, args);
}
/** Usage: >>> libBLD.Sin(bld_ptr) */
static PyObject *
bld_SineH( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_sinh, args);
}
/** Usage: >>> libBLD.Cos(bld_ptr) */
static PyObject *
bld_CosineH( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_cosh, args);
}
/** Usage: >>> libBLD.Tan(bld_ptr) */
static PyObject *
bld_TangentH( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_tanh, args);
}
/** Usage: >>> libBLD.Asin(bld_ptr) */
static PyObject *
bld_ArcSineH( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_asinh, args);
}
/** Usage: >>> libBLD.Acos(bld_ptr) */
static PyObject *
bld_ArcCosineH( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_acosh, args);
}
/** Usage: >>> libBLD.Atan(bld_ptr) */
static PyObject *
bld_ArcTangentH( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_atanh, args);
}


/** Usage: >>> libBLD.NaturalLog(bld_ptr) */
static PyObject *
bld_NaturalLog( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_log, args);
}

/** Antilogarithm.  Usage: >>> libBLD.Exp(bld_ptr) */
static PyObject *
bld_Exp( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_exp, args);
}

/** Raise all cell values to x power.  Usage: >>> libBLD.Pow(bld_ptr,x) */
static PyObject *
bld_Pow( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_pow, args);
}

/** Raise all cell values to other BLD-th power.
 *  Usage: >>> libBLD.PowBLD(bld_ptr, other_bld_ptr) */
static PyObject *
bld_PowBLD( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_powBLD, args);
}

//
// +=, -=, *= and /=
//

/** Usage: >>> libBLD.PlusEquals(bld_ptr, x), where x must be a scalar. */
static PyObject *
bld_PlusEquals( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_plusEquals, args);
}
/** Usage: >>> libBLD.PlusEqualsBLD(bld_ptr, other_bld_ptr) */
static PyObject *
bld_PlusEqualsBLD( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_plusEqualsBLD, args);
}

/** Usage: >>> libBLD.MinusEquals(bld_ptr, x) */
static PyObject *
bld_MinusEquals( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_minusEquals, args);
}
/** Usage: >>> libBLD.MinusEqualsBLD(bld_ptr, other_bld_ptr) */
static PyObject *
bld_MinusEqualsBLD( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_minusEqualsBLD, args);
}

/** Usage: >>> libBLD.TimesEquals(bld_ptr,x) */
static PyObject *
bld_TimesEquals( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_timesEquals, args);
}
/** Usage: >>> libBLD.TimesEqualsBLD(bld_ptr, other_bld_ptr) */
static PyObject *
bld_TimesEqualsBLD( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_timesEqualsBLD, args);
}

/** Usage: >>> libBLD.DivideEquals(bld_ptr, x) */
static PyObject *
bld_DivideEquals( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_divideEquals, args);
}
/** Usage: >>> libBLD.DivideEqualsBLD(bld_ptr, other_bld_ptr) */
static PyObject *
bld_DivideEqualsBLD( PyObject * self, PyObject *args )
{
    return SharedDiadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Apply_divideEqualsBLD, args);
}


/** Usage: >>> libBLD.Min(bld_ptr) */
static PyObject *
bld_Min( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Min, args);
}
/** Usage: >>> libBLD.Max(bld_ptr) */
static PyObject *
bld_Max( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Max, args);
}


/** Usage: >>> libBLD.Sum(bld_ptr) */
static PyObject *
bld_Sum( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::Sum, args);
}
/** Usage: >>> libBLD.SumOfSquares(bld_ptr) */
static PyObject *
bld_SumOfSquares( PyObject * self, PyObject *args )
{
    return SharedMonadicPythonFunc(
        &BoxLayoutDataInterfaceForPython::SumOfSquares, args);
}


/** Usage: >>> libBLD.GetBoxLayout(bld_ptr)
 *  Returns the box corners as a tuple of tuples of tuples.  Does not return
 *  any information on dx or origin.
*/
static PyObject *
bld_GetBoxLayout( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetBoxLayoutAsNestedVectors, args);
}


/** Usage: >>> libBLD.GetDx(bld_ptr)
 *  Cell dimensions in physical units.
*/
static PyObject *
bld_GetDx( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetDx, args);
}

/** Usage: >>> libBLD.GetOrigin(bld_ptr)
 *  Position of lower left-hand corner, in physical units.
*/
static PyObject *
bld_GetOrigin( PyObject * self, PyObject *args )
{
    return SharedNiladicPythonFunc(
        &BoxLayoutDataInterfaceForPython::GetOrigin, args);
}


static PyMethodDef BldMethods[] =
{
    {"Clone", bld_Clone, METH_VARARGS, "Returns ptr to copy."},
    {"Clamp", bld_Clamp, METH_VARARGS, "Restricts operations to indicated FAB"},
    {"ClampToGeneralBox", bld_ClampToGeneralBox, METH_VARARGS,
        "Restricts operations to cells within indicated rectangular prism."},
    {"CropToGeneralBox", bld_CropToGeneralBox, METH_VARARGS,
        "Trims away cells outside the indicated rectangular prism."},
    {"UnClamp", bld_UnClamp, METH_VARARGS, "Undoes Clamp." },

    {"GetBoxLayout", bld_GetBoxLayout, METH_VARARGS, "tuple(tuple(tuple()))"},
    {"GetDx", bld_GetDx, METH_VARARGS, "A tuple"},
    {"GetOrigin", bld_GetOrigin, METH_VARARGS, "A tuple"},

    {"GetDatum", bld_GetDatum, METH_VARARGS, "Get value of a cell." },
    {"SetDatum", bld_SetDatum, METH_VARARGS, "Set value of a cell." },

    {"AbsoluteValue", bld_AbsoluteValue, METH_VARARGS,
        "In-place: set all values to absolute value of themselves." },

    {"Sin", bld_Sine, METH_VARARGS,
        "In-place: set all values to sine of themselves." },
    {"Cos", bld_Cosine, METH_VARARGS,
        "In-place: set all values to cosine of themselves." },
    {"Tan", bld_Tangent, METH_VARARGS,
        "In-place: set all values to tangent of themselves." },
    {"Asin", bld_ArcSine, METH_VARARGS,
        "In-place: set all values to arcsine of themselves." },
    {"Acos", bld_ArcCosine, METH_VARARGS,
        "In-place: set all values to arccosine of themselves." },
    {"Atan", bld_ArcTangent, METH_VARARGS,
        "In-place: set all values to arctangent of themselves." },
    {"SinH", bld_SineH, METH_VARARGS,
        "In-place: set all values to hyperbolic sine of themselves." },
    {"CosH", bld_CosineH, METH_VARARGS,
        "In-place: set all values to hyperbolic cosine of themselves." },
    {"TanH", bld_TangentH, METH_VARARGS,
        "In-place: set all values to hyperbolic tangent of themselves." },
    {"AsinH", bld_ArcSineH, METH_VARARGS,
        "In-place: set all values to inverse hyperbolic sine of themselves." },
    {"AcosH", bld_ArcCosineH, METH_VARARGS,
        "In-place: set all values to inverse hyperbolic cosine of themselves."},
    {"AtanH", bld_ArcTangentH, METH_VARARGS,
       "In-place: set all values to inverse hyperbolic tangent of themselves."},

    {"NaturalLog", bld_NaturalLog, METH_VARARGS,
        "In-place: set all values to natural logarithm of themselves." },
    {"Exp", bld_Exp, METH_VARARGS, "In-place: antilogarithm." },
    {"Pow", bld_Pow, METH_VARARGS, "In-place: raise to x power." },
    {"PowBLD", bld_PowBLD, METH_VARARGS,
        "In-place: raise to other BLD-th power." },
    {"PlusEquals", bld_PlusEquals, METH_VARARGS, "Arg must be real"},
    {"PlusEqualsBLD", bld_PlusEqualsBLD, METH_VARARGS, "Arg is another BLD)" },
    {"MinusEquals", bld_MinusEquals, METH_VARARGS, "Arg must be real"},
    {"MinusEqualsBLD",bld_MinusEqualsBLD, METH_VARARGS, "Arg is another BLD)" },
    {"TimesEquals", bld_TimesEquals, METH_VARARGS, "Arg must be real"},
    {"TimesEqualsBLD",bld_TimesEqualsBLD, METH_VARARGS, "Arg is another BLD)" },
    {"DivideEquals", bld_DivideEquals, METH_VARARGS, "Arg must be real"},
    {"DivideEqualsBLD",bld_DivideEqualsBLD,METH_VARARGS,"Arg is another BLD)" },

    {"GetNumBoxes", bld_GetNumBoxes, METH_VARARGS, "Number of boxes." },
    {"GetNumCells", bld_GetNumCells, METH_VARARGS, "Number of cells." },
    {"Min", bld_Min, METH_VARARGS, "Minimum cell value." },
    {"Max", bld_Max, METH_VARARGS, "Maximum cell value." },
    {"Sum", bld_Sum, METH_VARARGS, "Sum of cell values." },
    {"SumOfSquares", bld_SumOfSquares, METH_VARARGS, "Sum of Squares." },
    {"GetFArrayAsTupleMatrix", bld_GetFArrayAsTupleMatrix, METH_VARARGS,
        "NumPy matrix"},
    {"GetFArray", bld_GetFArray, METH_VARARGS, "Handle to raw pointer"},

    {"UnitTest", bld_UnitTest, METH_VARARGS, "Unit test." },
    {0,0,0,0}
};


extern "C"
void
initlibbld()
{
    Py_InitModule( "libbld", BldMethods );
}
