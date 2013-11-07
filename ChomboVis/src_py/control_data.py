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

# File: control_data.py
# Author: TDSternberg
# Created: 5/28/01

""" Creates the control options area for data selection """

import Tkinter
import tkMessageBox

import Pmw

import sys

import algorithms
import anag_utils
import anag_megawidgets
from self_control import SelfControl

class ControlData( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """ Lay out and initialize the widgets.
        There are other classes in this module, but they're all implementation
        details of this one.
    """

    def getShortDescription(self):
        return "Data selection"
    def getLongDescription(self):
        return "Select a component, and visible refinement levels"
    
    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )

        SelfControl.__init__( self, dep_dict, [
            { 'name':'component_selection' },  # components.
            { 'name':'level_widget' }, # min and max scales.
            { 'name':'reslice_widget' }, # position and direction.
            { 'name':'show_dialog', 'save':2, 'initval':0},
            { 'name':'use_ghost_cells_checkbutton' }
        ] )
        self.decls = anag_utils.Declarations( "decls", instance = self )
        decls = anag_utils.Declarations( "decls", "self", "master", "kw",
            "dep_dict", "data_component_menu"
        ) 

        self.configure( title="Data selection" )
        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        #
        # Component selection menu.
        #
        if self.vtk_data.getNumComponents() > 0:
            self.component_selection = Pmw.ScrolledListBox(
                self.interior(), labelpos='nw', label_text='Component:' )
            component_names = (self.vtk_data.getComponentList())[:]
            self.component_selection.setlist( component_names )
            self.component_selection.component('listbox').configure(
                height = min(10, len(component_names)), exportselection=0 )
            self.component_selection.configure(
                selectioncommand = lambda self=self:
                    self._componentSelectionHandler(self.component_selection))
            if self.vtk_data.getCurComponent():
                self.component_selection.select_set(
                    self.vtk_data.getComponentSerialNum(
                        self.vtk_data.getCurComponent()))
            self.component_selection.pack( padx=22,pady=2, anchor=Tkinter.W )

        #
        # "Visible levels" Scales.
        #
        self.level_widget = anag_megawidgets.LevelWidget(
            master=self.interior(),
            dep_dict={ 'saved_states':self.saved_states,
                       'vtk_data':self.vtk_data,
                       'vtk_self':self.vtk_data })
        self.level_widget.pack()


        #
        # Ghost cell visibility.
        #
        self.use_ghost_cells_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Use ghost cells' )
        self.use_ghost_cells_checkbutton.configure(
            command = lambda butt = self.use_ghost_cells_checkbutton, self=self:
                          self.vtk_data.setAlwaysUsePaddedBoxes( butt.get() ))
        self.use_ghost_cells_checkbutton.pack( anchor='w' )
    
        #
        # New component generation.
        #
        new_component_butt = Tkinter.Button( self.interior(), 
            text = "Define new component",
            command = lambda self=self: _NewComponent(
                dep_dict = {'vtk_data':self.vtk_data} ) )
