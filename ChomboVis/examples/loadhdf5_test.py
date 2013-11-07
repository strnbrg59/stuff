#
# Tests correct loading of a second hdf5.
# Usage: chombodata user_script=loadhdf5_test.py
#
import sys
import time
import os
import os.path
import chombovis

c=chombovis.this()

#datadir='/home/hepburn/users/tdsternberg/anag/chombo-data/tdgl/'
datadir='./data/'

"""
counter=0
for f in os.listdir( datadir ):
    if f.split('.')[-1:][0] == 'hdf5':
        print "\n\n--- Loading", f, "counter=", counter
        c.reader.loadHDF5( datadir + f )
        c.reader.setVisibleLevelMax(2)
#        c.misc.restoreState( '/tmp/foo.state' )
        c.misc.vtkUpdate()
        if c.cmd_line.getOffScreen() != 1:
            c.misc.tkUpdate()
        #print c.reader.getDataSummary()
        time.sleep(4)

        counter = counter+1
#       if counter > 2: sys.exit(0)
sys.exit(0)
"""

""" Here's a version that will test for memory leaks:"""
for i in range(0,1000000):
    sys.stderr.write( '@@@iteration ' + str(i) + '\n' )
    c.reader.loadHDF5( datadir + 'test.2d.hdf5' )
    c.reader.setVisibleLevelMax(2)
    if c.cmd_line.getOffScreen() != 1:
        c.misc.tkUpdate()
#    c.reader.loadHDF5( '/home/hepburn/users/tdsternberg/anag/chombo-data/greg02.hdf5' )
#   print c.reader.getDataSummary()
    time.sleep(2)

""" """
