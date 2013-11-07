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

# File: control_fab_tables.py
# Author: TDSternberg
# Created: 9/17/01

"""
GUI-level functionality for the databrowser.
"""

import sys
import types
import re
import string

import Tkinter
import Pmw

import anag_megawidgets
import anag_utils
import vtk_fab_tables # For generate_covered{2,3}()
import tkMessageBox
from vtk_eb import g_eb_discriminator_name
from self_control import SelfControl

g_decls = anag_utils.Declarations( 'g_decls', 'g_eb_policy_tags',
    'g_sliderlength', 'g_scalelength' )
g_sliderlength = 35
g_scalelength = 100

class _EBPolicyTags:
    show_irregular = 'Show irregular cells'
    show_regular = 'Show regular cells'
    show_covered = 'Show covered cells'
    hilite_irregular = 'Hilite irregular cells'
g_eb_policy_tags = _EBPolicyTags()


class ControlFabTables( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """
    A control panel from which we launch databrowsers.
    Contains widgets to select component, level, box and axis.
    """

    #timer = anag_utils.Timer(label='')
    #timer.setStaticVerbose(1)


    def getShortDescription(self):
        return "Databrowsers"
    def getLongDescription(self):
        return "Launch and manage databrowsers"

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()

        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )

        SelfControl.__init__( self, dep_dict, metadata = [
            {'name':'component_chooser'},
            {'name':'component_chooser_selection', 'initval':None, 'save':2},
            {'name':'level_and_box_scales'},
            {'name':'slicing_axis'},  # radiobuttons
            {'name':'format'}, # E.g. %7.3f (EntryField)
            {'name':'format_str', 'save':2}, # The string -- s.th. we can save
            {'name':'databrowser_cltn', 'initval':[]},
            {'name':'show_dialog', 'save':2, 'initval':0},
            {'name':'synchronizer'},
            {'name':'local_vtk_data'},
            {'name':'show_ghost_cells_checkbutton'},
            {'name':'picked_particle_coordinates', 'initval':[]},
            {'name':'font_family', 'set':1, 'save':1, 'initval':'System'},
            {'name':'font_size', 'set':1, 'save':1, 'initval':10},
            {'name':'font_style', 'set':1, 'save':1, 'initval':'bold'}
        ] )
        self.decls = anag_utils.Declarations( 'decls', instance = self )
        decls = anag_utils.Declarations( 'decls', 'self', 'master', 'kw',
            'dep_dict', 'component_chooser'
        ) 

        self.configure( title='Data browsers',
                        buttons = ('Go', 'Close'),
                        command = self._bottomButtonHandler )
        self.format_str = '%11.4e'
        self.synchronizer = _Synchronizer(
            dep_dict={'saved_states':self.saved_states})
        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self )


    def _bottomButtonHandler( self, butt_name ):
        """
        Deals with button presses in the two buttons at the bottom --
        'Go' and 'Close'.
        """
        anag_utils.funcTrace()
        if butt_name == 'Go':
            self.goDatabrowser()        
        elif  butt_name == 'Close':
            self.withdrawGUI()
            if self.cmd_line.getNoVtk(): sys.exit(0)
        else:
            self.withdrawGUI()  # Covers window's own closer widget in corner.


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        self.local_vtk_data = self.vtk_fab_tables.getLocalVtkData()

        # Component
        self.component_chooser = Pmw.ScrolledListBox(
                                     self.interior(),
                                     labelpos='nw', label_text = 'Component' )
        component_names = self.local_vtk_data.getComponentList()
        self.component_chooser.component('listbox').configure(
            height = min(5, len(component_names)), exportselection=0 )
        self.component_chooser.configure( selectioncommand = lambda self=self:
            self._componentChooserHandler())
        self.component_chooser.pack( anchor=Tkinter.W )

        # Scales
        anag_megawidgets.HorizRule( self.interior(), width=200 ).pack(
            anchor=Tkinter.W )
        self.level_and_box_scales = _LevelAndBoxScales(
            self.interior(),
            dep_dict = {'saved_states':self.saved_states,
                        'local_vtk_data':self.local_vtk_data})

        self.level_and_box_scales.pack( expand=1, fill=Tkinter.X,
                                        anchor=Tkinter.W )
        anag_megawidgets.HorizRule( self.interior(), width=200 ).pack(
            anchor=Tkinter.W )


        # Format (e.g. "%7.4f")
        self.format = Pmw.EntryField(
            self.interior(),
            labelpos = 'w',
            label_text = 'format',
            )
        self.format.component('entry').configure( width = 6 )
        self.format.pack( anchor='nw', side='top' )

        # Axis (x|y|z)
        if( not self.local_vtk_data.is2DMode()
        or  self.local_vtk_data.isResliceMode() ):
            self.slicing_axis = anag_megawidgets.RadioSelect(
                self.interior(),
                buttontype = 'radiobutton',
                orient = 'horizontal',
                labelpos = 'w',
                label_text = 'normal'
                )
            map( self.slicing_axis.add, ('x','y','z') )
            self.slicing_axis.invoke( 'z' )
            self.slicing_axis.pack( anchor='nw', side='top' )


        # Synchronization checkbutton.  If on, kscales follow those of all other
        # databrowsers for the same box (i.e. databrowsers for other
        # components).  Also if on, reslice-mode kscale follows slice position.
        self.synchronizer.createSyncButton( self.interior() )

        self.show_ghost_cells_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Show ghost cell data' )
        self.show_ghost_cells_checkbutton.configure( command = lambda
            self=self, butt=self.show_ghost_cells_checkbutton:
                self._setShowGhostCells( butt.get() ))
        self.show_ghost_cells_checkbutton.pack( anchor='w' )
        self.show_ghost_cells_checkbutton.set(
            self.vtk_fab_tables.getShowGhostCells() )

        anag_megawidgets.HorizRule( self.interior(), width=200 ).pack(
            anchor=Tkinter.W )

        n_particle_comps = self.local_vtk_data.getNumParticleComponents()
        if n_particle_comps > 0:
            particles_area = Tkinter.Frame( self.interior())
            Tkinter.Label( particles_area, text='Picked particle:'
                ).pack()
            particles_area.pack()
            max_label_width=-1
            for i in range(0, n_particle_comps):
                max_label_width =\
                    max( max_label_width,
                         len(self.local_vtk_data.getParticleComponentName(i)) )
            for i in range(0, n_particle_comps):
                frame = Tkinter.Frame( particles_area )
                frame.pack()
                Tkinter.Label( frame, width=max_label_width,
                    text=self.local_vtk_data.getParticleComponentName(i)
                    ).pack(side='left', anchor='w')
                self.picked_particle_coordinates.insert( i,
                    Pmw.EntryField( frame ) )
                self.picked_particle_coordinates[i].component('entry'
                    ).configure( state='disabled' )
                self.picked_particle_coordinates[i].pack()


        self.registerForSelectorCallbacks()


    def _defineEBCoveredComponent( self, eb_component ):
        """
        Define new component that indicates covered/regular/irregular cells.
        This auxiliary component is needed in the databrowser.
        """
        anag_utils.funcTrace()
        if( not (g_eb_discriminator_name + str(eb_component))
            in self.local_vtk_data.getComponentList() ):
            eb_args =  ['fraction-' + str(eb_component),
                        'xnormal-'  + str(eb_component),
                        'ynormal-'  + str(eb_component) ]
            if self.local_vtk_data.is2DMode():
                self.vtk_data.defineNewComponent(
                    g_eb_discriminator_name + str(eb_component),
                    vtk_fab_tables.generate_covered2,  tuple(eb_args) )
            else:
                eb_args.append( 'znormal-'  + str(eb_component) )
                self.vtk_data.defineNewComponent(
                    g_eb_discriminator_name + str(eb_component),
                    vtk_fab_tables.generate_covered3,  tuple(eb_args) )


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()

        comp_num = self.local_vtk_data.getComponentSerialNum(
                        self.local_vtk_data.getCurComponent() )
        self.component_chooser.select_set( comp_num )
        self.component_chooser_selection = comp_num
        self._componentChooserHandler()

        for i in range(0, self.local_vtk_data.getNumEBs()):
            self._defineEBCoveredComponent( i )
        self._refresh()


    def registerForSelectorCallbacks( self ):
        """
        Gotta do this in _refresh(), of course, but also in _initForFirstHDF5()
        because we want picking ability even before any components have been
        loaded.
        """
        anag_utils.funcTrace()

        self.format.setentry( self.format_str )
        self.format.configure( validate = self._formatCallback )

        self.registerCallback(
            self.local_vtk_data.getNotifierSelectedBox(),
            self.level_and_box_scales.setLevelBoxNumAndExtents )
        self.registerCallback(
            self.local_vtk_data.getNotifierSelectedCell(),
            lambda cell_ijk, dummy, self=self, format=self.format:
                self.level_and_box_scales.showHiliteCellCoords(
                    cell_ijk,
                    format.get() ))


    def _refresh( self ):
        anag_utils.funcTrace()

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        # Doing this here, rather than in _initForFirstHDF5(), because vtk_eb
        # might have called defineNewComponent().
        component_names = self.local_vtk_data.getComponentList()
        self.component_chooser.setlist( component_names )
        if self.component_chooser_selection != None:
            self.component_chooser.select_set(self.component_chooser_selection)

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.
        
        self.registerForSelectorCallbacks()

        self.registerCallback(
            self.local_vtk_data.getNotifierResliceDirection(),
            self._resliceDirectionCallback )
        self.registerCallback(
            self.local_vtk_data.getNotifierCurComponent(),
            self._curComponentHandler )

        show_ghost_cells = self.vtk_fab_tables.getShowGhostCells()
        if( show_ghost_cells !=  self.show_ghost_cells_checkbutton.get() ):
            self.show_ghost_cells_checkbutton.set( show_ghost_cells )
        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )

        self.registerCallback(
            self.local_vtk_data.getNotifierPickedParticle(),
            self._pickedParticleHandler )

        self.registerCallback(
            self.local_vtk_data.getNotifierNewComponentNotifier(),
            self._newComponentNotifierHandler )


    def _pickedParticleHandler( self, particle_num, dummy ):
        anag_utils.funcTrace()
        for i in range(0, self.local_vtk_data.getNumParticleComponents()):
            name = self.local_vtk_data.getParticleComponentName( i )
            self.picked_particle_coordinates[i].setentry(
                self.format.get() %
                self.local_vtk_data.getParticleCoordinate(particle_num,name) )


    def _componentChooserHandler( self ):
        """ Handler for selection events in the component-choice menu """
        anag_utils.funcTrace()
        cell = self.local_vtk_data.getSelectedCell()
        if cell:
            self.local_vtk_data.setSelectedCell( cell )
            # Triggers callback that causes hilite cell value to refresh,
            # showing the value on the currently-selected component.
        self.component_chooser_selection =\
            self.local_vtk_data.getComponentSerialNum(
                self.component_chooser.getcurselection()[0])

        self.registerCallback(
            self.local_vtk_data.getNotifierSelectedCell(),
            lambda cell_ijk, dummy, self=self, format=self.format:
                self.level_and_box_scales.showHiliteCellValue(
                    cell_ijk, self.component_chooser.getcurselection()[0],
                    format.get() ))



    def _newComponentNotifierHandler( self, compname, unused ):
        anag_utils.funcTrace()
        anag_megawidgets.appendItemToPmwScrolledListBox(
            compname, self.component_chooser, 5 )


    def cleanup( self ):
        anag_utils.funcTrace()
        for assembly in self.databrowser_cltn:
            self._databrowserCloseHandler( None, assembly )
            assembly.cleanup()
        self.synchronizer.cleanup()
        self.dialog_inventory.unregisterDialog( self )
        self.withdrawGUI()


    def _paddedBoxesCallback( self, on_off, dummy ):
        anag_utils.funcTrace()
        show_ghost_cells = self.vtk_fab_tables.getShowGhostCells()
        if( show_ghost_cells !=  self.show_ghost_cells_checkbutton.get() ):
            self.show_ghost_cells_checkbutton.set( show_ghost_cells )


    def _setShowGhostCells( self, on_off ):
        """
        If on_off==1, show data for the ghost cells too.  By default, this is
        set to 1 iff there are user-supplied ghost cells in the hdf5 file.
        Changing that default setting means we have to create a new FAB and
        either generate ghost cell data, or (when there is user-supplied ghost
        cell data) cut that data out.  (Unless that FAB has already been
        generated for use by one of the pipelines -- typically isosurface/
        isocontour.)
        """
        anag_utils.funcTrace()
