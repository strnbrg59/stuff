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

# File: control_eb.py
# Author: TDSternberg
# Created: 11/15/01

import anag_utils
import anag_megawidgets
from self_control import SelfControl

import Tkinter
import Pmw

class ControlEb( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """ 
    Embedded boundaries GUI.
    """

    def getShortDescription(self):
        return "Embedded boundaries"
    def getLongDescription(self):
        return "Embedded boundaries stuff"

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            { 'name':'shading_radiobuttons' },
            { 'name':'normal_dir_radiobuttons' },
            { 'name':'visible_checkbutton' },
            { 'name':'capping_checkbutton' },
            { 'name':'clipping_checkbutton' },
            { 'name':'clipping_isovalue' },
            { 'name':'opacity_scale'},
            { 'name':'line_width'},
            { 'name':'ghost'},
            { 'name':'color_wheel' },
            { 'name':'boundary_list'},
            { 'name':'levelset_list'},
            { 'name':'fluid2_list'},
            { 'name':'show_dialog', 'save':6, 'initval':0},
            { 'name':'local_vtk_data'}
          ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'master', 'kw',
            'dep_dict', 
            'button_names',
            'boundary_items', 'i',
            '_colorWheelButtonHandler',
            '_shadingRadioButtonHandler',
            '_normalDirRadioButtonHandler' )

        self.configure( title = 'Embedded boundaries' )
        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self )


    def isOldStyle( self ):
        """
        Returns True iff hdf5 file contains 'fraction-0' component.
        Shouldn't be called until there's an hdf5 file loaded.
        """
        anag_utils.funcTrace()
        return (    ('fraction-0' in self.vtk_data.getComponentList())
               and not self.cmd_line.getIsoEb() )



    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        if not self.isOldStyle():
            self.vtk_eb = self.vtk_iso_eb
        else:
            if self.vtk_data.isResliceMode() == 1:
                return

        self.local_vtk_data = self.vtk_eb.getLocalVtkData()

        #
        # "Visible" checkbutton. (Default is 0 (off).)
        #
        self.visible_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Make visible' )
        self.visible_checkbutton.configure(
            command = lambda butt = self.visible_checkbutton, self=self:
                          self.vtk_eb.setIsVisible( butt.get() ))
        self.visible_checkbutton.pack( anchor='w' )

        #
        # Ghost cells for isocontour/surface.
        #
        if not self.isOldStyle():
            self.ghost = anag_megawidgets.Checkbutton(
                self.interior(),
                text = 'Use ghost cells' )
            self.ghost.configure(
                command = lambda butt = self.ghost, self=self:
                    self._ghostHandler( butt.get() ) )
            self.ghost.pack( anchor='w' )

        #
        # Boundarys.
        #
        if self.isOldStyle():
            self.boundary_list = Pmw.ScrolledListBox( self.interior(),
                labelpos='w', label_text='Boundaries:' )
            boundary_items = []
            for i in range(0, self.local_vtk_data.getNumEBs()):
                boundary_items.append( str(i) )
            self.boundary_list.setlist( boundary_items )
            self.boundary_list.component('listbox').configure(
            height = min( 10, len(boundary_items)), exportselection=0,
            selectmode='multiple' )
            self.boundary_list.configure( selectioncommand = lambda self=self:
                self._boundaryListHandler( self.boundary_list ))
            for i in range(0,self.local_vtk_data.getNumEBs()):
                if self.vtk_eb.getSelectedEbs()[i] == 1:
                    self.boundary_list.select_set( i )
            self._boundaryListHandler( self.boundary_list )
            self.boundary_list.pack()

        #
        # Levelset data (the signed-distance-to-EB function).
        #
        if not self.isOldStyle():
            anag_megawidgets.HorizRule( self.interior(), 298 ).pack()
            self.levelset_list = Pmw.ScrolledListBox(
                self.interior(),
                labelpos='nw',
                label_text='Levelset' )
            component_names = (self.local_vtk_data.getComponentList())
            self.levelset_list.setlist( component_names )
            self.levelset_list.select_set( 0 )
            self.levelset_list.component('listbox').configure(
                height = min(10,len(component_names)), exportselection=0)
            self.levelset_list.configure(
                selectioncommand = lambda self=self:
                    self._levelsetSelectionHandler(self.levelset_list))
            self.levelset_list.pack( padx=22,pady=2, anchor=Tkinter.W )


        #
        # "Clip" checkbutton. (Default is 0 (off).)
        #
        if self.isOldStyle(): # FIXME: should be available with new style too.
            self.clipping_checkbutton = anag_megawidgets.Checkbutton(
                self.interior(),
                text = 'Clip' )
            self.clipping_checkbutton.configure(
                command = lambda butt = self.clipping_checkbutton, self=self:
                              self.vtk_eb.setIsClipped( butt.get() ))
            if not self.local_vtk_data.is2DMode():
                self.clipping_checkbutton.pack( padx=22, anchor=Tkinter.W )
    
    
        #
        # "Capping" checkbutton. (Default is 0 (off).)
        #
        if self.isOldStyle():
            self.capping_checkbutton = anag_megawidgets.Checkbutton(
                self.interior(),
                text = 'Cap' )
            self.capping_checkbutton.configure(
                command = lambda butt = self.capping_checkbutton, self=self:
                              self.vtk_eb.setCapping( butt.get() ))
            self.capping_checkbutton.pack( padx=22, anchor=Tkinter.W )


        #
        # "Blank covered cells" checkbutton. (Default is 0 (off).)
        #
        self.blank_covered_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Blank covered cells' )
        self.blank_covered_checkbutton.configure(
            command = self._blankCoveredCellsHandler )
        self.blank_covered_checkbutton.pack( padx=22, anchor=Tkinter.W )


        #
        # 
        #
        if not self.isOldStyle():
            self.clipping_isovalue = anag_megawidgets.EntryScale(
                self.interior(),
                button_text = 'Clipping isovalue',
                dep_dict = {'saved_states':self.saved_states },
                no_validation=1,
                from_=-1.0, to=10.0, scale_normal_resolution=0.01,
                scale_callback = self._clippingIsovalueHandler )
            self.clipping_isovalue.pack()


        #
        # Levelset data (the signed-distance-to-EB function).
        #
        if not self.isOldStyle():
            component_names = self.local_vtk_data.getComponentList()
            self.fluid2_list = anag_megawidgets.ComponentSelectionLists(
                master_frame=self.interior(),
                orientation='vertical',
                main_label_text='',
                num_lists = 1,
                button_text='Show fluid 2',
                list_items = component_names,
                max_visible_list_items = min(5, len(component_names)),
                callback = self._fluid2SelectionHandler )
            self.fluid2_list.disable()
            self.fluid2_list.pack( padx=22,pady=2, anchor=Tkinter.W )

            anag_megawidgets.HorizRule( self.interior(), 298 ).pack()


        #
        # Normal direction. (Inward vs outward.  Default is 0 (Inward).)
        #
        if self.isOldStyle():
            self.normal_dir_radiobuttons = anag_megawidgets.RadioSelect(
                self.interior(),
                buttontype = 'radiobutton',
                orient = 'vertical',
                labelpos = 'nw',
                hull_borderwidth = 2, hull_relief = 'groove'
                )
            if self.isOldStyle():
                self.normal_dir_radiobuttons.configure(
                    label_text = 'Normal direction:' )
    
            map( self.normal_dir_radiobuttons.add, ('Inward','Outward'))
            button_names = self.normal_dir_radiobuttons.butt_names
            def _normalDirRadioButtonHandler( button_tag, self,
                                              button_names=button_names ):
                if button_tag == button_names.Inward:
                    self.vtk_eb.setNormalOutward( 0 )
                else:
                    self.vtk_eb.setNormalOutward( 1 )
            self.normal_dir_radiobuttons.configure(
                command = lambda button_tag,
                    handler=_normalDirRadioButtonHandler,
                    self=self: handler( button_tag, self ))
            self.normal_dir_radiobuttons.pack( padx=22, anchor=Tkinter.W )
    
        #
        # Shading (Default is 0 (Constant).)
        #
        self.shading_radiobuttons = anag_megawidgets.RadioSelect(
            self.interior(),
            buttontype = 'radiobutton',
            orient = 'vertical',
            labelpos = 'nw',
            label_text = 'Shading:',
            hull_borderwidth = 2, hull_relief = 'groove'
            )
        map( self.shading_radiobuttons.add, ('By value','Constant'))
        button_names = self.shading_radiobuttons.butt_names
        def _shadingRadioButtonHandler( button_tag, self,
                                        button_names=button_names ):
            if button_tag == button_names.By_value:
                self.vtk_eb.changeColor( 0 )
            else:
                self.vtk_eb.changeColor( 1 )
        self.shading_radiobuttons.configure(
            command = lambda button_tag, handler=_shadingRadioButtonHandler,
                             self=self: handler( button_tag, self ))
        self.shading_radiobuttons.pack( padx=22, anchor=Tkinter.W )

        #
        # Opacity (0 to 1 inclusive)
        #
        self.opacity_scale = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Opacity',
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1,
            from_=0.0, to=1.0, scale_normal_resolution=0.1,
            scale_callback = lambda x, self=self:
                self.vtk_eb.setOpacity( x )
            )
        if not self.local_vtk_data.is2DMode():
            self.opacity_scale.pack(anchor='e')


        #
        # Line width scale.
        #
        if self.local_vtk_data.is2DMode():
            self.line_width = anag_megawidgets.EntryScale(
                self.interior(),
                button_text = 'Line width:',
                no_validation=1,
                discrete=1,
                dep_dict = {'saved_states':self.saved_states},
                scale_callback = lambda n, self=self: self.vtk_eb.setLineWidth(
                    int(n) ))
            self.line_width.configure(
                from_=1, to=10,
                resolution = 1 )
            self.line_width.pack( anchor='e' )


        #
        # Colorwheel
        # 
        self.color_wheel = anag_megawidgets.ColorWheel(
            master = self.interior(),
            colorwheel_image = self.local_vtk_data.getChomboVisDataDir() +\
                '/ColorWheel.ppm',
            label_text = 'Constant color',
            show_rgb_entries = 1,
            command = lambda rgb, self=self,f=self._colorWheelButtonHandler:
                          f( rgb ),
            relief = Tkinter.GROOVE, borderwidth = 2
            )
        self.color_wheel.pack( padx=2, pady=2, anchor=Tkinter.N )


    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'self', 'decls', 'cur_selection' )
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        if not self.isOldStyle():
            self.vtk_eb = self.vtk_iso_eb
        else:
            if self.vtk_data.isResliceMode() == 1:
                return

        #
        # The dialog itself -- visible or hidden.
        #
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        self.visible_checkbutton.set( self.vtk_eb.getIsVisible() )
        if self.isOldStyle():
            self.clipping_checkbutton.set( self.vtk_eb.getIsClipped() )

        if self.isOldStyle():
            self.capping_checkbutton.set( self.vtk_eb.getCapping() )
        else:
            self.clipping_isovalue.setScaleSterile(
                self.vtk_data.getIsocontourClipValue() )
            if self.vtk_eb.getUseGhostCells() != self.ghost.get():
                self.ghost.toggle()


        self.blank_covered_checkbutton.set( self.vtk_eb.getBlankCoveredCells() )

        if self.isOldStyle():
            self.normal_dir_radiobuttons.invoke(
                self.vtk_eb.getNormalOutward() );

        if self.vtk_eb.getUseOneColor():
            self.shading_radiobuttons.invoke(
                self.shading_radiobuttons.butt_names.Constant )
        else:
            self.shading_radiobuttons.invoke(
                self.shading_radiobuttons.butt_names.By_value )
    
        self.opacity_scale.setScaleSterile( self.vtk_eb.getOpacity() )
        if self.local_vtk_data.is2DMode():
            self.line_width.setScaleSterile( self.vtk_eb.getLineWidth() )
        self.color_wheel.setRgbEntries( self.vtk_eb.getTheOneColor())

        if self.isOldStyle():
            cur_selection = self.boundary_list.component('listbox'
                                                        ).curselection()

            for i in range(0,self.local_vtk_data.getNumEBs()):
                if self.vtk_eb.getSelectedEbs()[i] == 1:
                    self.boundary_list.select_set( i )
                else:
                    self.boundary_list.component('listbox').select_clear( i )
        else:
            self.fluid2_list.configure( button_callback=
                self._fluid2ButtonHandler )
            if self.vtk_eb.getFluid2():
                self.fluid2_list.setSelections( (self.vtk_eb.getFluid2(),) )
                self.fluid2_list.setButton( 1 )
            self.levelset_list.component('listbox').select_clear( 0 )
            curcomp = self.vtk_eb.getLevelsetComponent()
            if type(curcomp) == type("foo"):
                self.levelset_list.select_set(
                    self.local_vtk_data.getComponentSerialNum(curcomp) )
                    

    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.dialog_inventory.unregisterDialog( self )
        self.withdraw()


    def _blankCoveredCellsHandler( self ):
        butt = self.blank_covered_checkbutton
        self.vtk_eb.setBlankCoveredCells( butt.get() )
        if not self.isOldStyle():
            if butt.get() == 0:
                self.fluid2_list.disable()
            else:
                self.fluid2_list.enable()


    def _fluid2SelectionHandler( self, component_selection, ndx ):
        """
        For selection of field component to act as second fluid, which is
        where the first has covered cells.
        """
        anag_utils.funcTrace()
        self.vtk_eb.setFluid2( component_selection )
        self.vtk_eb.updateVis()


    def _fluid2ButtonHandler( self, on_off ):
        """
        Handles on/off of fluid2 checkbutton.  If on, and a component is
        selected, then renders fluid2 over fluid1's covered cells.  If off,
        disables fluid2-rendering.
        """
        anag_utils.funcTrace()
        if on_off == 0:
            self.vtk_eb.setFluid2(None)
            self.vtk_eb.updateVis()
        else:
            selections = self.fluid2_list.getSelections()[0]
            if len(selections) > 0:
                self.vtk_eb.setFluid2( selections[0] )
                self.vtk_eb.updateVis()


    def _ghostHandler( self, yes_no ):
        """ Toggle ghost cells, when in eb-as-iso mode. """
        anag_utils.funcTrace()
        self.vtk_eb.setUseGhostCells( yes_no )


    def _clippingIsovalueHandler( self, x ):
        anag_utils.funcTrace()
        self.vtk_data.setIsocontourClipValue(float(x))
        self.vtk_eb.updateVis()


    def _levelsetSelectionHandler( self, component_selection ):
        """ Handler for selection events in the component-choice menu """
        anag_utils.funcTrace()
        choice = component_selection.getcurselection()[0]
        self.vtk_eb.setLevelsetComponent( choice )
        self.vtk_eb.updateVis()


    def _colorWheelButtonHandler( self, rgb ):
        """ Handler for <Button-1> on the ColorWheel.
            Arg rgb is the (r,g,b) coordinates of the point clicked on the
                colorwheel.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self', 'rgb' )

        if( self.shading_radiobuttons.getcurselection() ==
            self.shading_radiobuttons.butt_names.By_value ):
            self.vtk_eb.changeColor( 0, rgb )
        else:
            self.vtk_eb.changeColor( 1, rgb )            
        decls.memberFunctionAudit( self )


    def _boundaryListHandler( self, scrolled_list_box ):
        """ Handler the list of EB boundarys. """
        anag_utils.funcTrace()

        curselections = scrolled_list_box.getcurselection()
        
        for boundary in range(0, self.local_vtk_data.getReader().GetNumEBs()):
            if str(boundary) in curselections:
                self.vtk_eb.selectBoundary(boundary, on_off=1 )
            else:
                self.vtk_eb.selectBoundary(boundary, on_off=0 )
