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

"""
Prints breakdown of number of boxes and cells by level.
"""

import chombovis
c=chombovis.this()

n_cells = 0
n_boxes = 0
for level in range(0, c.reader.getNumberOfLevels() ):
    n_level_boxes = c.reader.getNumberOfBoxes( level )
    n_level_cells = 0
    for box in range(0, n_level_boxes ):
        extents_ijk = c.reader.getBoxExtents( level, box )
        n_box_cells = (1 + extents_ijk['i'][1] - extents_ijk['i'][0]) \
                    * (1 + extents_ijk['j'][1] - extents_ijk['j'][0]) \
                    * (1 + extents_ijk['k'][1] - extents_ijk['k'][0])
        n_level_cells = n_level_cells + n_box_cells

    n_cells = n_cells + n_level_cells
    n_boxes = n_boxes + n_level_boxes

    print "level", level, ":", n_level_boxes, "boxes,", n_level_cells, " cells."

print "All levels:", n_boxes, " boxes,", n_cells, " cells."