#       self.show_ghost_cells_checkbutton.set( on_off )
        self.vtk_fab_tables.setShowGhostCells( on_off )
        # Close all open databrowsers, or we run into trouble when user tries
        # to change the component they show:
        for assembly in self.databrowser_cltn:
            self._databrowserCloseHandler( None, assembly )


    def _curComponentHandler( self, component, dummy ):
        """
        Callback for vtk_data.cur_component.
        """
        anag_utils.funcTrace()
        selection = self.component_chooser.getcurselection()
        if selection != ():
            prev_component = self.local_vtk_data.getComponentSerialNum(
                                                                 selection[0])
            self.component_chooser.component('listbox').select_clear(
                                                                 prev_component)
        self.component_chooser.component('listbox').select_set( 
            self.local_vtk_data.getComponentSerialNum(component) )


    def _resliceDirectionCallback( self, dummy1, dummy2 ):
        """
        Whatever vtk_data.reslice_direction changes to, set the x|y|z radio-
        button selection to that.
        """
        anag_utils.funcTrace()
        self.slicing_axis.invoke( self.local_vtk_data.getResliceDirection() )


    def _formatCallback( self, str ):
        """
        This keeps self.format_str current with what's in the format widget.
        If we made this the 'command', it would only get called upon a <Return>.
        But instead we register this as the validation function, and so it gets
        called upon any keystroke.
        """
        anag_utils.funcTrace()
        self.format_str = self.format.get()
        return Pmw.OK

    def goDatabrowser( self, component=None, level=None, box_num=None ):
        """
        Constructs a new databrowser assembly, or if one already exists for the
        currently selected level and box, returns that one.
        """
        anag_utils.funcTrace()
        if self.component_chooser.getcurselection() == ():
            return                                         # Early return
        if not level:
            level = self.level_and_box_scales.getLevel()
        if not box_num:
            box_num = self.level_and_box_scales.getBoxNum()
        if not component:
            component = self.component_chooser.getcurselection()[0]
        self.makeDatabrowser( component, level, box_num )


    def withdrawDatabrowser( self, component, level, box_num ):
        anag_utils.funcTrace()

        for a in self.databrowser_cltn:
            if( (a.getLevel()==level) and (a.getBox()==box_num)
            and (a.getComponent() == component)):
                a.cleanup()
                return
        anag_utils.warning( "Browser doesn't exist: component", component,
            ", level", level, ", box_num", box_num )


    def makeDatabrowser( self, component, level, box_num ):
        """
        Does the actual work, that goDatabrowser() is just a front for.
        """
        anag_utils.funcTrace()

        if( not self.local_vtk_data.is2DMode() 
        or self.local_vtk_data.isResliceMode() ):
            axis = self.slicing_axis.getcurselection()
        else:
            axis = 'z' # Ignored

        # See if we already have this databrowser, i.e. a databrowser for the
        # same component, level and box.  If yes, don't bother to
        # build a new one.  But be sure to reconfigure the existing one so it's
        # consistent with the databrowser launcher's slicing axis and format.
        browser_already_exists = 0
        for a in self.databrowser_cltn:
            if( (a.getLevel()==level) and (a.getBox()==box_num)
            and (a.getComponent() == component)):
                browser_already_exists = 1
                if (    self.local_vtk_data.is2DMode()
                and not self.local_vtk_data.isResliceMode() ):
                    axis = 'z'
                else:
                    axis = self.slicing_axis.getcurselection()
                a.syncWithLaunchWidget(
                    axis=axis,
                    format = self.format.get() )
                a.showGUI()
        if browser_already_exists:
            self.local_vtk_data.setNewDatabrowser(1)
            return

        # If got here, we need to construct a new DatabrowserAssembly.
        the_fab_table = self.vtk_fab_tables.newFabTable(
            component, level, box_num )

        databrowser_assembly = _DatabrowserAssembly(
            dep_dict={'saved_states':self.saved_states,
                      'cmd_line':self.cmd_line,
                      'local_vtk_data':self.local_vtk_data,
                      'vtk_slice':self.vtk_slice,
                      'the_fab_table':the_fab_table,
                      'synchronizer':self.synchronizer,
                      'dialog_inventory':self.dialog_inventory },
            component = component,
            axis = axis,
            format = self.format.get(),
            font_family = self.font_family,
            font_size = self.font_size,
            font_style = self.font_style,
            master = self.interior() )

        databrowser_assembly.configure(
            command = lambda dummy, widget=databrowser_assembly, self=self:
                self._databrowserCloseHandler( dummy, widget ))
        databrowser_assembly.bind(
            '<FocusIn>',
            lambda e, self=self, s=databrowser_assembly:
                self._databrowserFocusHandler( s ))

        self.databrowser_cltn.append( databrowser_assembly )
        databrowser_assembly.syncWithLaunchWidget( axis=axis,
                                                   format = self.format.get() )

        databrowser_assembly.showGUI()
        self.local_vtk_data.setNewDatabrowser(1)

        # We return it so that from the API it can be turned off.
        return databrowser_assembly


    def _databrowserCloseHandler( self, dummy, widget ):
        """
        This is the 'command' option to _DatabrowserAssembly's Pmw.Dialog
        base class.  The default is just to destroy the widget.  But we want,
        in addition, to turn off the box's outline in the vtk display and
        eliminate the widget from self.databrowser_cltn.
        """
        anag_utils.funcTrace()

        i = 0
        while self.databrowser_cltn[i] != widget:
            i = i + 1

        widget.the_fab_table.showActors( 'off' )
        widget.the_fab_table.setHighlightingActivated( 0 )
        widget.withdrawGUI()
        del self.databrowser_cltn[i]


    def _databrowserFocusHandler( self, databrowser ):
        """
        Arg databrowser is a reference to a certain _DatabrowserAssembly.
        Make it bright, and dim all the others.  (The one we make bright has
        presumably just gained the focus.)
        """
        anag_utils.funcTrace()

        old_autorender = self.vtk_vtk.getAutorender()
        self.vtk_vtk.setAutorender(0)

        for s in self.databrowser_cltn:
            if( (s.getLevel() != databrowser.getLevel())
            or (s.getBox() != databrowser.getBox())):
                #anag_utils.info( "Dimming (", s.getLevel(), ",",
                #                              s.getBox(), ")")
                # Might be a databrowser for the same level&box but a diff
                # component.
                s.getTheFabTable().showActors( status = 'dim' )
                s.interior().configure( bg='#FFFFFF' )
                s.setInFocus( 0 )

        #anag_utils.info( "Brightening (", databrowser.getLevel(), ",",
        #                              databrowser.getBox(), ")")
        databrowser.getTheFabTable().showActors( status = 'on' )
        databrowser.interior().configure( bg='#FF0000' )
        databrowser.setInFocus( 1 )

        self.vtk_vtk.setAutorender( old_autorender )
        self.vtk_vtk.render()


