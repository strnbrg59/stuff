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
# 
# Script to demonstrate some API functions.
# Type "./chombovis user_script=examples/bulldog.py data/test.2d.hdf5"
# 

import chombovis

c = chombovis.latest()
c.misc.setRenderWidgetSize( width=600, height=600 )

c.iso.toggleVisibility( 1 )
c.iso.setMin( 1.4 )
c.iso.setMax( 7.5 )
c.iso.setNum( 15 )
c.reader.setVisibleLevelMax(1)

c.misc.setViewParams(
    camera_position=(0.510, 0.023, 2.986),
    axes_camera_position=(0.510, 0.023, 2.986),
    focal_point=(0.508, 0.022, -0.250),
    world_point=(0.508, 0.022, -0.250, 1.0),
    display_point=(199.0, 200.0, 0.171),
    view_up=(0.0, 1.0, 0.0),
    parallel_scale=0.0317,
    clipping_range=(0.676, 15.642))

c.misc.hardCopy( outfile_name='bulldog.ppm', mag_factor=2 )

c.misc.guiUpdate()
