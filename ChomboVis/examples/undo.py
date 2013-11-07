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
ChomboVis user_script for unlimited undo capability.  You get the following
widgets:

1. A scale.  Positions along the scale represent saved states.  When you move
the scale, the system restores itself to the state associated with the scale's
new position.

2. An "insert state" button.  Saves the current state of the system and
associates it with the scale's current position plus 1.

3. A "delete state" button.  Eliminates all knowledge of the system state that's
associated with the scale's current position, and shifts back by 1 the scale
positions associated with all states "to the right" of the deleted one.

Note the state of the undo scale itself (and the map between its positions and
saved states) is not itself saved as part of the system state.
"""

import os
import sys
import Tkinter
import chombovis
import anag_utils
import anag_megawidgets

class Undo( Tkinter.Frame ):

    def __init__(self, master, chombovis_ref, **kw ):
        anag_utils.funcTrace()
        Tkinter.Frame.__init__( self, master, **kw )

        self.highest_state_id=-1
        self.num_states=0
        self.state_name_list=[]
        self.sliderlength=35
        self.chombovis_ref = chombovis_ref

        #
        # State Scale.
        #
        self.scale = anag_megawidgets.Scale(
            self,
            resolution=1,
            sliderlength=self.sliderlength,
            length = self.scaleLength(),
            orient='horizontal'
            )
        self.scale.pack( side='left' )
        self.scale.setCallback( self.scaleHandler )
        
        button_frame = Tkinter.Frame( self )
        button_frame.pack( side='left' )

        #
        # InsertState Button
        #
        self.insert_butt = Tkinter.Button(
            button_frame,
            text="Insert state",
            command = self.insertButtHandler )
        self.insert_butt.pack( side='top' )

        #
        # DeleteState Button
        #
        self.delete_butt = Tkinter.Button(
            button_frame,
            text="Delete state",
            command = self.deleteButtHandler,
            state = 'disabled' )
        self.delete_butt.pack( side='top' )


    def insertState( self, pos ):
        """
        Saves a state and inserts its file name at the pos-th position of the
        state list.
        """
        anag_utils.funcTrace()
        state_name = self.generateNewStateName()

        self.num_states = self.num_states + 1
        if self.num_states > 1:
            self.delete_butt.configure( state='normal' )

        self.chombovis_ref.misc.saveState( state_name )
        self.state_name_list.insert( pos, state_name )
    

    def deleteState( self, pos ):
        anag_utils.funcTrace()
        if self.num_states > 1:
            del self.state_name_list[ pos ]
            self.num_states = self.num_states - 1
            if pos == self.num_states:
                self.restoreState( pos-1 )
            else:
                self.restoreState( pos )
            if self.num_states == 1:
                self.delete_butt.configure( state='disabled' )

    def restoreState( self, pos ):
        """
        Restores the state at position pos, without popping or otherwise
        eliminating it.
        """
        anag_utils.funcTrace()
        self.chombovis_ref.misc.restoreState( self.state_name_list[pos] )


    def insertButtHandler( self ):
        anag_utils.funcTrace()
        #print "num_states=", self.num_states
        if self.num_states == 0:
            self.insertState( 0 )
        else:
            self.insertState( int(self.scale.get()) + 1 )
        self.scale.configure( length = self.scaleLength(),
                              to = max(0,self.num_states-1) )
        self.scale.set( int(self.scale.get()) + 1 )
        

    def deleteButtHandler( self ):
        anag_utils.funcTrace()
        #print "num_states=", self.num_states
        self.deleteState( int(self.scale.get()) )
        self.scale.configure( length = self.scaleLength(),
                              to = max(0,self.num_states-1) )

    
    def generateNewStateName( self ):
        """
        Saves current state to a file.  The files are numbered consecutively.
        Returns name of new file.
        """
        anag_utils.funcTrace()
        self.highest_state_id = self.highest_state_id + 1
        result = '/tmp/' + "undo" + str(self.highest_state_id) + '_' + \
                 str(os.getpid()) + '.state'
        return result
    

    def scaleLength( self ):
        anag_utils.funcTrace()
        return self.sliderlength + 3 +\
            min( 10 * max(0, (self.num_states-1)),
                 100)
    
    
    def scaleHandler( self, arg ):
        """ Restore saved state corresponding to the position of the scale. """
        anag_utils.funcTrace()
        if self.num_states > 0:
            self.restoreState( int(arg) )
    

#
# "main"
#
c=chombovis.this()
undo = Undo( c.misc.getTopWindow(), chombovis_ref=c,
             relief='ridge', borderwidth=2 )
undo.pack( side='left', padx=10 )