class _LevelAndBoxScales( Tkinter.Frame, SelfControl ):
    """
    Two scale widgets plus labels.  Box scale configures itself so that its
    max reflects the number of boxes under the indicated level.
    Also, box extents, hilite cell coords and value.

    Radiobuttons to select slicing plane -- x, y or z.
    """

    def __init__(self, parent, dep_dict, **kw ):
        anag_utils.funcTrace()
        Tkinter.Frame.__init__( self, parent, **kw )
        SelfControl.__init__( self, dep_dict, metadata=[
            { 'name':'level' },    # Scale widget
            { 'name':'box' },      # Scale widget
            { 'name':'box_var' },  # Scale widget's var
            { 'name':'box_extents_text' },
            { 'name':'cell_ijk_text' },
            { 'name':'cell_value_text'}
        ])

        level_frame = Tkinter.Frame( self )

        self.level = anag_megawidgets.EntryScale(
                         level_frame,
                         button_text = 'level',
                         length = g_scalelength,
                         discrete = 1,
                         scale_callback = self._levelScaleCallback,
                         dep_dict = {'saved_states':self.saved_states } )

        num_boxes = self.local_vtk_data.getReader().GetLevelNumBoxes( 0 )
        box_frame = Tkinter.Frame( self )
        self.box = anag_megawidgets.EntryScale(
                         box_frame,
                         button_text = 'box',
                         length = g_scalelength,
                         dep_dict = {'saved_states':self.saved_states },
                         discrete = 1 )
        self.box.configure( from_=0, to=num_boxes-1, resolution=1,
                             sliderlength=g_sliderlength )
        self.box.setScaleValue( 0 )

        self.level.configure(
            from_ = self.local_vtk_data.getMinAvailableLevel(),
            to = self.local_vtk_data.getMaxAvailableLevel(),
            resolution=1, sliderlength=g_sliderlength )

        level_frame.pack(anchor='w')
        self.level.pack()
        box_frame.pack(anchor='w')
        self.box.pack(anchor='w')

        # Info on highlighted cell.
        hilite_cell_frame = Tkinter.Frame( self )

        box_extents_frame = Tkinter.Frame( hilite_cell_frame )
        Tkinter.Label( box_extents_frame, text='box extents:   ' ).pack(
            side='left' )
        self.box_extents_text = Tkinter.Label( box_extents_frame, width=15,
                                               borderwidth=2,relief='groove' )
        self.box_extents_text.pack(side='left')
        box_extents_frame.pack()

        cell_index_frame = Tkinter.Frame( hilite_cell_frame )
        Tkinter.Label( cell_index_frame, text="hilite cell (i,j,k):" ).pack(
            side='left')
        self.cell_ijk_text = Tkinter.Label( cell_index_frame,width=15,
                                            borderwidth=2,relief='groove' )
        self.cell_ijk_text.pack(side='left')
        cell_index_frame.pack()

        cell_value_frame = Tkinter.Frame( hilite_cell_frame )
        Tkinter.Label( cell_value_frame, text="hilite cell value:" ).pack(
            side='left')
        self.cell_value_text = Tkinter.Label( cell_value_frame,width=15,
                                              borderwidth=2,relief='groove' )
        self.cell_value_text.pack( expand=1, fill=Tkinter.X )
        cell_value_frame.pack(expand=1,fill=Tkinter.X)

        hilite_cell_frame.pack( expand=1, fill=Tkinter.X, anchor='w' )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def getLevel( self ): return int(self.level.getScaleValue())

    def getBoxNum( self ): return int(self.box.getScaleValue())

    def setLevelBoxNumAndExtents( self, lg, extra_info ):
        """
        Callback for Notifier vtk_data.selected_box.
        Sets the level, box and box_extents widgets to describe the currently
        selected box.
        """
        level = lg[0]
        box   = lg[1]
        self.level.setScaleValue( level ) # Doesn't trigger its callback! ...
        self._levelScaleCallback( level ) # ... have to call it by hand!?
        self.box.setScaleValue( box )

        box_extents = self.local_vtk_data.getBoxExtents(level,box,1)
        fixedup_extents = str('{i:'+ str(box_extents['i']) +',') \
                        + str(' j:'+ str(box_extents['j']) )
        if self.local_vtk_data.is2DMode():
            fixedup_extents += '}'
        else:
            fixedup_extents += str(', k:'+str(box_extents['k'])+'}')
        self.box_extents_text.configure(text=fixedup_extents )
        w = len(fixedup_extents)
        self.box_extents_text.configure( width=int(w*2.4/3))#variable font


    def _levelScaleCallback( self, level ):
        anag_utils.funcTrace()
        num_boxes = self.local_vtk_data.getReader().GetLevelNumBoxes(int(level))
        self.box.configure( from_=0, to=num_boxes-1 )
        if num_boxes == 1:
           self.box.configure( length=g_sliderlength + 3 ) 
        else:
            self.box.configure( length=g_scalelength )


    def _doHiliteCellWork( self, a_cell_ijk ):
        """
        Called from showHiliteCellCoords() and showHiliteCellValue().  Returns
        a tuple of stuff those functions need.
        """
        anag_utils.funcTrace()
        cell_ijk = a_cell_ijk[:]
        level = self.local_vtk_data.getSelectedBox()[0]
        box   = self.local_vtk_data.getSelectedBox()[1]

        if self.local_vtk_data.isResliceMode():
            direction = self.local_vtk_data.getResliceDirection()
            pos = self.local_vtk_data.getReslicePosition()[direction]
            axis_nums = {'x':0,'y':1,'z':2}
            dx = self.local_vtk_data.getReader().GetLevelDx(level)[
                    axis_nums[self.local_vtk_data.getResliceDirection()]]
            cell_ijk[2] = int( pos / dx )
            cell_ijk = self.local_vtk_data.inversePermuteCoords( cell_ijk,
                            self.local_vtk_data.getResliceDirection() )
        
        box_extents = self.local_vtk_data.getBoxExtents(
                          level, box, permuted_in_reslice_mode=0 )
        cell_in_box = 1
        for ijk in 'i', 'j', 'k':
            index_num = (ijk=='j') + 2*(ijk=='k')
            if( not (box_extents[ijk][0] <= cell_ijk[index_num]
                                         <= box_extents[ijk][1]) ):
                cell_in_box = 0

        if cell_in_box:
            i = cell_ijk[0] - box_extents['i'][0]
            j = cell_ijk[1] - box_extents['j'][0]
            if( self.local_vtk_data.is2DMode()
            and not self.local_vtk_data.isResliceMode()):
                k = 0
            else:
                k = cell_ijk[2]-box_extents['k'][0]
        else:
            (i,j,k) = (None,None,None)

        return (cell_in_box, (i,j,k), level, box, cell_ijk )


    def showHiliteCellCoords( self, a_cell_ijk, format ):
        """
        Print out the (i,j,k) coordinates of the cell.
        """
        anag_utils.funcTrace()

        (cell_in_box, (i, j, k), level, box, cell_ijk) =\
            self._doHiliteCellWork( a_cell_ijk )

        if cell_in_box:
            if( self.local_vtk_data.is2DMode()
            and not self.local_vtk_data.isResliceMode()):
                self.cell_ijk_text.configure(text=
                    '(' + str(cell_ijk[0]) + ', ' + str(cell_ijk[1]) + ')')
            else:
                self.cell_ijk_text.configure(text=
                    '(' + str(cell_ijk[0]) + ', ' + str(cell_ijk[1]) 
                    + ', ' + str(cell_ijk[2]) + ')')

    def showHiliteCellValue( self, a_cell_ijk, component, format ):
        """
        Print out the value of the cell -- a good cheap alternative if
        the box is very large and launching a databrowser would take a
        long time.
        """
        anag_utils.funcTrace()

        (cell_in_box, (i, j, k), level, box, cell_ijk) =\
            self._doHiliteCellWork( a_cell_ijk )

        if cell_in_box:
            try:
              self.cell_value_text.configure(text=
                format % self.local_vtk_data.getReader().GetDatum(
                           level, box,
                           self.local_vtk_data.getComponentSerialNum(component),
                           i,j,k))
            except:
              tkMessageBox.showerror( message="Invalid floating point format: "
                  + format )
              return
            """
            anag_utils.info( "selected cell = ",
                '(' + str(cell_ijk[0]) + ', ' + str(cell_ijk[1]) 
                + ', ' + str(cell_ijk[2]) + '); value = ',
                format % self.local_vtk_data.getReader().GetDatum(
                           level, box,
                           self.local_vtk_data.getComponentSerialNum(component),
                           i,j,k))
            """


