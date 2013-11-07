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

# File: control_annotation.py
# Author: TDSternberg
# Created: 4/26/04

import algorithms
import anag_utils
import anag_megawidgets
from self_control import SelfControl
import vtk_annotation # for g_fonts

import Tkinter
import Pmw

class ControlAnnotation( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """ 
    Captions and stuff.
    """

    def getShortDescription(self):
        return "Captions"
    def getLongDescription(self):
        return "Captions and stuff"

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            { 'name':'show_dialog', 'save':2, 'initval':0},
            { 'name':'new_butt'},
            { 'name':'delete_butt'},
            { 'name':'bold_butt'},
            { 'name':'italic_butt'},
            { 'name':'modes'},
            { 'name':'position', 'initval':{}}, # EntryScales
            { 'name':'rotation', 'initval':{}}  # EntryScales
          ])
        self.decls = anag_utils.Declarations( 'decls',  instance=self )
        decls = anag_utils.Declarations( 'decls', 'master', 'kw',
            'dep_dict',
            'borderwidth', 'top_frame', 'active_caption_frame', 'props_frame',
            'left_props_frame', 'data_dir', 'right_props_frame', 'font_frame',
            'position_frame', 'caption_list_frame', 'caption_list_button_frame',
            'pos_map', 'rot_map', 'axis', 'position_rotation_frame',
            'rotation_frame')

        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self )

        #
        # Setup up widgets
        #
        borderwidth=5
        self.configure( title='annotation' )

        #--------------------------------------------------------
        top_frame = Tkinter.Frame( self.interior(),
            relief=Tkinter.GROOVE, borderwidth=borderwidth,
            highlightbackground='yellow', highlightthickness=3 )
        top_frame.pack()
        #--------------------------------------------------------
        active_caption_frame = Tkinter.Frame( top_frame,
            relief=Tkinter.GROOVE, borderwidth=borderwidth )
        active_caption_frame.pack( fill='both', expand=1 )

        Tkinter.Label( active_caption_frame, text='active caption' ).pack()
        self.active_caption = Pmw.ScrolledText(
            active_caption_frame,
            usehullsize=1,
            hull_width=500,
            hull_height=100,
            text_wrap='none' )
        self.active_caption.pack( fill='both', expand=1 )
        self.active_caption.component('text'
            ).bind( '<KeyRelease>', self._activeCaptionHandler )
        self.active_caption.component('text').configure(state='disabled')
        #--------------------------------------------------------

        #--------------------------------------------------------
        #--------- props -- caption properties ------------------
        props_frame = Tkinter.Frame( top_frame,
                         relief=Tkinter.GROOVE, borderwidth=borderwidth )
        props_frame.pack()


        #--------------------------------------------------------
        left_props_frame = Tkinter.Frame( props_frame,
                          relief=Tkinter.GROOVE, borderwidth=borderwidth )
        left_props_frame.pack( side = 'left', anchor='nw' )

        self.size = anag_megawidgets.EntryScale(
            left_props_frame,
            orient='horizontal',
            button_text = 'font size',
            no_validation=1,
            from_=1, to=100,
            scale_callback = lambda x, self=self:
                self.vtk_annotation.setSize(x) )
        self.size.pack()

        self.opacity = anag_megawidgets.EntryScale(
            left_props_frame,
            orient='horizontal',
            button_text = 'opacity',
            scale_callback = lambda x, self=self:
                self.vtk_annotation.setOpacity(x) )
        self.opacity.pack()

        data_dir = self.vtk_data.getChomboVisDataDir()
        self.color = anag_megawidgets.ColorWheel(
            left_props_frame,
            colorwheel_image= data_dir + '/ColorWheel.ppm',
            relief=Tkinter.GROOVE, borderwidth=borderwidth,
            command = lambda rgb, self=self:
                self.vtk_annotation.setColor(rgb) )
        self.color.pack( anchor='w')


        #--------------------------------------------------------
        #--------------------------------------------------------
        right_props_frame = Tkinter.Frame( props_frame,
                           relief=Tkinter.GROOVE, borderwidth=borderwidth )
        right_props_frame.pack( side='right', anchor='e')


        #--------------------------------------------------------
        font_frame = Tkinter.Frame( right_props_frame )
        font_frame.pack()

        self.modes = anag_megawidgets.RadioSelect(
            font_frame,
            buttontype = 'radiobutton',
            orient = 'horizontal' )
        self.modes.component('hull').configure(
            relief=Tkinter.GROOVE, borderwidth=2 )
        map( self.modes.add, (vtk_annotation.g_modes.static2d,
                              vtk_annotation.g_modes.dynamic2d,
                              vtk_annotation.g_modes.dynamic3d) )
        self.modes.configure( command = self._handleMode )
        self.modes.pack()

        self.font = anag_megawidgets.RadioSelect(
            font_frame,
            buttontype = 'radiobutton',
            orient = 'horizontal' )
        self.font.component('hull').configure(
            relief=Tkinter.GROOVE, borderwidth=2 )
        map( self.font.add, (vtk_annotation.g_fonts.arial,
                             vtk_annotation.g_fonts.courier,
                             vtk_annotation.g_fonts.times) )
        self.font.configure(
            command = lambda button_tag, self=self:
                self.vtk_annotation.setFont( button_tag ) )
        self.font.pack(side='left')

        # "bold" button
        self.bold_butt = anag_megawidgets.Checkbutton(font_frame,text='bold')
        self.bold_butt.configure(
            command = lambda self=self, butt = self.bold_butt:
                          self.vtk_annotation.setBold( butt.get() ))
        self.bold_butt.pack(anchor='w')

        # "italic" button
        self.italic_butt=anag_megawidgets.Checkbutton(font_frame,text='italic')
        self.italic_butt.configure(
            command = lambda self=self, butt = self.italic_butt:
                          self.vtk_annotation.setItalic( butt.get() ))
        self.italic_butt.pack(anchor='w')
        #--------------------------------------------------------
        position_rotation_frame = Tkinter.Frame( right_props_frame,
                         relief=Tkinter.GROOVE, borderwidth=borderwidth )
        position_rotation_frame.pack()

        position_frame = Tkinter.Frame( position_rotation_frame,
                         relief=Tkinter.GROOVE, borderwidth=borderwidth )
        position_frame.pack()

        pos_map = {'x':self.vtk_annotation.setXPosition,
                   'y':self.vtk_annotation.setYPosition,
                   'z':self.vtk_annotation.setZPosition}
        for axis in 'x','y','z':
            self.position[axis] = anag_megawidgets.EntryScale(
                position_frame,
                orient='horizontal',
                button_text = axis + ' position',
                no_validation=1,
                scale_callback = lambda u, axis=axis: pos_map[axis](u))
            self.position[axis].pack()


        rotation_frame = Tkinter.Frame( position_rotation_frame,
                         relief=Tkinter.GROOVE, borderwidth=borderwidth )
        rotation_frame.pack()

        rot_map = {'x':self.vtk_annotation.setXRotation,
                   'y':self.vtk_annotation.setYRotation,
                   'z':self.vtk_annotation.setZRotation}
        for axis in 'x','y','z':
            self.rotation[axis] = anag_megawidgets.EntryScale(
                rotation_frame,
                orient='horizontal',
                button_text = axis + ' rotation',
                no_validation=1,
                from_=0.0, to=360.0, scale_normal_resolution=1.0,
                scale_callback = lambda u, axis=axis: rot_map[axis](u))
            self.rotation[axis].pack()
        #--------------------------------------------------------

        #--------------- caption list ---------------------------
        caption_list_frame = Tkinter.Frame( self.interior(),
            relief=Tkinter.GROOVE, borderwidth=borderwidth,
            highlightbackground='yellow', highlightthickness=3 )
        caption_list_frame.pack()
        Tkinter.Label( caption_list_frame, text='all captions' ).pack()

        caption_list_button_frame = Tkinter.Frame( caption_list_frame )
        caption_list_button_frame.pack(side='left', anchor='nw')

        self.new_butt=Tkinter.Button( caption_list_button_frame, text='new',
            command = self._newCaption )
        self.new_butt.pack()

        self.delete_butt=Tkinter.Button( caption_list_button_frame,
            text='delete',
            command = self._deleteCaption,
            state = 'disabled' )
        self.delete_butt.pack()

        self.caption_list = Pmw.ScrolledListBox( caption_list_frame,
            usehullsize=1,
            hull_width=500,
            hull_height=100 )
        self.caption_list.configure(
            selectioncommand = lambda widget=self.caption_list:
                self._handleCaptionListSelection(widget) )
        self.caption_list.component('listbox').configure( exportselection = 0 )
        self.caption_list.pack() # fill='both', expand=1 )
        #--------------------------------------------------------


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()


    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()

        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        for c in self.vtk_annotation.getCaptionList():
            self.caption_list.insert('end', c['text'])
        index = self.vtk_annotation.getCurrCaptionIndex()
        if index != None:
            self._loadCaption( index )
            self.caption_list.component('listbox').select_set( index )
            self.delete_butt.configure( state = 'normal' )
            self.active_caption.component('text').configure(state='normal')


    def cleanup( self ):
        anag_utils.funcTrace()
        self.dialog_inventory.unregisterDialog( self )
        self.withdrawGUI()


    def _handleMode( self, mode_tag ):
        """
        Deal with the static2d, dynamic2d and dynamic3d thing by reconfiguring
        some of the other widgets.
        """
        anag_utils.funcTrace()

        if mode_tag == vtk_annotation.g_modes.dynamic3d:
            for b in range(0,3):
                self.font.button(b).configure( state = 'disabled' )
            self.bold_butt.configure( state = 'disabled' )
            self.italic_butt.configure( state = 'disabled' )
            for axis in 'x','y','z':
                self.rotation[axis].configure( state = 'normal' )
        else:
            for b in range(0,3):
                self.font.button(b).configure( state = 'normal' )
            self.bold_butt.configure( state = 'normal' )
            self.italic_butt.configure( state = 'normal' )
            for axis in 'x','y','z':
                self.rotation[axis].configure( state = 'disabled' )

        if mode_tag == vtk_annotation.g_modes.static2d:
            for axis in 'x','y':
                self.position[axis].configure(
                    from_=0, to=1, scale_normal_resolution=0.01)
            self.position['z'].configure( state = 'disabled' )
        else:
            self.position['z'].configure( state = 'normal' )
            extents = self.vtk_data.getDomainExtentsXYZ()
            axis_map = {'x':0, 'y':1, 'z':2}
            for axis in 'x','y','z':
                lo = axis_map[axis]
                hi = axis_map[axis]+3
                self.position[axis].configure(
                    from_= extents[lo] - 0.5*(extents[hi]-extents[lo]),
                    to   = extents[hi] + 0.5*(extents[hi]-extents[lo]),
                    scale_normal_resolution = (extents[hi]-extents[lo])/100.0 )

        self.vtk_annotation.setMode( mode_tag )


    def _handleCaptionListSelection( self, widget ):
        anag_utils.funcTrace()
        selection = widget.component('listbox').curselection()
        if selection:
            index = int(selection[0])
            self.vtk_annotation.setCurrCaptionIndex( index )
            self._loadCaption( index )
        

    def _loadCaption( self, index ):
        """
        Load the index-th caption from vtk_annotation and update all the widgets
        to reflect its properties.
        """
        anag_utils.funcTrace()
        self.active_caption.settext(self.vtk_annotation.getText())
        self.size.setScaleSterile( self.vtk_annotation.getSize() )
        self.opacity.setScaleSterile( self.vtk_annotation.getOpacity() )
        self.font.setcurselection( self.vtk_annotation.getFont() )
        if self.bold_butt.get() != self.vtk_annotation.getBold():
            self.bold_butt.toggle()
        if self.italic_butt.get() != self.vtk_annotation.getItalic():
            self.italic_butt.toggle()
        self.color.setRgbEntries( self.vtk_annotation.getColor() )

        self.modes.setvalue(self.vtk_annotation.getMode())
        self._handleMode( self.vtk_annotation.getMode() )
        self.position['x'].setScaleSterile( self.vtk_annotation.getXPosition() )
        self.position['y'].setScaleSterile( self.vtk_annotation.getYPosition() )
        prev_state = {}
        for w in self.position['z'],\
                 self.rotation['x'], self.rotation['y'], self.rotation['z']:
            prev_state[w] = w.scale['state']
            w.configure(state='normal')
        self.position['z'].setScaleSterile( self.vtk_annotation.getZPosition() )
        rot_map = {'x':self.vtk_annotation.getXRotation,
                   'y':self.vtk_annotation.getYRotation,
                   'z':self.vtk_annotation.getZRotation}
        for axis in 'x','y','z':
            self.rotation[axis].setScaleSterile( rot_map[axis]() )
        for w in prev_state.keys():
            w.configure(state = prev_state[w])

    def _newCaption( self ):
        """
        Create a new caption and load it as the current one.
        """
        anag_utils.funcTrace()

        self.vtk_annotation.newCaption()
        self._loadCaption( self.vtk_annotation.getCurrCaptionIndex() )

        # Unselect current, to show something's up.
        curr_selection = self.caption_list.component('listbox').curselection()
        if curr_selection:
            index = int(curr_selection[0])
            self.caption_list.component('listbox').select_clear(index)
        self.active_caption.focus_set()

        # Make an entry, in captions_list, for the new caption.
        self.caption_list.component('listbox').insert('end',
            self.vtk_annotation.getText() )
        self.caption_list.component('listbox').select_set(
            len(self.vtk_annotation.getCaptionList())-1 )

        # Enable delete button (only necessary if the caption list was, until
        # now, empty.
        self.delete_butt.configure( state = 'normal' )

        self.active_caption.component('text').configure(state='normal')


    def _deleteCaption( self ):
        anag_utils.funcTrace()
        curr_selection = self.caption_list.component('listbox').curselection()
        assert( curr_selection ) # Otherwise, button should be disabled
        index = int(curr_selection[0])
        self.vtk_annotation.deleteCaption( index )
        self.caption_list.delete( index )
        if index > 0:
            self.caption_list.select_set( index - 1 )
        else:
            if self.caption_list.size() > 0:
                self.caption_list.select_set( 0 )
            else:
                self.delete_butt.configure( state = 'disabled' )
                self.active_caption.settext('')
        self._handleCaptionListSelection( self.caption_list )
        if self.caption_list.size() == 0:    
            self.active_caption.component('text').configure(state='disabled')


    def _activeCaptionHandler( self, event ):
        """
        Every time there's a keystroke in self.active_caption, update its
        corresponding entry in self.caption_list and vtk_annotation.
        """
        anag_utils.funcTrace()

        # vtk
        self.vtk_annotation.setText( self.active_caption.get() )
        self.vtk_annotation.updateVis()

        # self.caption_list
        selection_index = int(self.caption_list.component('listbox'
                                                         ).curselection()[0])
        vtk_index = self.vtk_annotation.getCurrCaptionIndex()
        if selection_index != vtk_index:
            anag_utils.fatal( 'selection_index=', selection_index,
                              ', vtk_index=', vtk_index )
        assert( selection_index == vtk_index )
        self.caption_list.component('listbox').delete(vtk_index,vtk_index)
        self.caption_list.component('listbox').insert(
            vtk_index, self.vtk_annotation.getText())
        self.caption_list.component('listbox').select_set(
            (vtk_index,))
