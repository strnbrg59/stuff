#include "vtkTestmePythonIface.h"
#include <cassert>
#include <string>
#include <iostream>

/* Implemented in testme.cxx */
void doit();

void vtkTestmePythonIface::go()
{
    doit();
}

vtkStandardNewMacro(vtkTestmePythonIface);
