#ifndef __PYTHON_IFACE_H
#define __PYTHON_IFACE_H

#include <vtkObject.h>
#include "VTKTestmeConfigure.h"
#include "vtkObjectFactory.h"

class VTK_VTKTestme_EXPORT vtkTestmePythonIface : public vtkObject
{
public:
    vtkTypeMacro(vtkTestmePythonIface, vtkObject);
    virtual ~vtkTestmePythonIface() {}
    static vtkTestmePythonIface* New();
    void go();
};

#endif
