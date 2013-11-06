#ifndef __PYTHON_IFACE_H
#define __PYTHON_IFACE_H

#include <vtkObject.h>
#include "VTKRubicConfigure.h"
#include "vtkObjectFactory.h"
#include "vtk_stuff.h"

class VTK_VTKRubic_EXPORT vtkRubicPythonIface : public vtkObject
{
public:
    vtkTypeMacro(vtkRubicPythonIface, vtkObject);
    virtual ~vtkRubicPythonIface() {}
    static vtkRubicPythonIface* New();
    vtkRenderWindowInteractor* getInteractor() {
        return ::getInteractor();
    }
    void init(int dim);
    void turn(char);
    int isUnscrambled();
private:
    int dim_;
};

#endif
