#include "vtk_stuff.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkExtractEdges.h"
#include "vtkFloatArray.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTubeFilter.h"
#include <cassert>

vtkRenderer* getRenderer()
{
    static vtkRenderer *thing = vtkRenderer::New();
    return thing;
}

vtkRenderWindowInteractor* getInteractor()
{
    static vtkRenderWindowInteractor* thing = vtkRenderWindowInteractor::New();
    return thing;
}

vtkCellPicker* getPicker()
{
    static vtkCellPicker* thing = vtkCellPicker::New();
    return thing;
}

vtkPolyData* getCube()
{
    static vtkPolyData *thing = vtkPolyData::New();
    return thing;
}

vtkPolyDataMapper* getCubeMapper()
{
    static vtkPolyDataMapper *thing = vtkPolyDataMapper::New();
    return thing;
}

vtkLookupTable* getLut()
{
    static vtkLookupTable *thing = vtkLookupTable::New();
    static bool initialized = false;
    if (!initialized)
    {
        thing->SetNumberOfTableValues(12);
        thing->Build();

        // Fill in a few known colors, the rest will be generated if needed
        thing->SetTableValue(0, 1.0, .67, 0, 1); // Orange
        thing->SetTableValue(1, 1.0, 0, 0, 1); // Red
        thing->SetTableValue(2, 0, 0, 1.0, 1); // Blue
        thing->SetTableValue(3, 0, 1.0, 0, 1); // Green
        thing->SetTableValue(4, 1, 1, 1, 1); // White
        thing->SetTableValue(5, 1.0, 1.0, 0, 1); // Yellow
        // Same colors but darker
        thing->SetTableValue(6, 0.9, .28, 0, 1); // Orange
        thing->SetTableValue(7, 0.8, 0, 0, 1); // Red
        thing->SetTableValue(8, 0, 0, 0.8, 1); // Blue
        thing->SetTableValue(9, 0, 0.8, 0, 1); // Green
        thing->SetTableValue(10, 0.8, 0.8, 0.8, 1); // White
        thing->SetTableValue(11, 0.8, 0.8, 0, 1); // Yellow
        initialized = true;
    }
    return thing;
}

// This only exists as a kludge to force pipeline execution.
vtkLookupTable* getLut2()
{
    static vtkLookupTable *thing = vtkLookupTable::New();
    static bool initialized = false;
    if (!initialized)
    {
        thing->SetNumberOfTableValues(12);
        thing->Build();
        for (int i=0;i<12;++i)
        {
            thing->SetTableValue(i, 1, 1, 1);
        }
        initialized = true;
    }
    return thing;
}

template<int N>
void vtk_stuff(std::string moves)
{
    // We'll create the building blocks of polydata including data attributes.
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *polys = vtkCellArray::New();
    getVtkScalars()->SetNumberOfComponents(1);

    initGeometry<N>(points, polys);

    //
    // Associate colors to the cells.
    //
    CubeColoring<N>& coloring(CubeColoring<N>::instance());
    coloring.permute(Permutation<N>());
    permuteColoring<N>(moves);

    // We now assign the pieces to the vtkPolyData.
    getCube()->SetPoints(points);
    points->Delete();
    getCube()->SetPolys(polys);
    polys->Delete();
    getCube()->GetCellData()->SetScalars(getVtkScalars());

    // Now we'll look at it.
    getCubeMapper()->SetInput(getCube());
    getCubeMapper()->SetScalarRange(0,11);
    getCubeMapper()->SetLookupTable(getLut());
    getCubeMapper()->ImmediateModeRenderingOn();
    vtkActor *cubeActor = vtkActor::New();
    cubeActor->SetMapper(getCubeMapper());

    // Color the edges.
    vtkExtractEdges *edges = vtkExtractEdges::New();
    edges->SetInput(getCube());
    vtkTubeFilter *tube = vtkTubeFilter::New();
    tube->SetInput(edges->GetOutput());
    tube->SetRadius(0.03);
    tube->SetNumberOfSides(4);
    vtkPolyDataMapper *tubeMapper = vtkPolyDataMapper::New();
    tubeMapper->SetInput(tube->GetOutput());
    vtkActor *tubeActor = vtkActor::New();
    tubeActor->SetMapper(tubeMapper);
    tubeActor->GetProperty()->SetColor(0,0,0);

    // Cell picking
    getPicker()->AddObserver(vtkCommand::EndPickEvent, getPickCommand<N>());

    // The usual rendering stuff.
    vtkCamera *camera = vtkCamera::New();
    camera->SetPosition(1,1,1);
    camera->SetFocalPoint(0,0,0);

    getRenderWindow()->AddRenderer(getRenderer());

    getInteractor()->SetRenderWindow(getRenderWindow());
    MyInteractorStyle *mis = MyInteractorStyle::New();
    mis->SetDimension(N);

    getInteractor()->SetInteractorStyle(mis);
    getInteractor()->SetPicker(getPicker());

    getRenderer()->AddActor(cubeActor);
    getRenderer()->AddActor(tubeActor);
    getRenderer()->SetActiveCamera(camera);
    getRenderer()->ResetCamera();
    getRenderer()->SetBackground(0.9,0.9,0.9);

    getRenderWindow()->SetSize(400,400);

    // interact with data
    getRenderWindow()->Render();
/**/
    getInteractor()->Start();

    // Clean up

    cubeActor->Delete();
    edges->Delete();
    tubeActor->Delete();
    tubeMapper->Delete();
    tube->Delete();
    camera->Delete();
    mis->Delete();
/**/
}

