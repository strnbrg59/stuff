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
If you want movies with timestamped frames, then this is part of your
solution.  The complete solution is as follows:

1. Go into the directory that has the hdf5 files you want to make the movie 
out of.

2. Pass this file as an argument (after -u) to movie.py.  movie.py is in the
same directory as this file.  Before using it, it's a good idea to type
"movie.py --help".

3. On some systems, captions don't show up if you run movie in offscreen (i.e.
-n) mode.
"""

import chombovis
import os
import sys
c=chombovis.latest()

c.annotation.addNewCaption()
c.annotation.setColor(0,0,0)
c.annotation.setSize(80)
c.annotation.setPosition(0.5,0.3)
c.annotation.setText( str(c.reader.getLevelTime(0)) )

c.misc.hardCopy( outfile_name=os.getenv('IMAGE_FILE'))
sys.exit(0)