class _Synchronizer( SelfControl ):
    """
    Coordinates various modes of coordination -- between databrowsers,
    between databrowser planes and slice positions, etc.
    """
    def __init__(self, dep_dict ):
        SelfControl.__init__( self, dep_dict, metadata=
           [{'name':'page_name', 'notify':1}, # Databrowser tab visible
            {'name':'synchronization_on', 'get':1, 'set':1, 'initval':0,
             'save':2},
            {'name':'sync_button', 'get':1, 'set':1}
           ])

    def _initForFirstHDF5( self ):
        pass


    def _refresh( self ):
        if self.sync_button: # None, if no field components
            self.sync_button.set( self.getSynchronizationOn() )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def createSyncButton( self, parent_widget ):
        """
        Called from class ControlFabTables, so we can pack it inside a parent
        widget defined there.  Owned by this class so we can manage its
        persistence.
        """
        self.sync_button =  anag_megawidgets.Checkbutton(
            parent_widget,
            text = "synchronized" )
        self.sync_button.configure( command = lambda
            self=self, butt=self.sync_button:
                self.setSynchronizationOn( butt.get() ) )
        self.sync_button.set(
            self.getSynchronizationOn())
        self.sync_button.pack( anchor='w')


class _DatabrowserAssembly( anag_megawidgets.SelfDescribingDialog,
                            SelfControl ):
    """
    Contains all the databrowsers for this box, plus all the GUI controls.
    """

    def getShortDescription(self):
        return "Databrowser"
    def getLongDescription(self):
        return "Databrowser plus control widgets"

    def __init__( self, dep_dict,
                  component,
                  axis,   # initial slicing axis
                  format, # e.g. "%5.3f",
                  font_family, font_size, font_style,
                  master, **kw )    :
        anag_utils.funcTrace()

        #timer = anag_utils.Timer( '_DatabrowserAssembly ctor' )
        #timer.on()

        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict, metadata=
          [{'name':'notebooks','initval':{}}, #Keys:i,j,k. Values:Pmw.NoteBook
           {'name':'notebook_frame'}, # Holds the notebooks (one at a time).
           {'name':'databrowser_dict', 'initval':{}}, # One databrowser/tab
           {'name':'cur_browser', 'initval':None}, # _Databrowser instance.
           {'name':'scrollbar_position',
                   'initval':{"x":None,"y":None,"z":None} },
           {'name':'format'}, # Pmw.EntryField
           {'name':'slicing_axis'}, # x|y|z radiobuttons
           {'name':'kscale'}, # pick 3rd dimension position (rather than tabs)
           {'name':'kscale_label'}, # varies with the axis.
           {'name':'component', 'get':1, 'set':2}, # curcomponent
           {'name':'component_selector'}, # scrolled list
           {'name':'eb_component_selector'}, # if there's eb data
           {'name':'eb_component', 'initval':0}, # 0, 1, 2,... (the "interface")
           {'name':'eb_policy', 'initval':{}},
           {'name':'eb_policy_buttons'},
           {'name':'master'},
           {'name':'in_focus', 'get':1, 'set':1, 'initval':1},
           {'name':'transparent_plane_button'},
           {'name':'font_family'},
           {'name':'font_size'},
           {'name':'font_style'}
          ])
        self.decls = anag_utils.Declarations( "decls", instance=self )
        self.dialog_inventory.registerDialog( self )
        self.master = master
        self.component = component
        self.font_family = font_family
        self.font_size = font_size
        self.font_style = font_style

        self.eb_policy =  {g_eb_policy_tags.show_irregular:    1,
                           g_eb_policy_tags.show_regular:      1,
                           g_eb_policy_tags.show_covered:      0,
                           g_eb_policy_tags.hilite_irregular:  1 }

        self.wm_title( 'level=' + str(int(self.getLevel())) + ':' +
                       'box=' + str(self.getBox()) )

        # 3rd dimension scale ('k')
        kscale_frame = Tkinter.Frame( self.interior() )
        self.kscale_label = Tkinter.Label( kscale_frame )
        self.kscale = anag_megawidgets.Scale(
            kscale_frame,
            orient = 'horizontal', repeatdelay = 1000,
            length = g_scalelength, sliderlength=g_sliderlength,
            resolution=1, callback = 0 )
        if not self.local_vtk_data.is2DMode() or self.local_vtk_data.isResliceMode():
            self.kscale_label.pack( side='left' )
            self.kscale.pack( side='left' )

        # Toggle for transparent plane.  On by default.  It's convenient to turn
        # that plane off when we're in synchronized mode and the slice plane
        # itself shows where we are (and the transparent plane is then only an
        # annoyance).  I tried and failed at a strategy to switch this plane on
        # and off automatically.  The idea was to turn it off at the bottom of
        # _handleSlicingPlanePosition(), and turn it back on in
        # VtkFabTables.setPlane().  Unfortunately, the gui stuff happens on
        # multiple threads, and I found I couldn't count on the timing of events
        # to make things happen in the necessary order.
        self.transparent_plane_button = \
            anag_megawidgets.Checkbutton( kscale_frame,text='Show tracer plane')
        self.transparent_plane_button.configure(
            command = lambda self=self, butt = self.transparent_plane_button:
                self.the_fab_table.togglePlane( on_off= butt.get() ))
        if( (not self.cmd_line.getNoVtk()) 
        and (not self.local_vtk_data.isResliceMode())):
            self.transparent_plane_button.pack( anchor='se' )
        self.transparent_plane_button.set( 1 )


        # User-specifiable formatting code.
        bottom_frame = Tkinter.Frame( self.interior() )
        format_and_axis_frame = Tkinter.Frame( bottom_frame )
        format_and_axis_frame.pack(anchor='nw', side='left')
        format_frame  = Tkinter.Frame( format_and_axis_frame, borderwidth=2,
                                       relief='groove' )
        self.format = Pmw.EntryField( format_frame,
                                      value = format, # ctor arg
                                      labelpos='w', label_text="format:")
        self.format.component('entry').configure( width=6 )
        self.format.configure( command = lambda self=self:
            self._formatHandler( self.format.get() ))

        # Slicing axis radiobuttons.
        axis_frame = Tkinter.Frame( format_and_axis_frame,
            borderwidth=2, relief='groove')
        self.slicing_axis = anag_megawidgets.RadioSelect(
            axis_frame,
            buttontype = 'radiobutton',
            orient = 'horizontal',
            command = self._slicingAxisHandler,
            labelpos = 'w',
            label_text = 'normal:'
            )
        map( self.slicing_axis.add, ('x','y','z') )
        self.slicing_axis.setcurselection( axis )
        # But we call invoke() a little later.  We need to setcurselection here
        # so that all our databrowsers get their axis set to something valid.


        # Component selector.
        self.component_selector = Pmw.ScrolledListBox(
            bottom_frame,
            labelpos='n', label_text='component:')
        self.component_selector.component('hull').configure(
            borderwidth=2, relief='groove' )
        component_names = self.local_vtk_data.getComponentList()
        self.component_selector.setlist( component_names )
        self.component_selector.component('listbox').configure(
            height = min(3, len(component_names)), exportselection=0 )
        self.component_selector.configure(
            selectioncommand = lambda self=self, axis=axis:
                self._componentSelectionHandler( self.component_selector ))
        self.component_selector.select_set(
            self.local_vtk_data.getComponentSerialNum( self.component ))


        #
        # EB stuff
        #
        if self._getNumCoveredCellInfoComponents() > 0:
            eb_frame = Tkinter.Frame( bottom_frame,
                borderwidth=2, relief='groove' )

            # EB component selector.
            self.eb_component_selector = Pmw.ScrolledListBox(
                eb_frame,
                labelpos='n', label_text='Embedded boundary')
            self.eb_component_selector.configure( selectioncommand =
                self._changeEBComponent )
            boundary_items = []
            for i in range(0, self._getNumCoveredCellInfoComponents()):
                boundary_items.append( str(i) )
            self.eb_component_selector.setlist( boundary_items )
            self.eb_component_selector.component('listbox').configure(
                height = min(3, len(boundary_items)), exportselection=0 )
            self.eb_component_selector.select_set( '0' )
            self.eb_component_selector.pack(side='left')

            # EB policy
            self.eb_policy_buttons = anag_megawidgets.RadioSelect(
                eb_frame,
                buttontype = 'checkbutton',
                orient = 'vertical',
                command = self._handleEBPolicy )
            map( self.eb_policy_buttons.add,
                    ( g_eb_policy_tags.show_irregular,
                      g_eb_policy_tags.show_regular,
                      g_eb_policy_tags.show_covered,
                      g_eb_policy_tags.hilite_irregular ) )
            self.eb_policy_buttons.pack(side='left')
            on_button_list = []
            for k in self.eb_policy.keys():
                if self.eb_policy[k] == 1:
                    on_button_list.append(k)
            self.eb_policy_buttons.setvalue(on_button_list)

        # Frame for the notebooks (only one notebook is visible at a time).
        # A notebook is one of those things with tabs sticking out the top.
        self.notebook_frame = Tkinter.Frame( self.interior() )
        if ( not self.local_vtk_data.is2DMode()
             or self.local_vtk_data.isResliceMode() ):
            for axis in 'x','y','z':
                self.notebooks[axis] = Pmw.NoteBook(
                    self.notebook_frame,
                    raisecommand = lambda page_name, self=self, axis=axis:
                        self._notebookPageRaiseCommand( axis, page_name ),
                    borderwidth=3 )
                if   axis == 'x':
                    tabs = self._assembleMarginTags( 'z', 'top' )
                elif axis == 'y':
                    tabs = self._assembleMarginTags( 'x', 'top' )
                elif axis == 'z':
                    tabs = self._assembleMarginTags( 'x', 'left' )
                    tabs.reverse()
                for item in tabs:
                    self.notebooks[axis].add( item )


        else:
            self.notebooks['z'] = Pmw.NoteBook(
                self.notebook_frame,
                raisecommand = lambda page_name, self=self:
                    self._notebookPageRaiseCommand( 'z', page_name ),
                borderwidth=3,
                tabpos = None )

            # This spends 99% of its time in _pageRaiseCommand():
            self.notebooks['z'].add( 'k=0' )


        # This is where the databrowser gets filled with data:
        self.slicing_axis.invoke( axis )

        # Pack things.
        self.format.pack()
        format_frame.pack( side='top', anchor='nw')
        if(   not self.local_vtk_data.is2DMode()
        or    self.local_vtk_data.isResliceMode() ):
            self.slicing_axis.pack()
            axis_frame.pack( side='top', anchor='nw' )
            kscale_frame.pack( expand=0, fill=Tkinter.X )
        self.component_selector.pack(side='left', anchor='nw')
        if self._getNumCoveredCellInfoComponents() > 0:
            self.eb_component_selector.pack()
        self.notebook_frame.pack( expand=1, fill=Tkinter.BOTH )
        if self._getNumCoveredCellInfoComponents() > 0:
            eb_frame.pack(side='left')
        bottom_frame.pack( expand=0, fill=Tkinter.X )

        # Manage box dimming in response to focus.
        self.the_fab_table.showActors( status = 'on' )

        # Move plane to position corresponding to slicing plane position.
        if( ( (not self.local_vtk_data.is2DMode()) 
        or  (self.local_vtk_data.isResliceMode()))
        and (not self.cmd_line.getNoVtk())):
            for axis in 'x','y','z':
                if self.local_vtk_data.isResliceMode():
                    direction = self.local_vtk_data.getResliceDirection()
                    position =\
                        self.local_vtk_data.getReslicePosition([direction])
                else:
                    direction = axis
                    position = self.vtk_slice.getSlicingPlanes(
                                    [axis+'0']).getPlanePosition()
                if self.local_vtk_data.getCurComponent():
                    if( self.local_vtk_data.planeIntersectsBox(
                      self.getLevel(), self.getBox(), axis, position )):
                        self._syncBrowserToSlicePosition( direction, position )

        # Gotta call this, to register with notifiers.  Classes constructed at
        # init time have their _refresh() method called then.  But this class
        # isn't constructed at init time, so we have to call _refresh() manually
        # here.
        self._refresh()

        self.withdrawGUI()

        #timer.stop()


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()


    def _refresh( self ):
        anag_utils.funcTrace()

        self.registerCallback(
            self.synchronizer.getNotifierPageName(),
            self._pageNameHandler )
        self.registerCallback(
            self.local_vtk_data.getNotifierSelectedCell(),
            self._handleSelectedCell )
        self.registerCallback(
            self.local_vtk_data.getNotifierResliceDirection(),
            self._handleResliceDirection )
        self.registerCallback(
            self.local_vtk_data.getNotifierResliceNotifier(),
            self._handleReslicePosition )

        if( (not self.local_vtk_data.is2DMode())
        and (not self.cmd_line.getNoVtk()) ):
            for axis in 'x','y','z':
                self.registerCallback(
                    self.vtk_slice.getSlicingPlanes([axis+'0']
                                               ).getNotifierPlanePosition(),
                    lambda position, dummy, axis=axis, self=self:
                        self._handleSlicingPlanePosition( axis, position ))

        self.registerCallback( self.synchronizer.getNotifierPageName(),
                               self._pageNameHandler )
        self.registerCallback(
            self.local_vtk_data.getNotifierNewComponentNotifier(),
            self._newComponentNotifierHandler )


    def _newComponentNotifierHandler( self, compname, unused ):
        anag_utils.funcTrace()
        anag_megawidgets.appendItemToPmwScrolledListBox(
            compname, self.component_selector, 10 )
        

    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        for k in self.databrowser_dict.keys():
            self.databrowser_dict[k].cleanup()
        self.zeroSelfVariables()
        self.dialog_inventory.unregisterDialog( self )
        self.withdrawGUI()


    def _componentSelectionHandler( self, component_selector ):
        """ Handler for selection events in the component-choice menu """
        anag_utils.funcTrace()
        self.setComponent( component_selector.getcurselection()[0] )
        self.local_vtk_data.setNewDatabrowser(1)


    def _handleEBPolicy( self, button_name, on_off ):
        anag_utils.funcTrace()
        self.eb_policy[button_name] = on_off
        if self.eb_policy[g_eb_policy_tags.show_irregular] == 0:
            self.eb_policy_buttons.button(3).configure( state='disabled' )
        else:
            self.eb_policy_buttons.button(3).configure( state='normal' )
        for j in self.databrowser_dict.keys():
            databrowser = self.databrowser_dict[j]
            databrowser.formatText( self.format.get(), self.eb_policy )


    def setComponent( self, component ):
        """
        Besides setting self.component, refresh the currently visible notebook
        page (an instance of class _Databrowser) to reflect the new component.
        """
        anag_utils.funcTrace()
        if component != self.component:
            self.component = component
            axis = self.cur_browser.getAxis()
            self._notebookPageRaiseCommand(
                axis,
                self._generatePagename( axis, self.kscale.get() ))


    def _changeEBComponent( self ):
        """ Handler for EB component selector """
        anag_utils.funcTrace()
        axis = self.cur_browser.getAxis()

        eb_component = self.eb_component_selector.getvalue()[0]
        changed_eb = (eb_component != self.eb_component)
        self.eb_component = eb_component
        self._notebookPageRaiseCommand(
            axis, self._generatePagename( axis, self.kscale.get() ), None,
            changed_eb = changed_eb )


    def _generatePagename( self, axis, num ):
        """
        Generate the labels that go on the notebook tabs, e.g. "k=2".
        """
        anag_utils.funcTrace()
        xyz2ijk = {'x':'i', 'y':'j', 'z':'k'}
        result = xyz2ijk[axis] + '=' + str(num)
        return result


    def _pageNameHandler( self, page_name, extra_info ):
        """
        Callback for _Synchronizer.page_name -- gets tripped whenever a
        _DatabrowserAssembly has a new tab ("k-scale" element) exposed.
        In respose, _DatabrowserAssembly's of the same level&box but different
        components expose the same tab.
        """
        anag_utils.funcTrace()

        if self.synchronizer.getSynchronizationOn() == 0:
            return

        if( (extra_info['level'] == self.getLevel())
        and (extra_info['box']   == self.getBox())
        and (extra_info['axis'] == self.slicing_axis.getcurselection() )
        and (extra_info['component'] != self.component) ):
            self._notebookPageRaiseCommand(
                axis=extra_info['axis'],
                page_name=page_name,
                responding_to_notification=1 )


    def _handleReslicePosition( self, dummy1, dummy2 ):
        """
        Raise the notebook tab, i.e. _Databrowser instance, that corresponds to
        the reslice position.
        """
        anag_utils.funcTrace()

        direction = self.local_vtk_data.getResliceDirection()
        position  = self.local_vtk_data.getReslicePosition([direction])

        if( (not self.local_vtk_data.planeIntersectsBox(
            self.getLevel(), self.getBox(), direction, position ))
        or  (self.synchronizer.getSynchronizationOn() == 0) ):
            return
        else:
            self._syncBrowserToSlicePosition( direction, position )


    def _handleSlicingPlanePosition( self, axis, position ):
        """
        Raise the notebook tab, i.e. _Databrowser instance, that corresponds to
        the slice position.  Just like _handleReslicePosition, but for 3D mode.
        """
        anag_utils.funcTrace()

        if( (not self.local_vtk_data.planeIntersectsBox(
            self.getLevel(), self.getBox(), axis, position ))
        or  (axis != self.slicing_axis.getcurselection())
        or  (self.synchronizer.getSynchronizationOn() == 0) ):
            return
        else:
            self._syncBrowserToSlicePosition( axis, position )


    def _syncBrowserToSlicePosition( self, slice_direction, slice_position ):
        """
        Raise the notebook tab, i.e. _Databrowser instance, that corresponds to
        the slice position.
        """
        anag_utils.funcTrace()

        xyz = [0,0,0]
        axis = slice_direction
        axis_num = (axis=='y') + 2 * (axis=='z')
        axis_ijk = {'x':'i', 'y':'j', 'z':'k' }
        xyz[axis_num] = slice_position
        ijk = list(self.local_vtk_data.xyz2ijk( xyz, self.getLevel() ))
        box_hi = self.local_vtk_data.getBoxExtents(
                    self.getLevel(), self.getBox(),
                    permuted_in_reslice_mode=0 )[ axis_ijk[axis] ][1]
        ijk[axis_num] = min( ijk[axis_num], box_hi )
        page_name = self._generatePagename( axis, ijk[axis_num] )
        self.notebooks[axis].selectpage( page_name )
        

    def _handleResliceDirection( self, dummy1, dummy2 ):
        """
        When vtk_data.reslice_direction changes, we set the normal (x|y|z)
        radiobutton of the DatabrowserAssembly accordingly.
        """
        anag_utils.funcTrace()

        axis = self.slicing_axis.getcurselection()
        reslice_direction = self.local_vtk_data.getResliceDirection()
        if axis != reslice_direction:
            self.slicing_axis.invoke( reslice_direction )


    def _getVisibleDatabrowser( self ):
        """
        Returns the _Databrowser that is currently on top of the currently
        packed notebook.
        """
        for axis in 'x','y','z':
            try:
                self.notebooks[axis].pack_info()
                k = self.notebooks[axis].getcurselection()
                return self.databrowser_dict[k]
            except:
                pass


    def _handleSelectedCell( self, dummy1, dummy2 ):
        """
        Callback for Notifier vtk_data.selected_cell.

        Looks for open databrowsers that cover the position of the selected
        cell, and which are (in reslice mode) oriented the same way as 
        vtk_data.getResliceDirection().  For all those databrowsers, deletes
        any existing hilite, and then hilites current selected cell in blue.

        In 3D mode, hilites databrowser cell only if kscale position is same
        as position of slicing plane corresponding to the same axis as the
        orientation of the databrowser.

        The semantics of cell hiliting are deliberately simple and unambitious.
        We don't try to hilite cells in databrowser planes not the same as
        vtk_data.getResliceDirection().  We don't try to hilite cells of
        databrowsers opened after the cell was selected (for that, the user can
        just do another ctrl-button1).  We don't try to rehilite after a change
        of numerical format.
        """
        anag_utils.funcTrace()

        cell_ij = self._domainCoords2DatabrowserCoords(
            self.local_vtk_data.getSelectedCell())

        for k in self.databrowser_dict.keys():
            self.databrowser_dict[k].deleteHighlightCell( self.format.get(),
                                                          self.eb_policy )

        if cell_ij:
            databrowser = self._getVisibleDatabrowser()
            direction = self.local_vtk_data.getResliceDirection()
            if( (not self.local_vtk_data.isResliceMode())
            or  ( ( (databrowser.getAxis() ==
                     self.local_vtk_data.getResliceDirection()))
                and (self.local_vtk_data.planeIntersectsBox(
                        self.getLevel(), self.getBox(),
                        direction,
                        self.local_vtk_data.getReslicePosition([direction]))))):
                databrowser.highlightCell( cell_ij, self.format.get(),
                                           self.eb_policy )


    def _domainCoords2DatabrowserCoords( self, absolute_cell_ijk ):
        """
        Convert cell (i,j,k) coordinates from absolute (i.e. relative to the
        lower LH corner of the domain) to the system used in a databrowser
        plane (observing the right-hand-coordinate-system convention, and the
        rule that the index along the side of the databrowser should grow
        upward).

        Check that cell's level&box are the same as those of this
        DatabrowserAssembly widget.  In 3D mode, check also that the "k-scale"
        value is the same as absolute_cell_ijk[2], that is, that the plane
        visible in the databrowser corresponds to the position of the slicing
        plane in the VTK window.

        Return the pair of relative-to-box coordinates, or None if arg
        absolute_cell_ijk isn't in this _DatabrowserAssembly's box, or (in 3D
        mode) if the "k-scale" doesn't correspond to the slicing plane's
        position.
        """
        anag_utils.funcTrace()

        assert( absolute_cell_ijk )

        if not self.local_vtk_data.is2DMode():
            browser_axis = self.slicing_axis.getcurselection()
        else:
            browser_axis = 'z'


        # Check that the databrowser is for the box that's highlighted on the
        # selector ray.
        if( (self.getLevel() != self.local_vtk_data.getSelectedBox()[0])
        or  (self.getBox() != self.local_vtk_data.getSelectedBox()[1]) ):
            return None

        # Check that, in 3D mode, the databrowser's plane corresponds to the
        # position of the slicing plane in the VTK window.
        axis_nums = {'x':0, 'y':1, 'z':2}
        if not self.local_vtk_data.is2DMode():
            if self.kscale.get() != absolute_cell_ijk[axis_nums[browser_axis]]:
                return None

        box_extents = self.local_vtk_data.getBoxExtents( self.getLevel(),
                                                   self.getBox() )
        result = [0,0] # [i,j]


