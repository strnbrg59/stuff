#ifndef VTK_STUFF_H
#define VTK_STUFF_H

#include <map>
#include "rubic.h"
#include "vtkCell.h"
#include "vtkCommand.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkRenderWindowInteractor.h"
class vtkFloatArray;
class vtkPoints;
class vtkCellArray;
class vtkObject;
class vtkCellPicker;
class vtkLookupTable;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkShrinkPolyData;
class vtkRenderWindow;

template<int N> void vtk_stuff(std::string);

void forcePipelineUpdate();

vtkRenderWindowInteractor* getInteractor();

vtkFloatArray* getVtkScalars();

void initGeometry(vtkPoints* points, vtkCellArray* polys);

void insertFacePointsPermuted(vtkPoints* points, int axis,
                              int c0, int c1, int c2);

template<int N> struct MyPickCommand : public vtkCommand
{
    void Execute(vtkObject*, long unsigned int, void*);
};

vtkRenderWindow* getRenderWindow();

struct TurnInterpreterBase
{
    virtual void Turn(char const*) = 0;
    void ExecuteFromFile(bool inverse);
};
template<int N>
struct TurnInterpreter : public TurnInterpreterBase
{
    void Turn(char const*);
};

struct MyInteractorStyle : public vtkInteractorStyleTrackballCamera
{
    static MyInteractorStyle* New();
    vtkTypeRevisionMacro(MyInteractorStyle,vtkInteractorStyleTrackballCamera);
    virtual void OnRightButtonDown();
    virtual void OnMiddleButtonDown();
    virtual void OnKeyPress();
    virtual void OnKeyRelease();
    virtual void OnKeyDown() {}
    virtual void OnChar() {}
    void SetDimension(int d);
private:
    TurnInterpreterBase* turn_interpreter_;
    std::string last_keysym_;
};

template<int N> class Picks2Turn
{
    typedef vtkIdType CellIdT;
    typedef std::pair<CellIdT,CellIdT> CellIdPairT;
    typedef std::map<CellIdPairT, Permutation<N> > TurnMapT;
    TurnMapT turnMap_;
    void initTurnMap(); // Only part that needs specialization on N
    void addInverseTurns();
    Turns<N> turns_;
    Permutation<N> I;
    CellIdT prevPick_;
public:
    Picks2Turn();
    Permutation<N> const& getPermutation(CellIdT cellId);
};

template<int N> vtkCommand* getPickCommand();

vtkCellPicker* getPicker();
vtkPolyData* getCube();
vtkPolyDataMapper* getCubeMapper();
vtkLookupTable* getLut();
vtkLookupTable* getLut2();

#include "vtk_stuff.h_impl"

#endif
