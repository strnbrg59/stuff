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
ChomboVis user_script that puts two buttons beneath the VTK window:
"reset camera", and "exit".  This is suitable for use as a .chombovisrc.py
file.
"""

import sys
import Tkinter
import chombovis

c=chombovis.this()

# Restore-camera-position button.
c.misc.savePosition( '/tmp/chombovis_cameras.state' )
Tkinter.Button( c.misc.getTopWindow(), text="reset camera",
                command = lambda c=c:
                    c.misc.restoreState('/tmp/chombovis_cameras.state' )
              ).pack( side='left' )

# Exit button
Tkinter.Button( c.misc.getTopWindow(), text="exit",
                command = lambda: sys.exit(0) ).pack( side='left' )