#       We could do the following without the three cases, but it would mean
#       really hairy double-indirect subscripting.
        if browser_axis == 'z':
            result[0] = absolute_cell_ijk[0] - box_extents['i'][0]
            result[1] = box_extents['j'][1] - absolute_cell_ijk[1] 
        elif browser_axis == 'y':
            result[0] = absolute_cell_ijk[2] - box_extents['k'][0]
            result[1] = box_extents['i'][1] - absolute_cell_ijk[0]
        else:
            result[0] = absolute_cell_ijk[1] - box_extents['j'][0]
            result[1] = box_extents['k'][1] - absolute_cell_ijk[2] 

        return result


    def syncWithLaunchWidget( self, axis, format ):
        """
        This is for when the databrowser launcher widget is asked for a
        databrowser that already exists.  We don't construct a new one.  We just
        .show() the existing one.  But we also want to change its slicing axis
        and its format, if necessary, so they are the same as requested in the
        launcher widget.
        """
        anag_utils.funcTrace()

        current_axis = self.slicing_axis.getcurselection()
        if current_axis != axis:
            self.slicing_axis.invoke( axis )

        if self.format.get() != format:
            self.format.setentry( format )
            self._formatHandler( format )


    def getTheFabTable( self ):
        """
        self.the_fab_table comes through dep_dict arg to SelfControl ctor.
        """
        anag_utils.funcTrace()
        result = self.the_fab_table
        return result


    def getLevel( self ):
        anag_utils.funcTrace()
        result = self.the_fab_table.getLevel()
        return result


    def getBox( self ):
        """ Returns the box number, within the level. """
        anag_utils.funcTrace()
        result = int(self.the_fab_table.getBox())
        return result

    def _formatHandler( self, format ):
        """ Callback for the format Entry widget. """
        anag_utils.funcTrace()
        for j in self.databrowser_dict.keys():
            databrowser = self.databrowser_dict[j]
            databrowser.formatText( format, self.eb_policy )


    def _notebookPageRaiseCommand( self, axis, page_name,
                                   responding_to_notification=None,
                                   changed_eb=None ):
        """
        Put a databrowser on this page, if there isn't one already.

        Arg responding_to_notification helps avoid an infinite recursion.
        """
        anag_utils.funcTrace()
        #timer = anag_utils.Timer( '_notebookPageRaiseCommand()' )
        #timer2 = anag_utils.Timer('_notebookPageRaiseCommand() interior')
        #timer.on()

        #
        # Record position we left the previous page (i.e. browser) at.
        # Variable self.cur_browser is the current browser, except for a while
        # in this function, while we're setting up the browser that is to be
        # the current one.
        #
        if self.cur_browser:
            self.scrollbar_position[ self.cur_browser.getAxis() ] = \
                self.cur_browser.getScrollbarPositions()

        this_page = self.notebooks[axis].page(page_name)
        this_tab =  self.notebooks[axis].tab(page_name)

        # Construct the notebook pages (each one an instance of _Databrowser)
        # only as needed.  And one is needed if either (1) it has never been
        # constructed, or (2) it has, but it was constructed when the current
        # component for this _DatabrowserAssembly was something different than
        # it is now.
        if( (self.databrowser_dict.has_key( page_name ) )
        and (
             (self.databrowser_dict[page_name].getComponent() !=self.component)
          or
             changed_eb )):
            self.databrowser_dict[page_name].forget()

        if(( not self.databrowser_dict.has_key( page_name ) )
        or ( self.databrowser_dict[page_name].getComponent() !=self.component)
        or changed_eb ):
            databrowser = _Databrowser( master = this_page,
                                        dep_dict = {},
                                        axis = axis, component=self.component,
                                        font_family=self.font_family,
                                        font_size=self.font_size,
                                        font_style=self.font_style )
            databrowser.pack( expand=1,fill=Tkinter.BOTH )

            #timer2.on()
            axis_position = int(page_name.split('=')[1])

            if self._getNumCoveredCellInfoComponents() > 0:
                eb_data = self.the_fab_table.getBoxData(
                    g_eb_discriminator_name + str(self.eb_component),
                    axis, axis_position )
            else:
                eb_data = None

            databrowser.displayNewData(
                format = self.format.get(),
                data = self.the_fab_table.getBoxData(
                    self.component,
                    axis, axis_position ),
                eb_data = eb_data,
                eb_policy = self.eb_policy,
                left_margin = self._assembleMarginTags( axis, 'left' ),
                top_margin = self._assembleMarginTags( axis, 'top' ) )

            #timer2.stop()

            self.databrowser_dict[page_name] = databrowser

        else:
            databrowser = self.databrowser_dict[page_name]


        # Color the selected tab.
        pagenames = self.notebooks[axis].pagenames()
        if( not self.local_vtk_data.is2DMode() 
        or self.local_vtk_data.isResliceMode() ):
            for item in pagenames:
                tab = self.notebooks[axis].tab(item)
                tab.configure( bg='#DDDDDD' )
            this_tab.configure( bg='#FF0000' )

        # Set the scale to the currently visible tab.
        self.kscale.set( int(page_name.split('=')[1]))

        # Set position to previous one saved from a browser for this axis.
        position = self.scrollbar_position[ databrowser.getAxis() ]
        if position:
            databrowser.setScrollbarPositions( position )


        # Trip the Notifier that synchronizes databrowsers of other components
        # but of this box.
        if not responding_to_notification:
            self.synchronizer.setPageName(
                page_name,
                extra_info= { 'component':self.component,
                              'level':self.getLevel(),
                              'box':self.getBox(),
                              'axis':databrowser.getAxis() })

        self.cur_browser = databrowser

        #timer.stop()


    def _getNumCoveredCellInfoComponents( self ):
        """
        Doesn't work like VtkData.getNumEBs(), which counts the number of
        fraction-* components.  This one counts the number of components named
        __covered-*.  When we don't have old-fashioned type EB data, the user is
        expected to provide such __covered-* components, in which 0 means
        covered, 1 means irregular, and 2 means regular.
        """
        anag_utils.funcTrace()
        comps = self.local_vtk_data.getComponentList()
        i = 0
        while g_eb_discriminator_name + str(i) in comps:
            i += 1
        return i        


    def _slicingAxisHandler( self, axis ):
        """
        Reorient the databrowser, so it shows a slice in another plane.
        """
        anag_utils.funcTrace()
        assert( axis=='x' or axis=='y' or axis=='z' )

        for a in self.notebooks.keys(): self.notebooks[a].forget()
        self.notebooks[axis].pack(fill=Tkinter.BOTH,expand=1)

        # Configure the kscale and its label
        axis_data = {'x':'i', 'y':'j', 'z':'k'}
        krange = self.the_fab_table.getKRange( axis )
        page_name = self.notebooks[axis].getcurselection()
        page_num = int(page_name.split('=')[1])
        self.kscale.configure( from_= krange[0], to = krange[1] )
        self.kscale.setCallback( lambda x, axis=axis, self=self:
                self._kscaleCallback( x, axis ))
        self.kscale.setSterile( page_num )
        self._kscaleCallback( page_num, axis )
        self.kscale_label.configure( text = axis_data[axis] + ': ' +
            str(krange[0]) + ',...,' + str(krange[1]) )
        # We called kscale.setSterile() followed by kscaleCallback() because
        # kscale.set() doesn't reliably trigger the callback.

        # This is to ensure that the scrollbar positions are recorded:
        self._notebookPageRaiseCommand( axis, page_name )
        # (That call accounts for almost all the time in this function.)


    def _kscaleCallback( self, pos, axis ):
        anag_utils.funcTrace()
        self.notebooks[axis].selectpage(
            self._generatePagename( axis, pos ))

        # Set the transparent plane.
        self.the_fab_table.setPlane( axis, self.kscale.get() )


    def _assembleMarginTags( self, axis, margin ):
        """
        Return a list of the labels that should go along either the left 
        or the top margin of the databrowser.
        """
        assert( margin=='left' or margin=='top' )

        index_data = {'x':('z','y'),
                      'y':('x','z'),
                      'z':('y','x')}
        axis_data = {'x':'i', 'y':'j', 'z':'k'}

        result = []

        if margin == 'left':
            result_range = self.the_fab_table.getKRange(index_data[axis][0])
            for i in range( 0, result_range[1] - result_range[0] + 1 ):
                result.append( self._generatePagename(
                    index_data[axis][0], result_range[1] - i ))
        else:
            result_range = self.the_fab_table.getKRange(index_data[axis][1])
            for i in range( result_range[0], result_range[1]+1 ):
                result.append( self._generatePagename(
                    index_data[axis][1], i ))

        return result


