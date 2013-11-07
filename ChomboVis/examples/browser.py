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

## Author: TJLigocki

"""
This script is used in Chombo to run ChomboVis on data from a debugger
and to bring up a data browser automatically.
"""

# Load the ChomboVis module
import chombovis

# Get a ChomboVis object
c = chombovis.this()

# Bring up 2 menus (data selection and data browser launcher)
c.reader.showGUI()
c.misc.showDatabrowserLauncher()

# Turn the slices off in 3D
if not c.vtk_data.is2DMode():
  for axis in 'x','y','z':
    c.slice.toggleVisibility(0,axis)

# Show all the bounding boxes
c.reader.setVisibleLevelMax(c.reader.getNumberOfLevels()-1)
c.grid.setDetail('Bounding boxes')

# start a default data browser with the first component on level 0, FAB 0
c.misc.makeDatabrowser('',0,0)

# Make the GUI controls consistent.
c.misc.guiUpdate()
