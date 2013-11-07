##   _______              __
##  / ___/ /  ___  __ _  / /  ___
## / /__/ _ \/ _ \/  ' \/ _ \/ _ \
## \___/_//_/\___/_/_/_/_.__/\___/ 
##
## This software is copyright (C) by the Lawrence Berkeley
## National Laboratory.  Permission is granted to reproduce
## this software for non-commercial purposes provided that
## this notice is left intact.
## 
## It is acknowledged that the U.S. Government has rights to
## this software under Contract DE-AC03-765F00098 between
## the U.S. Department of Energy and the University of
## California.
##
## This software is provided as a professional and academic
## contribution for joint exchange.  Thus it is experimental,
## is provided ``as is'', with no warranties of any kind
## whatsoever, no support, no promise of updates, or printed
## documentation.  By using this software, you acknowledge
## that the Lawrence Berkeley National Laboratory and
## Regents of the University of California shall have no
## liability with respect to the infringement of other
## copyrights by any part of this software.
##

## Author: TDSternberg

# This is a script to slice, and save to an hdf5 file, a 2D slice of a 3D
# hdf5 file.
# Type "chombodatalite slice.py --help" for a usage message. (chombodatalite
# is a script in the same directory you have the chombovis script.)

import sys
import anag_utils
import box

if len(sys.argv) != 4:
    sys.stderr.write( "Usage: chombodatalite " + sys.argv[0] +
                      " something.hdf5 slice_axis axis_position.\n"
                      "   Example: chombodatalite " + sys.argv[0] +
                      " $CHOMBOVIS_HOME/share/ChomboVis/data/ebtest.3d.hdf5 'y' 14.6\n" )
    sys.exit(1)


filename = sys.argv[1]
slice_axis = sys.argv[2]
axis_position = float(sys.argv[3])

import visualizable_dataset
v = visualizable_dataset.VisualizableDataset( filename )

t1 = anag_utils.Timer( 'slicing' ); t1.on()
cv = v.slice( slice_axis, float(axis_position) )
t1.stop()

if filename.find('/') == -1:
    outfilename = 'sliced_' + filename
else:
    outfilename='sliced_' + filename[filename.rindex('/')+1:]
t2 = anag_utils.Timer( 'writing slice to ' + outfilename ); t2.on()
cv.saveToHDF5( outfilename=outfilename, ascii=0 )
t2.stop()
