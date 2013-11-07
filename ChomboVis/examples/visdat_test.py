# Must have LD_LIBRARY_PATH and PYTHONPATH set to ../lib.
# Must have CHOMBOVIS_HOME defined too.

import visualizable_dataset
import anag_utils

def diffsqr(x,y): return (x-y)*(x-y)

i=0
while i==0:
    visdat = visualizable_dataset.VisualizableDataset('../data/test.2d.hdf5')
    visdat.defineNewComponent( 'diffsqr', diffsqr, ('x-momentum','y-momentum') )
    visdat.saveToHDF5( '/tmp/newshock.hdf5' )
    visdat.release()
    print "File saved in /tmp/newshock.hdf5"
    print i
    i = i+1
