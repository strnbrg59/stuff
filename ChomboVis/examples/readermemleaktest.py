import libVTKChomboPython
import sys

for i in range(0,30000):
    reader = 0
    reader = libVTKChomboPython.vtkChomboReader()

    if i%2 == 0:
        reader.LoadFile( 'data/test.2d.hdf5' )
    else:
        reader.LoadFile( 'data/test.3d.double.hdf5' )

    reader.SetAlwaysUseRealBoxes( 1 )
    reader.SetAlwaysUsePaddedBoxes( 0 )

    if i%2 == 0:
        reader.SetCurrentComponent( "density" )
    else:
        reader.SetCurrentComponent( "phi" )

    reader.ShowLevel(0)
    reader.ShowLevel(1)
    reader.ShowLevel(2)
    sys.stderr.write( '@@@ GetVisibleRange()=' + str(reader.GetVisibleRange()) + '\n' )
    sys.stderr.write( '@@@ Iteration ' + str(i) + '\n' )
