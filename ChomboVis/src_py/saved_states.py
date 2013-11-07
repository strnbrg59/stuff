#
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

# File: saved_states.py
# Author: TDSternberg
# Created: 8/28/01

"""
Owns the dictionary which contains the information chombovis needs to restore
some previous state.

State restoration does not extend to this class; if we restore some previous
state, we don't want to also restore the collection of saved states as of that
time!
"""

import anag_utils
from self_control import SelfControl
import pickle

class SavedStates( SelfControl ):
    def __init__(self, dep_dict):
        anag_utils.funcTrace()

        #
        # Variable "declarations"
        #
        SelfControl.__init__( self, dep_dict, metadata =
        [
            { 'name':'go_save', 'initval':0, 'notify':1 },
                # When this changes, all other classes save their state into
                # self.state_dict.
            { 'name':'state_dict', 'initval':{}, 'get':1, 'notify':1 },
                # The state of the system, saved with pickle.dump().
                # When this changes, all other classes restore themselves using
                # the values in self.state_dict.
            { 'name':'num_HDF5s_loaded', 'get':1, 'set':1, 'initval':0,
              'notify':1 },
                # Set from VtkData, after it's done its part to load a new file.
                # When 0, means there's no data file loaded.  Refers to the
                # number *ever* loaded (there can be only one loaded at a time).
            { 'name':'autorender_override', 'get':1, 'set':1, 'initval':1,
                'notify':1 },
                # When 0, no autorendering occurs regardless of the value of
                # VtkVtk.autorender.  When 1, VtkVtk.autorender is decisive.
                # We use this to absolutely turn off autorendering during the
                # state-restoration process, so as to speed it up.
                # It's also a Notifier, and with just one listener: VtkVtk.
                # That's really somewhat of a kludge, but what it does is
                # invoke VtkVtk.render() upon the end of a restore; if we didn't
                # do that, we'd have to click in the RenderWindow to see the
                # results of a restore operation.

            # This is about the next two variables, prerestore_is_reslice_mode
            # and is_3D_mode.  Notice the second is 'save':1 while the first is
            # not.  This means that in a state-restoration, once SavedStates
            # is restored (and it is restored before anything else) we have a
            # way of knowing if we're in reslice mode and trying to restore a
            # 3D state.  SelfControl needs to know that, to do the right thing
            # with 'save':4 variables.
            { 'name':'prerestore_is_reslice_mode', 'get':1, 'set':1 },
            { 'name':'is_3D_mode',                 'get':1, 'set':1, 'save':1 }

        ])

        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'dep_dict' )

        self._refresh()

    def _initForFirstHDF5( self ):
        pass

    def _refresh( self, is_new_hdf5=None):
        """
        Overrides SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()

    def cleanup( self ):
        anag_utils.funcTrace()


    def loadState( self, picklefile_name, gui_only=0 ):
        """
        From the indicated file, which contains all the information about a
        previously-saved state, load up self.state_dict, and set the notifier
        that causes all other classes to restore and refresh themselves.

        If arg gui_only is 1, then we restore only GUI-specific state (e.g.
        whether dialogs were open, whether EntryScales were in scale or entry
        mode, etc.)
        """
        anag_utils.funcTrace()

        picklefile = open( picklefile_name, 'r' )
        self.setAutorenderOverride(0)
        self.setStateDict( pickle.load( picklefile ),
                           extra_info = gui_only ) #Notifier
        self.setAutorenderOverride(1)

        picklefile.close()

        # This is for if we're in reslice mode but we've just restored a 3D
        # state.  At this point is_3D_mode equals 1, but we want to set it so
        # it's correct for the mode we're really in.  For variables like that in
        # other classes, we use the appropriate 'save' value, but we can't
        # restore SavedStates, so we have to do this manually:
        if (self.is_3D_mode == 1) and (self.prerestore_is_reslice_mode==1):
            self.is_3D_mode = None


    def saveState( self, picklefile_name, one_class_name=None ):
        """
        Saves the current state to the indicated file.

        Triggers a Notifier which causes all other classes to call this
        class's saveClassDict().

        If picklefile_name names a pre-existing file, that one gets lost.
        (From the GUI we don't let you do that so easily.)

        Optional arg one_class_name is for when you want to save the state of
        just one class.  Set it to the string you'd get from a
        str(self.__class__) in the respective class, e.g. 'vtk_data.VtkData'.
        """
        anag_utils.funcTrace()

        self.setStateDict( {}, suppress_callbacks=1 )

        self.setGoSave( 1, extra_info=one_class_name )
        # That triggered saveClassState() everywhere.  self.state_dict is now
        # complete, and we dump it to file.
        picklefile = open( picklefile_name, 'w' )
        pickle.dump( self.getStateDict(), picklefile )
        picklefile.close()


    def saveClassDict( self, class_name, class_dict ):
        """
        Insert one class's state dictionary into the master dictionary of
        all classes.  This is called by all subclasses of SelfControl.
        """
        anag_utils.funcTrace()

        if self.getStateDict().has_key( class_name ):
            anag_utils.fatal("Attempted to introduce duplicate name (",
                class_name, ") into state dictionary.  This is probably",
                " a sign that you have more than one instance of this class."
                " If so, distinguish the instances by defining in each instance"
                " a unique attribute named self.multi_instance_qualifier." )
        self.getStateDict()[class_name] = class_dict
        # Notice we've just modified a Notifier -- self.state_dict -- but this
        # sort of modification won't trigger the callbacks, nor do we want it
        # to.  This is somewhat hackish; it would be ideal to observe a
        # convention that Notifiers call back whenever they're set.


    def flushPickles( self ):
        """
        In vtk_vtk.py, this is bound to the action of destroying the main Vtk
        window.
        """
        anag_utils.funcTrace()
        self.picklefile.close()


    def unitTest( self ):
        anag_utils.funcTrace()
