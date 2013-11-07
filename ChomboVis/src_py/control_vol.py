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

# File: control_vol.py
# Author: TDSternberg
# Created: 6/11/01

""" Create the control options area for Volume Rendering """

import anag_utils
import anag_megawidgets
from self_control import SelfControl

import vtkpython

import Tkinter
import Pmw


class ControlVol( anag_megawidgets.SelfDescribingDialog, SelfControl ):

    def getShortDescription(self):
        return "Volume rendering"
    def getLongDescription(self):
        return "Volume rendering"

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        self.configure( title="Volume rendering" )
        SelfControl.__init__( self, dep_dict, metadata = [
                {'name':'visible_checkbutton'},
                {'name':'component_chooser'},
                {'name':'xfer_points_widget'},
                {'name':'read_xfer_points_button'},
                {'name':'show_dialog', 'save':6, 'initval':0},
                {'name':'level_widget' } # min and max scales.
            ])
        self.decls = anag_utils.Declarations( "decls", instance=self )
        self.dialog_inventory.registerDialog( self )

    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self", "is_new_hdf5",
            "dep_dict", 
            'component_names',
            'xfer_button_frame',
            'xfer_points_clear',
            "xferPointsHandler",
            "i","w", "p"
            )

        if self.vtk_data.is2DMode():
            return

        #
        # Make-visible checkbutton.
        #    
        self.visible_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'make visible' )
        self.visible_checkbutton.configure(
            command = lambda self=self, butt=self.visible_checkbutton:
                self.vtk_vol.setIsVisible( on_off = butt.get() ))
        self.visible_checkbutton.pack()
    
        #
        # component chooser
        #
        anag_megawidgets.HorizRule( self.interior(), width=300 ).pack()
        self.component_chooser = Pmw.ScrolledListBox(
            self.interior(), labelpos='nw', label_text='Component:' )
        component_names = self.vtk_data.getComponentList()
        self.component_chooser.setlist( component_names )
        self.component_chooser.component('listbox').configure(
            height = min(5, len(component_names)), exportselection=0 )
        self.component_chooser.configure(
            selectioncommand = lambda self=self:
                self.vtk_vol.setComponent(
                    self.component_chooser.getcurselection()[0] ))
        self.component_chooser.pack( anchor='w' )
        anag_megawidgets.HorizRule( self.interior(), width=300 ).pack()


        #
        # Visible levels.
        #
        self.level_widget = anag_megawidgets.LevelWidget(
            master=self.interior(),
            dep_dict={ 'saved_states':self.saved_states,
                       'vtk_data':self.vtk_data,
                       'vtk_self':self.vtk_vol })
        self.level_widget.pack( anchor='w' )
    

        #
        # Transfer function points.
        #
        self.xfer_points_widget = \
            anag_megawidgets.EntryArray(
                self.interior(),
                n_pairs = 1,
                label_text = "Transfer points",
                borderwidth=2, relief='groove' )
        # These numbers define a step function on [0,255].  The range
        # values are in [0.0,1.0] and indicate opacity.
        # <Return> in last EntryField brings up an extra pair below it.
        self.xfer_points_widget.pack()
        self.xfer_points_widget.setRegisteredEntryCallback(
            self._handleXferPointsWidgetEntry )

        xfer_button_frame = Tkinter.Frame( self.interior() )

        self.read_xfer_points_button = Tkinter.Button( xfer_button_frame,
            text='Read Transfer',  command = lambda self=self,
              pts=self.xfer_points_widget:
                self.xferPointsHandler( pts ) )

        xfer_points_clear = Tkinter.Button( xfer_button_frame,
            text='Clear', command = self.clearXferPoints )

        for w in xfer_points_clear, self.read_xfer_points_button:
            w.pack( side='left', anchor='w' )
        xfer_button_frame.pack( anchor='w' )
    
        decls.memberFunctionAudit( self )


    def clearXferPoints( self ):
        """
        Clear all the transfer points entries, leaving just one blank row of
        them.
        Switch off the make-visible checkbutton.
        """
        anag_utils.funcTrace()
        self.xfer_points_widget.clear()
        self.xfer_points_widget.newPair()
        self.xfer_points_widget.newPair()
        if self.vtk_vol.getIsVisible() == 1:
            self.visible_checkbutton.toggle()
            self.vtk_vol.setIsVisible( 0 )
            self.vtk_vol.setXferPoints( [] )
        self.read_xfer_points_button.configure( state='disabled' )
        self.visible_checkbutton.configure( state='disabled' )


    def xferPointsHandler( self, widget ):
        """
        Loads contents of xfer points widget into the xfer function and
        rerenders.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls",
            "xfer_fun",
            "text_xfer"
            )

        # The Tcl version accepts an arbitrary Tcl function in the text,
        # and creates the function with "eval".  Here, we'll settle for
        # letting the user type in some points.  To specify more general
        # functions, the user can resort to the API.
        xfer_points = []
        for i in range( 0, widget.getNumPairs()):
            if widget.getElement(i,0).get():
                xfer_points.append(
                    (int(widget.getElement(i,0).get()),
                    float(widget.getElement(i,1).get())))
        self.vtk_vol.setXferPoints( xfer_points )
        self.xfer_points_widget.deleteUnusedRows()


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.vtk_data.is2DMode() or self.vtk_data.getNumComponents() == 0:
            return

        self.registerCallback(
            self.vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        #
        # The dialog itself -- visible or hidden.
        #
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )


        #
        # Min and Max visible levels.
        #
        self.level_widget.setMinNoCallback( self.vtk_vol.getMinVisibleLevel())
        self.level_widget.setMaxNoCallback( self.vtk_vol.getMaxVisibleLevel())

        #
        # self.visible_checkbutton
        #
        if self.vtk_vol.getIsVisible() != self.visible_checkbutton.get():
            self.visible_checkbutton.toggle()        


        #        
        # self.component_chooser
        #
        cur_selection = self.component_chooser.component(
                    'listbox').curselection()
        if cur_selection != ():
           self.component_chooser.component(
                    'listbox').select_clear( cur_selection[0] )
        if self.vtk_vol.getComponent():
            self.component_chooser.select_set(
               self.vtk_data.getComponentSerialNum(
                   self.vtk_vol.getComponent() ))


        #
        # Transfer points.
        #
        self.xfer_points_widget.clear()
        for i in range(0, len(self.vtk_vol.getXferPoints())):
            self.xfer_points_widget.newPair()
            p = self.vtk_vol.getXferPoints()[i]
            self.xfer_points_widget.setText( str(p[0]), i, 0 )
            self.xfer_points_widget.setText( str(p[1]), i, 1 )

        self.registerCallback(
            self.vtk_data.getNotifierNewComponentNotifier(),
            self._newComponentNotifierHandler )


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()


    def _newComponentNotifierHandler( self, compname, unused ):
        anag_utils.funcTrace()
        anag_megawidgets.appendItemToPmwScrolledListBox(
            compname, self.component_chooser, 5 )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.dialog_inventory.unregisterDialog( self )
        self.withdraw()


    def _handleXferPointsWidgetEntry( self ):
        """
        This gets called whenever the EntryArray grows by another row.  At
        those moments, we consider re-enabling the set-visible and read-points
        buttons -- if they are disabled (as they are after a call to
        clearXferPoints()), and if there are at least two filled-in lines of
        transfer points.
        """
        anag_utils.funcTrace()
        if self.xfer_points_widget.getNumPairs() >= 2:
            self.read_xfer_points_button.configure( state='normal' )
            self.visible_checkbutton.configure( state='normal' )
