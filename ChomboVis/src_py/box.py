#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
# 
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#

# File: box.py
# Author: TDSternberg
# Created: 8/08/03

import anag_utils
import self_control

class Box( self_control.SelfControl ):
#Cut to here
    """
    A low corner and a high corner, with getters.
    To obtain a specific Box from ChomboVis, do something like this:
    $ chombodata -i data/test.2d.hdf5
    >>> vis = c.reader.getVisualizableDataset()
    >>> box = vis.getBoxLayout(level=2,component=3)[12]
    >>> box
    ((112,40), (135,63))
    >>> box[1]
    (135,63)
    """
    def __init__( self, corners ):
        """
        Arg corners is a tuple of tuples -- (i,j[,k]) within (locorner,hicorner)
        e.g. ((1,2,3),(4,5,6)).
        """
        anag_utils.funcTrace()
        self_control.SelfControl.__init__( self, dep_dict={}, metadata = [
           { 'name':'corners', 'get':1 }
        ])        
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'corners' )

        assert( len(corners) == 2 )
        assert( (len(corners[0])==2) or (len(corners[0])==3) )
        self.corners = corners

    def getLoCorner( self ):
        anag_utils.funcTrace()
        return self.corners[0]

    def getHiCorner( self ):
        anag_utils.funcTrace()
        return self.corners[1]

    def getCorners( self ):
        anag_utils.funcTrace()
        return self.corners


    def grow( self, incr, at_both_ends=1 ):
        """
        If arg incr is an integer, then expand by that number of cells in each
        dimension.  Arg incr can be any integer, positive or negative.
        If incr is a tuple, then interpret each element of the tuple as the
        number of cells to grow by in the corresponding dimension.
        If arg at_both_ends==1, then grow at both ends.  If it's 0, then grow
        only at the high end.

        Compare to Box::Grow() in Box.cpp.
        """
        anag_utils.funcTrace()
        if type(incr) == type(1):
            incr_tuple = (incr,incr,incr)
        else:
            incr_tuple = incr
            if len(incr) == 2:
                incr_tuple = (incr[0], incr[1], 0)

        lo = list(self.getLoCorner())
        hi = list(self.getHiCorner())
        for i in range(0,len(lo)):
            if at_both_ends == 1:
                lo[i] -= incr_tuple[i]
            hi[i] += incr_tuple[i]
        self.corners = ( tuple(lo), tuple(hi) )


    def shrink( self, decr, at_both_ends=1 ):
        """
        Like grow() with arg decr multiplied by -1.
        """
        anag_utils.funcTrace()        
        if type( decr ) == type( 1 ):
            self.grow( -decr, at_both_ends )
        else:
            if len(decr) == 2:
                self.grow( (-decr[0], -decr[1]), at_both_ends )
            else:
                self.grow( (-decr[0], -decr[1], -decr[2]), at_both_ends )


    def getNumCells( self ):
        """
        Return the number of cells.  If the box is degenerate or worse, e.g.
        some locorner > some hicorner (can result from self.grow()), return 0.
        """
        anag_utils.funcTrace()
        result = 1
        for i in range(0,len(self.corners[0])):
            side = 1 + self.corners[1][i] - self.corners[0][i]
            if side > 0:
                result *= side
            else:
                return side                                 # Early return
        return result
            

    def __getitem__(self, i ):
        """ operator[]() """
        anag_utils.funcTrace()
        return self.corners[i]

    def __cmp__( self, that ):
        """ operator==() """
        anag_utils.funcTrace()
        return self.corners != that.corners

    def __repr__( self ):
        anag_utils.funcTrace()
        return str(self.corners)
#Cut from here
