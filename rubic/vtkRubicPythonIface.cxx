#include "vtkRubicPythonIface.h"
#include "vtk_stuff.h"
#include "rubic.h"
#include <cassert>
#include <string>

#include <iostream>

void vtkRubicPythonIface::init(int dim)
{
    dim_ = dim;
    switch(dim) {
        case 2 : vtk_stuff<2>(""); break;
        case 3 : vtk_stuff<3>(""); break;
        default : assert(0);
    }
}

void vtkRubicPythonIface::turn(char c)
{
    char str[2];
    str[0] = c;
    str[1] = 0;
    switch(dim_) {
        case 2 : permuteColoring<2>(str); break;
        case 3 : permuteColoring<3>(str); break;
        default : assert(0);
    }
    forcePipelineUpdate();
}

int vtkRubicPythonIface::isUnscrambled()
{
    switch(dim_) {
        case 2 : return cubeIsUnscrambled<2>(); break;
        case 3 : return cubeIsUnscrambled<3>(); break;
        default : assert(0);
    }
}

vtkStandardNewMacro(vtkRubicPythonIface);
