# Line plot example.  Load data/particles3d.hdf5; other files might not have
# data at the coordinates we use here.
import chombovis
import vtk_line_plot
c=chombovis.latest()

lineplots = []
for i in range(0,3):
    p0 = (-2.14, 4.285+i/10.0, -0.31)
    p1 = (-2.14, 4.285+i/10.0,  0.49)
    lineplots.append( c.misc.makeLinePlot( p0,p1,8,0,2 ) )
vtk_line_plot.multiGnuplot( lineplots, 'multi.gp' )
