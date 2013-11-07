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

# File: control_cameras.py
# Author: TDSternberg
# Created: 10/29/01

import types
import Tkinter
import Pmw
import sys

import anag_utils
import anag_megawidgets
from self_control import SelfControl

class ControlCameras( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """
    GUI control over clipping plane and camera parameters.
    """

    def getShortDescription(self):
        return "Cameras"
    def getLongDescription(self):
        return "Various camera parameters"

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        self.configure( title='Camera parameters' )
        SelfControl.__init__( self, dep_dict,
            [ {'name':'camera_position'},
              {'name':'axes_camera_position'},
              {'name':'focal_point'},
              {'name':'world_point'},
              {'name':'clipping_range'},
              {'name':'display_point'},
              {'name':'parallel_scale'},
              {'name':'view_up'},
              {'name':'axes_view_up'},
              {'name':'show_dialog', 'save':2, 'initval':0}
          ])
        self.decls = anag_utils.Declarations( "decls", instance=self )
        self.configure( title = "Cameras" )
        self.dialog_inventory.registerDialog( self )
        self.configure( buttons=('close', 'dump'), title='Camera parameters',
                        command=self._buttonHandler )

    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        # The data gets displayed in noneditable EntryFields.
        self.camera_position = MultiEntry( n_entries=3,
            master=self.interior(), label_text='camera position' )
        self.axes_camera_position = MultiEntry( n_entries=3,
            master=self.interior(), label_text='axes camera position' )
        self.focal_point = MultiEntry( n_entries=3,
            master=self.interior(), label_text='focal point' )
        self.world_point = MultiEntry( n_entries=4,
            master=self.interior(), label_text='world point' )
        self.display_point = MultiEntry( n_entries=3,
            master=self.interior(), label_text='display point' )
        self.view_up = MultiEntry( n_entries=3,
            master=self.interior(), label_text='view up' )
        self.axes_view_up = MultiEntry( n_entries=3,
            master=self.interior(), label_text='axes view up' )
        self.parallel_scale = MultiEntry( n_entries=1,
            master=self.interior(), label_text='parallel scale' )
        self.clipping_range = MultiEntry( n_entries=2,
            master=self.interior(), label_text='clipping range', is_writable=1)
        self.clipping_range.setCommand(
            lambda clip_near,clip_far, vol_tk=self.vol_tk_interactor:
                vol_tk.setClippingRange((clip_near,clip_far),
                                        suppress_callbacks=1))
        for multi_entry in (self.camera_position, self.axes_camera_position,
            self.focal_point, self.world_point, self.display_point,
            self.view_up, self.axes_view_up,
            self.clipping_range, self.parallel_scale ):
            multi_entry.pack( anchor='w')



    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )
        self.registerCallback(
            self.vol_tk_interactor.getNotifierCameraPosition(),
            lambda multi, extra_info, entries=self.camera_position:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierAxesCameraPosition(),
            lambda multi, extra_info, entries=self.axes_camera_position:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierFocalPoint(),
            lambda multi, extra_info, entries=self.focal_point:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierWorldPoint(),
            lambda multi, extra_info, entries=self.world_point:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierDisplayPoint(),
            lambda multi, extra_info, entries=self.display_point:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierViewUp(),
            lambda multi, extra_info, entries=self.view_up:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierAxesViewUp(),
            lambda multi, extra_info, entries=self.axes_view_up:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierParallelScale(),
            lambda multi, extra_info, entries=self.parallel_scale:
                entries.set( multi ))
        self.registerCallback(
            self.vol_tk_interactor.getNotifierClippingRange(),
            lambda multi, extra_info, entries=self.clipping_range:
                entries.set( multi ))

        self.camera_position.set( self.vol_tk_interactor.getCameraPosition() )
        self.focal_point.set( self.vol_tk_interactor.getFocalPoint() )
        self.world_point.set( self.vol_tk_interactor.getWorldPoint() )
        self.display_point.set( self.vol_tk_interactor.getDisplayPoint() )
        self.view_up.set( self.vol_tk_interactor.getViewUp() )
        self.parallel_scale.set( self.vol_tk_interactor.getParallelScale() )
        self.clipping_range.set( self.vol_tk_interactor.getClippingRange() )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()
        self.dialog_inventory.unregisterDialog( self )
        self.withdrawGUI()


    def _buttonHandler( self, button_name ):
        if button_name == 'dump':
            self.vol_tk_interactor.printParams()
        else:  # Covers 'close' button, as well as window close widget.
            self.withdrawGUI()


class MultiEntry( Tkinter.Frame ):
    """
    A number of Entry widgets -- typically representing x and y, and maybe z --
    plus a label.
    Methods to set the widgets one at a time or (from a tuple) all together.
    """
    def __init__( self, master, n_entries, label_text, is_writable=None,
                  command=None, **kw ):
        anag_utils.funcTrace()
        Tkinter.Frame.__init__( self, master, **kw )
        
        self.is_writable = is_writable

        Tkinter.Label( self, text=label_text, width=20 ).pack( side='left' )
        self.entry = []
        for i in range(0,n_entries):
            self.entry.append(Pmw.EntryField(self))
            self.entry[i].component('entry').configure( state='disabled' )
            self.entry[i].pack( side='left' )


    def setCommand( self, function ):
        """
        Every time there's a carriage return in any of the elements of
        self.entry, apply the contents of all the elements of self.entry as
        separate parameters to arg function.
        """
        anag_utils.funcTrace()
        for e in self.entry:
            e.configure( command = lambda f=function, self=self:
                self._applyFunctionToAllEntries( f ))


    def _applyFunctionToAllEntries( self, function ):
        args = []
        for item in self.entry:
            args.append( float(item.get()) )
        apply( function, list(args) )


    def set( self, text_multi ):
        anag_utils.funcTrace()
        if type(text_multi) == types.TupleType:
            for i in range(0,len(self.entry)):
                self.entry[i].component('entry').configure( state='normal' )
                self.entry[i].setentry( str(text_multi[i]) )
                if not self.is_writable:
                    self.entry[i].component('entry'
                                           ).configure( state='disabled' )
        else: # special case for parallel_scale
            self.entry[0].component('entry').configure( state='normal' )
            self.entry[0].setentry( str(text_multi) )
            self.entry[0].component('entry').configure( state='disabled' )