vtkFloatArray* getVtkScalars()
{
    static vtkFloatArray* thing = vtkFloatArray::New();
    return thing;
}

void MyInteractorStyle::OnRightButtonDown()
{
    int x, y;
    getInteractor()->GetMousePosition(&x, &y);
    getPicker()->Pick(x, y, 0, getRenderer());

    vtkInteractorStyleTrackballCamera::OnRightButtonDown();
}

void MyInteractorStyle::OnKeyPress()
{
    int key_code = getInteractor()->GetKeyCode();
    last_keysym_ = getInteractor()->GetKeySym();
    if (strchr("fFbBuUdDrRlL", key_code) && (int(key_code) != 0)) {
        cerr << char(key_code);
        char buf[2]; buf[0] = key_code; buf[1] = 0;
        turn_interpreter_->Turn(buf);
    }
}

void MyInteractorStyle::OnKeyRelease()
{
    last_keysym_ = "[reset]";
}

void MyInteractorStyle::OnMiddleButtonDown()
{
    turn_interpreter_->ExecuteFromFile(last_keysym_ == "Control_L");
    vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
}

void MyInteractorStyle::SetDimension(int d)
{
    assert((d==2) || (d==3));
    switch(d) {
        case 2: turn_interpreter_ = new TurnInterpreter<2>(); break;
        case 3: turn_interpreter_ = new TurnInterpreter<3>();
    }
}

template<int N> void
TurnInterpreter<N>::Turn(char const* str)
{
    permuteColoring<N>(str);
    forcePipelineUpdate();
}

/** If arg==inverse, then execute the inverse of what's in the file.
  * Good for undoing stuff.
  */
void
TurnInterpreterBase::ExecuteFromFile(bool inverse)
{
    std::ifstream infile("/tmp/rubicmoves.txt");
    std::string turns;
    char* buf = new char[80];
    while (infile.good() && !infile.eof())
    {
        infile.getline(buf, 80, '\n');
        turns += buf;
    }
    if (inverse)
    {
        turns = invert_turnstring(turns);
    }
    Turn(turns.c_str());
    delete[] buf;
}

vtkCxxRevisionMacro(MyInteractorStyle, "$Revision: 1.27 $");
vtkStandardNewMacro(MyInteractorStyle);


template<> void
Picks2Turn<3>::initTurnMap()
{
    turnMap_[std::make_pair(0,1)] = turns_.F;
    turnMap_[std::make_pair(10,12)] = turns_.L;
    std::cerr << "Turn map not finished\n";
}

template<> void
Picks2Turn<2>::initTurnMap()
{
    turnMap_[std::make_pair(15,13)] =
    turnMap_[std::make_pair(22,23)] =
    turnMap_[std::make_pair(9,11)]  =
    turnMap_[std::make_pair(19,18)] = turns_.R;

    turnMap_[std::make_pair(16,17)] =
    turnMap_[std::make_pair(10,8)]  =
    turnMap_[std::make_pair(21,20)] =
    turnMap_[std::make_pair(12,14)] = turns_.L;

    turnMap_[std::make_pair(14,15)] =
    turnMap_[std::make_pair(7,5)]   =
    turnMap_[std::make_pair(11,10)] =
    turnMap_[std::make_pair(1,3)]   = turns_.F;

    turnMap_[std::make_pair(4,6)]   =
    turnMap_[std::make_pair(8,9)]   =
    turnMap_[std::make_pair(2,0)]   =
    turnMap_[std::make_pair(13,12)] = turns_.B;

    turnMap_[std::make_pair(23,21)] =
    turnMap_[std::make_pair(6,7)]   =
    turnMap_[std::make_pair(17,19)] =
    turnMap_[std::make_pair(3,2)]   = turns_.U;

    turnMap_[std::make_pair(20,22)] =
    turnMap_[std::make_pair(5,4)]   =
    turnMap_[std::make_pair(18,16)] =
    turnMap_[std::make_pair(0,1)]   = turns_.D;

    addInverseTurns();
}

vtkRenderWindow* getRenderWindow()
{
    static vtkRenderWindow* thing = vtkRenderWindow::New();
    thing->SetWindowName("rubic");
    return thing;
}

void forcePipelineUpdate()
{
    getCubeMapper()->SetLookupTable(getLut2());
    getCubeMapper()->SetLookupTable(getLut());
    getRenderWindow()->Render();
}

// This isn't about permuting faces, it's just about switching x, y and z
// when calculating the coordinates of the four corners of a facet.
void insertFacePointsPermuted(vtkPoints* points, int axis,
                              int c0, int c1, int c2)
{
    vtkIdType llc[3] = {c0, c1, c2};
    int permutation[3][3] = {{0,1,2}, {2,0,1}, {1,2,0}};
    points->InsertNextPoint(llc[permutation[axis][0]],
                            llc[permutation[axis][1]],
                            llc[permutation[axis][2]]);
}

template void vtk_stuff<2>(std::string);
template void vtk_stuff<3>(std::string);
template void vtk_stuff<4>(std::string);