class _Databrowser( Tkinter.Frame, SelfControl ):
    """
    A 2D array of numbers and margin tags.

    The GUI controls, and ownership of the other planes of data, all go into
    class _DatabrowserAssembly.
    """

    def __init__( self, master, dep_dict, axis, component,
                  font_family, font_size, font_style, **kw ):
        """ Sets up infrastructure, but doesn't display anything.
        """
        anag_utils.funcTrace()

        Tkinter.Frame.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict, metadata=
          [
            { 'name':'the_data'},    # Pmw.ScrolledText
            { 'name':'top_legend'},  # Tkinter.Text
            { 'name':'left_legend'},  # Tkinter.Text
            { 'name':'data', 'initval':None}, # Array of numbers
            { 'name':'eb_data', 'initval':None}, # Determines covered etc status
            { 'name':'datum_width'},  # Of entry in main part of databrowser
            { 'name':'left_margin'}, # list of strings, e.g. ['i=0','i=1']
            { 'name':'top_margin'},  # list of strings
            { 'name':'cell_ijk', 'initval':None}, # currently highlighted
            { 'name':'axis', 'get':1}, # Axis normal to 'x','y' or 'z'
            { 'name':'component', 'get':1},
            { 'name':'font_family'},
            { 'name':'font_size'},
            { 'name':'font_style'}
          ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'master', 'dep_dict',
            'kw', 'metadata', 'light_grey', 'axis', 'component',
            'font_family', 'font_size', 'font_style' )

        self.font_family = font_family
        self.font_size = font_size
        self.font_style = font_style
        self.axis = axis
        self.component = component

        light_grey = '#DDDDDD'

        self.rowconfigure( 0,weight=0 )
        self.rowconfigure( 1,weight=1 )
        self.columnconfigure( 0,weight=0 )
        self.columnconfigure( 1,weight=1 )

        self.the_data = Pmw.ScrolledText( self, text_wrap='none',
            vscrollmode='static', hscrollmode='static',
            columnheader=1, rowheader=1 )
        self.the_data.component('text').configure(
            font=(self.font_family, self.font_size, self.font_style) )
        # Scrollmodes are 'static' to ensure that the width and height of the
        # data widget always match those of top_legend and left_legend.  Else
        # they tend to go out of sync with each other.
        self.the_data.component('vertscrollbar').configure(
            command = self._vertscroller )
        self.the_data.component('horizscrollbar').configure(
            command = self._horizscroller )
        self.the_data.grid( row=1,column=1,sticky=Tkinter.NSEW )

        self.top_legend = self.the_data.component('columnheader')
        self.top_legend.configure(
            font=(self.font_family, self.font_size, self.font_style) )
        # If you don't commit yourself and set hscrollmode to either 'static'
        # or 'none', then under certain circumstances Pmw doesn't know what to
        # do and just vibrates wildly between showing and not showing a
        # scrollbar.  We use 'static' here only to get the side/top_legend
        # and the the_data to have the same size.
        self.left_legend = self.the_data.component('rowheader')
        self.left_legend.configure(
            font=(self.font_family, self.font_size, self.font_style) )

        decls.memberFunctionAudit(self)


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def _horizscroller(self, a,b,c=None):
        """ Handler for the horizontal scrollbar, by which we arrange to move
            both the_data and and top_legend at the same time.
        """
        anag_utils.funcTrace()
        # WARNING: I'm using a private method of Pmw.ScrolledFrame here.
        self.the_data._textbox.xview(a,b,c)


    def _vertscroller(self, a,b,c=None):
        """ Handler for the vertical scrollbar, by which we arrange to move
            both the_data and and left_legend at the same time.
        """
        anag_utils.funcTrace()
        self.the_data._textbox.yview(a,b,c)


    def deleteHighlightCell( self, format, eb_policy ):
        """ Delete the existing highlight, if there is one. """
        anag_utils.funcTrace()

        if self.cell_ijk:
            self._highlightCell( self.cell_ijk, format, colortag='none',
                                 eb_policy=eb_policy)
            self.cell_ijk = None


    def highlightCell( self, cell_ijk, format, eb_policy ):
        anag_utils.funcTrace()

        assert( cell_ijk )
        self.cell_ijk = cell_ijk
        self._highlightCell( self.cell_ijk, format, colortag='blue',
                             eb_policy=eb_policy )


    def _highlightCell( self, cell_ijk, format, colortag, eb_policy ):
        """
        Called from highlightCell().  Does the real work.
        """
        anag_utils.funcTrace()

        i = cell_ijk[1]
        j = cell_ijk[0]

        try: # Bail out if [i][j] are out of range.
            item = format % self.data[i][j]
        except:
            return

        if not self._cellIsHiddenForEB(i,j,eb_policy):
            i_pos_begin = cell_ijk[0] * (self.datum_width+1)
            i_pos_end   = (1+cell_ijk[0]) * (self.datum_width+1)
            j_pos = 1 + cell_ijk[1]
            tkinter_text = self.the_data.component('text')
            tkinter_text.delete(
                '%d.%d' % (j_pos, i_pos_begin),
                '%d.%d' % (j_pos, i_pos_end))
            tkinter_text.insert(
                '%d.%d' % (j_pos,i_pos_begin),
                item + (self.datum_width-len(item))*' ' + ' ', colortag)

        self.scrollToSelectedCell( cell_ijk )


    def _cellIsHiddenForEB( self, i, j, eb_policy ):
        """
        Returns True if the cell is covered and eb_policy is for covered cells
        to be hidden; or the cell is regular and eb_policy is for regular cells
        to be hidden; or the cell is irreular and eb_policy is for irregular
        cells to be hidden.
        """
        anag_utils.funcTrace()
        show_covered   = eb_policy[g_eb_policy_tags.show_covered]
        show_regular   = eb_policy[g_eb_policy_tags.show_regular]
        show_irregular = eb_policy[g_eb_policy_tags.show_irregular]

        return ( self.eb_data
            and
               (   ((show_covered==0)   and (self.eb_data[i][j]==0))
                or ((show_regular==0)   and (self.eb_data[i][j]==2))
                or ((show_irregular==0) and (self.eb_data[i][j]==1)) )
               )


    def getScrollbarPositions( self ):
        """
        Return a 2-tuple containing the left edge of the two scrollbars --
        horizontal and vertical.  The units are fractions of the length of the
        zone within which the scrollbars can slide.
        """
        anag_utils.funcTrace()
        result = [0,0]
        result[0] = self.the_data.component('horizscrollbar').get()[0]
        result[1] = self.the_data.component('vertscrollbar').get()[0]
        result = tuple(result)        
        return result


    def setScrollbarPositions( self, position ):
        """
        Arg position is a 2-tuple indicating the left edge of the two scrollbars
        -- horizontal and vertical.
        """
        anag_utils.funcTrace()
        self._horizscroller( 'moveto', position[0], None )
        self._vertscroller( 'moveto', position[1]+0.00001, None )


    def scrollToSelectedCell( self, cell_i_j ):
        """
        Move the scrollbars so the selected cell (colored blue) is visible.
        """
        anag_utils.funcTrace()

        # It doesn't seem to work, if we try using self.the_data.component(
        # 'horizscrollbar').set().  That's why I'm going to a lower level here.
        slider_range = self.the_data.component('horizscrollbar').get()
        slider_relative_size = slider_range[1] - slider_range[0]
        i_frac = float(cell_i_j[0])/len(self.data[0]) - slider_relative_size/2

        slider_range = self.the_data.component('vertscrollbar').get()
        slider_relative_size = slider_range[1] - slider_range[0]
        j_frac = float(cell_i_j[1])/len(self.data) - slider_relative_size/2

        self._horizscroller( 'moveto', i_frac, None )
        self._vertscroller( 'moveto', j_frac+0.00001, None )


    def displayNewData( self, format, data, eb_data, 
                        eb_policy, left_margin, top_margin ):
        """
        Show the databrowser (the constructor doesn't display anything).
        Called repeatedly, every time we want to modify the data.

        Arg format is, e.g. %4.3f, %6.2e, %g -- any legal format for the
        Python % operator.  (See _Learning Python_, p.41.)

        We want the indices' directions to reflect the layout of a 2D rendering.
        For a 2D dataset, that means indices i run across the top and j down
        the side (decreasing downwards).  For resliced 3D data, we present the
        indices in a way that reflects a right-hand coordinate system.
        """
        anag_utils.funcTrace()

        # Reject arg data if wrong type:
        if( type(data) != types.ListType
        and type(data) != types.TupleType
        and type(data[0]) != types.ListType
        and type(data[0]) != types.TupleType ):
            anag_utils.error( "Arg data must be a list of lists or ",
                              "a tuple of tuples." ) # tuple of lists OK too
            return

        self.data = data
        if eb_data:
            self.eb_data = eb_data
        else:
            self.eb_data = None
        self.left_margin = left_margin[:]
        self.top_margin = top_margin[:]

        # FIXME: I'd like to disable user editing, but this stuff makes the
        # whole data display disappear.
