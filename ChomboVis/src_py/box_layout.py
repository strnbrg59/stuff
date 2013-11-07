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

# File: box_layout.py
# Author: TDSternberg
# Created: 8/07/03

#
#  Unlike BoxLayoutData, this is not a shadow for the C++ class of the same
#  name.
#

import anag_utils
import self_control
import box

class BoxLayout( self_control.SelfControl ):
#Cut to here
    """
    A collection of boxes, dx and origin.
    The customary way to obtain a BoxLayout is from the getBoxLayout() functions
    of either the VisualizableDataset or BoxLayoutData class.
    """
    def __init__( self, boxes, dx, origin ):
        """
        Arg boxes is a tuple of tuples of tuples -- (i,j,k) within
            (locorner,hicorner) within box within boxlayout.
        Arg dx is cell size -- a tuple.
        Arg origin is the position of the lower-left-hand corner, in physical
            distance units.
        """
        self_control.SelfControl.__init__( self, dep_dict={}, metadata = [
           { 'name':'boxes', 'get':2},  # 'get':2 because we want pydoc to show.
           { 'name':'dx', 'get':2},
           { 'name':'origin', 'get':2}
        ])        
        anag_utils.funcTrace()
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'boxes', 'dx', 'origin' )

        self.boxes  = boxes
        self.dx     = dx
        self.origin = origin


    def getBoxes( self ):
        """ Return the tuple of tuple of tuples """
        anag_utils.funcTrace()
        return self.boxes

    def getDx( self ):
        """ Size of a cell, in each direction """
        anag_utils.funcTrace()
        return self.dx

    def getOrigin( self ):
        """ Position of lower left-hand corner, in physical distance units. """
        anag_utils.funcTrace()
        return self.origin


    def getNumBoxes( self ):
        """ Return the number of boxes. """
        anag_utils.funcTrace()
        return len( self.boxes )


    def clone( self ):
        """
        Return a fully-independent object just like this one.
        """
        anag_utils.funcTrace()
        return BoxLayout( self.boxes, self.dx, self.origin )


    __len__ = getNumBoxes


    def __getitem__(self, b):
        """
        Operator[]().  Returns b-th box -- a pair of corner coordinates.
        """
        anag_utils.funcTrace()
        return self.boxes[b]

    getBox = __getitem__


    def __cmp__( self, that ):
        """ operator==() """
        anag_utils.funcTrace()
        return not (    (self.dx==that.dx) and (self.origin==that.origin)
                    and (self.boxes==that.boxes) )

    def __repr__( self ):
        anag_utils.funcTrace()
        return str(self.boxes)
#Cut from here


    def shrink( self, decr, at_both_ends=1 ):
        """
        Adjust this object down for data centering (that's arg decr).
        Internally the box dimensions are those of the data array (not the cell
         array) but at the API level we want users to think of boxes of cells.
        """
        anag_utils.funcTrace()
        boxes_copy = list( self.boxes )
        for b in range(0, len(self.boxes) ):
            boxes_copy[b] = box.Box( self.boxes[b] )
            boxes_copy[b].shrink( decr, at_both_ends)
        boxes_copy = tuple( boxes_copy )
        self.boxes = boxes_copy[:]
