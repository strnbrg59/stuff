#
# Suitable for chombo-data/petermc-nodes.3d.hdf5
#
import Tkinter
import chombovis


c=chombovis.latest()
c.vtk_eb.setClipOthers(1, 0.014)
c.vtk_slice.setIsClipped(1)

def scaleFunc( x ):
    c.vtk_eb.setClipOthers(1, float(x))
    c.vtk_slice.setIsClipped(1)

implicit_value_scale = Tkinter.Scale( c.misc.getTopWindow(),
    resolution=0.001,
    from_= 0.0,
    to=0.02,
    command = scaleFunc )
implicit_value_scale.set( 0.014 )
implicit_value_scale.pack()

c.iso.setMin( 0.0055 )
c.iso.toggleVisibility(1)

c.clip.setInsideOut(1)

c.misc.guiUpdate()
