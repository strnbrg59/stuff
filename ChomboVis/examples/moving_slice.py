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
# Script to make a movie of a moving slice.
# Type "./chombovis user_script=examples/moving_slice.py off_screen=1 data/test.3d.double.hdf5"
# 

import chombovis
import os
import sys

# User: set the number of desired movie frames here.  Must be 2 or more.
n_frames = 13
assert( n_frames >= 2 )

c = chombovis.this()

domain_lo = c.reader.getDomainBounds()['z'][0]
domain_hi = c.reader.getDomainBounds()['z'][1]
dz = (domain_hi - domain_lo)/(n_frames-1.0)
ppm_list = ""
for i in range(0,n_frames):
    c.slice.setPosition( domain_lo + dz*i, 'z' )
    ppm_name = 'moving_slice_' + str(i) + '.ppm'
    c.misc.hardCopy( ppm_name )
    ppm_list = ppm_list + " " + ppm_name
    if c.cmd_line.getOffScreen():
        sys.stderr.write('.')

# Now try to make the movie.  Need /usr/X11R6/bin/convert, plus mpeg2encode (which is called
# by convert).
print "We will now attempt to make a movie, using /usr/X11R6/bin/convert and mpeg2encode."
if  (0 != os.system('which convert')) or (0 != os.system('which mpeg2encode')):
    print "You don't have convert and/or mpeg2encode.  The frames for your movie are in"
    print "moving_slice_*.ppm.  It is now up to you to find some way to make a movie out"
    print "of them."
else:
    os.system( 'convert ' + ppm_list + ' moving_slice.mpg' )

sys.exit(0)