#       new_component_butt.pack()

        #
        # Reslicer.
        #
        if self.vtk_data.isResliceMode():
            axis = self.vtk_data.getResliceDirection()
            self.reslice_widget = _ResliceWidget(
                self.interior(),
                axis = axis,
                axis_position = self.vtk_data.getReslicePosition([axis,]),
                relief = 'groove', borderwidth=2,
                dep_dict = {'saved_states':self.saved_states,
                            'vtk_data':self.vtk_data}
                )
            self.reslice_widget.pack()


    def _refresh( self ):
        anag_utils.funcTrace()

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        if self.vtk_data.getNumComponents() > 0:

            # Set component menu to curcomponent.
            cur_selection = \
                self.component_selection.component('listbox').curselection()
            if cur_selection != ():
                self.component_selection.component('listbox').select_clear(
                                                                cur_selection)
            if self.vtk_data.getCurComponent():
                self.component_selection.select_set(
                    self.vtk_data.getComponentSerialNum(
                        self.vtk_data.getCurComponent()))

        # Set ghost-cell button.
        use_ghost_cells = self.vtk_data.getAlwaysUsePaddedBoxes()
        if( use_ghost_cells !=  self.use_ghost_cells_checkbutton.get() ):
            self.use_ghost_cells_checkbutton.set( use_ghost_cells )

        # Set scales in line with vtk_data:
        self.level_widget.setMinNoCallback( self.vtk_data.getMinVisibleLevel())
        self.level_widget.setMaxNoCallback( self.vtk_data.getMaxVisibleLevel())

        # self.reslice_widget refreshes position and direction:
        if self.vtk_data.isResliceMode():
            direction = self.vtk_data.getResliceDirection()
            self.reslice_widget.setPositionNoCallback(
                self.vtk_data.getReslicePosition([direction,]))
            self.reslice_widget.setDirectionNoCallback( direction )

        self.registerCallback(
            self.vtk_data.getNotifierNewComponentNotifier(),
            self._newComponentNotifierHandler )


    refresh = _refresh  # For use by chombovis.py


    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()


    def _newComponentNotifierHandler( self, compname, unused ):
        anag_utils.funcTrace()
        anag_megawidgets.appendItemToPmwScrolledListBox(
            compname, self.component_selection, 10 )


    def cleanup( self ):
        anag_utils.funcTrace()
        if self.vtk_data.isResliceMode():
            self.reslice_widget.cleanup()
            self.reslice_widget.forget()
        self.dialog_inventory.unregisterDialog( self )
        self.withdraw()


    def _componentSelectionHandler( self, component_selection ):
        """ Handler for selection events in the component-choice menu """
        anag_utils.funcTrace()
        self.vtk_data.setCurComponent( component_selection.getcurselection()[0])