#        self.the_data.component('text').configure( state='normal' )
        self.formatText( format, eb_policy )
#        self.the_data.component('text').configure( state='disabled' )


    def formatText( self, format, eb_policy ):
        """
        Set the text in the ScrolledText widgets.
        """
        anag_utils.funcTrace()
        #timer = anag_utils.Timer( 'formatText' )
        #timer.on()

        try:
            teststr = format % 3.14
        except:
            tkMessageBox.showerror( message="Invalid floating point format: "
                + format )
            return

        # Find the widest cell, so you can set all the cells to the same size.
        # You can't count on the width being in format[1]; legal formats
        # don't have to include the size.  E.g. %e is also a legal format.
        # (But if the format does specify the width, then we do use that.)
        format_specifies_length=0
        try:
            self.datum_width, format_specifies_length =\
                self._findMaxWidth( format )
        except TypeError:
            anag_utils.error( 'Invalid format' )
            anag_utils.excepthook()
            return

        peagreen = '#ADB800'
        grey = '#AAAAAA'
        blue = '#AAAAFF'
        
        # Main data area.
        self.the_data.clear()
        tkinter_text = self.the_data.component('text')
        tkinter_text.tag_config( 'peagreen', background=peagreen )
        tkinter_text.tag_config( 'grey', background=grey )
        tkinter_text.tag_config( 'blue', background=blue )
        tkinter_text.tag_config( 'none', background='#EEEEEE' )
        tkinter_text.tag_config( 'border', borderwidth=1 )
        n_cols = len(self.data[0])

        # This loop over rows and items in rows is the costliest part of the
        # code, when we go to really big databrowsers (e.g. 256x256).
        # The Tkinter.Text.insert() function is quite expensive, so you don't
        # want to call it for every single data item.  On the other hand, it
        # doesn't pay to do what it takes to have a single call to insert() --
        # setting up a huge format string -- because string concatenations
        # are also expensive.
        # Here is an example of that over-the-top approach:
        #def plus(self,a,b): return a+b
        #full_format = []
        #for row in self.data:
        #    row_format = []
        #    for item in row:
        #        str_item = (format % item )
        #        width = len(str_item)
        #        row_format.append( format+(self.datum_width-width)*' '+' ')
        #    full_format.append( reduce(self.plus,row_format) + '\n' )
        #tkinter_text.insert( 'end',
        #                    reduce(self.plus, full_format) %
        #                    tuple(reduce(self.plus, self.data)))
        #
        row_list = []
        i=0
        blank_format = '%' + str(self.datum_width) + 's'
        show_irregular = eb_policy[g_eb_policy_tags.show_irregular]
        hilite_irregular = eb_policy[g_eb_policy_tags.hilite_irregular]
        found_NaN = None
        for row in self.data:
            row_copy = row[:]
            j = 0
            row_format_list = []
            for item in row_copy:
                if format_specifies_length == 1:
                    row_format_list.append(format+' ')
                else:
                    str_item = (format % item )
                    width = len(str_item)
                    row_format_list.append(
                        format+(self.datum_width-width)*' '  +' ')
                if self._cellIsHiddenForEB(i,j, eb_policy):
                        row_copy[j] = ' '
                        row_format_list.pop()
                        row_format_list.append(blank_format+' ')

                if( (str(item) == 'nan')
                and (anag_utils.getDebugLevel() > 2)
                and (not found_NaN) ):
                    tkMessageBox.showwarning( message=
                        "Found a NaN.  Look at your xterm to see "
                        "where it is.  There may "
                        "be other NaNs in this box, but we won't warn you about"
                        " all of them.\n  To suppress these warnings altogether"
                        " set your debug level to 2 or less." )
                    found_NaN = 1
                j = j+1

            row_list.append( string.join(row_format_list,'') % tuple(row_copy) )
            row_list.append( '\n' )
            i=i+1
        tkinter_text.insert( 'end', string.join( row_list, '' ) )

        if self.eb_data and (hilite_irregular == 1) and (show_irregular==1):
            i=0
            for row in self.data:
                j=0
                for item in row:
                    if self.eb_data[i][j] == 1:
                        self._highlightCell((j,i,0),format,colortag='peagreen',
                                            eb_policy=eb_policy)
                    j = j+1
                i = i+1

        # Left legend
        j = 0
        max_left_width=0
        self.left_legend.delete(1.0,Tkinter.END) # 1.0 means "beginning"
        for item in self.left_margin:
            self.left_legend.insert( 'end', item + '\n' )
            max_left_width = max( max_left_width, len(item) )
            j = j + 1
        self.left_legend.configure(width=max_left_width)

        # Top legend
        i = 0
        self.top_legend.tag_config( 'peagreen', background=peagreen )
        self.top_legend.tag_config( 'grey', background=grey )
        self.top_legend.delete(1.0,Tkinter.END)
        for item in self.top_margin:
            width = len(item)
            right_padding = (self.datum_width - width)/2
            left_padding = self.datum_width - width - right_padding + 1
            self.top_legend.insert( 'end',
                left_padding*' ' + item + right_padding*' ' )
            i = i + 1

        #timer.stop()


    def _findMaxWidth( self, format ):
        """
        Find now much space the widest data element will require.
        Returns a pair -- the width, and a 0|1 indicating if the format string
        itself specifies the width (e.g. %8.3f does, but %f doesn't).
        """
        anag_utils.funcTrace()

        # If the format already tells us the max width, then use that.
        pctpos = format.find('%')
        m = re.search('\D', format[pctpos+1:])
        stated_width = format[pctpos+1:pctpos+1+m.start()]
        if stated_width != '':
            return int(stated_width), 1
        else:
            result = 0
            i = 0
            for row in self.data:
                j = 0
                for item in row:
                    if type(item) != type(' '):
                        width = len(format % item)
                        result = max( width, result )
                    j = j+1
                i = i+1
            return result,0


    def registerForRestoration(self):
        """
        Overrides version in SelfControl.  We want a no-op here, as
        dialog_inventory is below saved_states in the class "hierarchy".
        """
        anag_utils.funcTrace()


if __name__ == '__main__':
    print "foo bar"
