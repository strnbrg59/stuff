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
Script to demonstrate some API functions.
Type "./chombovis user_script=examples/saturn.py data/test.3d.double.hdf5"
"""

import chombovis

c = chombovis.latest()

for a in 'x','y':
    c.slice.toggleVisibility( on_off=0, axis=a )
c.grid.toggleDomainBox( 0 )
c.iso.toggleVisibility( 1 )
c.iso.setMin( -0.0288 )
c.iso.setMax( -0.0288 )
c.iso.setConstantColor( rgb=(0.54,0.36,0.0) )
c.misc.setViewParams(
    camera_position=(-34.47, 49.28, -1.53),
    axes_camera_position=(-4.01, 7.88, -2.42),
    focal_point=(-6.86, -4.89, 15.15),
    world_point=(-6.80, -4.97, 14.83, 1.0),
    display_point=(205.5, 152.0, 0.85),
    view_up=(0.16, -0.21, -0.96),
    parallel_scale=8.0,
    clipping_range=(11.10, 346.63))

c.misc.guiUpdate()