class _ResliceWidget( Tkinter.Frame, SelfControl ):
    """ For control over data slicing.  Shows up when we start chomboVis with
        three arguments (file, x|y|z, position).

        Consists of a "position" entry and an x|y|z radiobutton set.

        "Position" is the position we want to slice at, along the chosen axis
        (x, y, or z).
    """

    def __init__( self, master, axis, axis_position, dep_dict, **kw ):
        """ Arg axis is 'x', 'y' or 'z'.
            Arg axis_position is a float.
        """
        anag_utils.funcTrace()
        Tkinter.Frame.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            {'name':'position_scale'},
            {'name':'direction_butts'}
          ] )
        self.decls = anag_utils.Declarations( "decls", instance=self )

        # "Reslice" Label.
        Tkinter.Label( self, text="Reslice:" ).pack( anchor=Tkinter.W )

        # "Position" Scale
        self.position_scale = anag_megawidgets.EntryScale(
            self,
            button_text="position",
            dep_dict = {'saved_states':self.saved_states },
            length=100 )
        self.position_scale.pack()

        # "Direction" Radiobuttons
        self.direction_butts = anag_megawidgets.RadioSelect(
            self,
            buttontype = 'radiobutton',
            orient = 'horizontal',
            labelpos = 'n',
            label_text = "direction",
            command = 0,
            hull_borderwidth = 2, hull_relief = 'groove',
            )
        map( self.direction_butts.add, ('x','y','z') )
        self.direction_butts.pack( padx=20, anchor=Tkinter.W )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        self.position_scale.configure(
            scale_callback = lambda pos, data=self.vtk_data, self=self:
                self._repositionSlice( data.getResliceDirection(), pos))

        self.direction_butts.configure( command =
            lambda button_tag, f = self._directionButtHandler,
                    scale=self.position_scale:
                f( button_tag, scale ))


        self.direction_butts.invoke( self.vtk_data.getResliceDirection() )


    def _refresh( self ):
        anag_utils.funcTrace()
        self.registerCallback(
            self.vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicFactorsCallback )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def _directionButtHandler( self, axis, scale ):
        anag_utils.funcTrace()
        axis_num = (axis=='y') + 2*(axis=='z')
        position = self.vtk_data.getReslicePosition([axis,])
        extents = self.vtk_data.getDomainExtentsXYZ( permute_in_reslice_mode=0 )
        #print "Setting from_=",extents[axis_num], "to=",extents[3+axis_num], \
        #"for axis", axis
        scale.configure( from_=extents[axis_num], to=extents[3+axis_num],
                         scale_normal_resolution = algorithms.roundDown10(
                                (extents[3+axis_num] - extents[axis_num])/50))
        scale.setScaleValue( position )
        self._repositionSlice(  axis, position )
                                    

    def _anisotropicFactorsCallback( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._directionButtHandler( self.vtk_data.getResliceDirection(),
                                    self.position_scale )


    def _repositionSlice( self, axis, x ):
        anag_utils.funcTrace()
        self.vtk_data.repositionSlice( axis, x )


    def setPositionNoCallback(self,x):
        """
        For use by ControlData.refresh(): sets position scale in line with
        what's in vtk_data, without triggering the callback.
        """
        self.position_scale.setScaleSterile( x )


    def setDirectionNoCallback(self,d):
        """
        For use by ControlData.refresh(): sets direction (x, y or z) in line
        with what's in vtk_data, without triggering the callback.
        """        
        self.direction_butts.setcurselection(d)


# End of class _ResliceWidget


class _NewComponent( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """
    GUI for defining a new component, as a function of existing ones.
    """
    def getShortDescription(self): return "New component"
    def getLongDescription(self):
        return "Define a new component as a function of existing ones."

    def __init__( self, dep_dict, master=None, **kw ):
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict, metadata = [
            {'name':'component_name'},
            {'name':'argument_list'},
            {'name':'component_chooser'}
        ] )
        self.decls = anag_utils.Declarations( 'decls', instance = self )

        self.configure( title="Define New Component",
                        buttons = ('Apply', 'Cancel', 'Close' ))

        #
        # Entry to type new component's name
        #
        entry_frame = Tkinter.Frame( self.interior() )
        Tkinter.Label( entry_frame, text="New component name:" ).pack()
        self.component_name = anag_megawidgets.Entry( entry_frame )
        self.component_name.pack()
        entry_frame.pack( anchor=Tkinter.W )

        anag_megawidgets.HorizRule( self.interior(), width=300 ).pack(
            anchor=Tkinter.W )

        #
        # Frame to hold two ScrolledListBox's -- arguments of the function,
        # and existing components -- and append/insert/delete buttons.
        #
        list_box_frame = Tkinter.Frame( self.interior() )

        self.argument_list = Pmw.ScrolledListBox(
            list_box_frame,
            labelpos='n', label_text = 'Function arguments' )
        self.argument_list.setlist(
            ('xmomentum-0', 'ymomentum-0', 'zmomentum-0') )
        self.argument_list.component('listbox').configure(
            height = 3, exportselection=0 )
        self.argument_list.pack( side='left', anchor=Tkinter.N )

        component_chooser = Pmw.ScrolledListBox(
                                list_box_frame,
                                labelpos='n', label_text = 'Component chooser' )
        component_names = self.vtk_data.getComponentList()
        component_chooser.setlist( component_names )
        component_chooser.component('listbox').configure(
            height = min(5, len(component_names)),
            exportselection=0 )
        component_chooser.pack( side='left' )

        list_box_frame.pack( anchor=Tkinter.W )

        # Append/insert/delete buttons
        button_frame = Tkinter.Frame( self.interior() )
        append_butt = Tkinter.Button( button_frame, text='append' )
        append_butt.pack( side='left' )
        insert_butt = Tkinter.Button( button_frame, text='insert' )
        insert_butt.pack( side='left' )
        delete_butt = Tkinter.Button( button_frame, text='delete' )
        delete_butt.pack( side='left' )
        button_frame.pack( side='top', anchor=Tkinter.W )

        anag_megawidgets.HorizRule( self.interior(), width=300 ).pack(
            anchor=Tkinter.W)

        #
        # Load transformation function from a file.
        #
        Tkinter.Label( self.interior(), text='Define transformation' ).pack()
        transf_func_file_butt = Tkinter.Button( self.interior(),
            text = 'Load transformation function from file' )
            # command = ...
        transf_func_file_butt.pack( anchor=Tkinter.W )
        transf_func_text = Pmw.ScrolledText( self.interior(), text_wrap='none',
            usehullsize=1, hull_width=400, hull_height=100 )
        transf_func_text.pack()

        self.show()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()
