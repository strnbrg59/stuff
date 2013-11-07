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

# File: control_grid.py
# Author: TDSternberg
# Created: 6/05/01

""" GUI controls for box and cell outlines """

import Tkinter
import Pmw

import anag_utils
import anag_megawidgets
from self_control import SelfControl
import vtk_grid # For g_button_tags

g_decls = anag_utils.Declarations( 'g_decls', 'g_shading_options' )

class ControlGrid( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """ Lay out and initialize the widgets. """

    def getShortDescription(self):
        return "Grid detail"
    def getLongDescription(self):
        return "Boxes and cells at various detail levels"
    
    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            { 'name':'cell_detail_radiobuttons'},
            { 'name':'show_domainbox_button'},
            { 'name':'show_tickmarks_button'},
            { 'name':'show_slicecells_button'},
            { 'name':'show_solidboxes_button'},
            { 'name':'clipping_checkbutton' },
            { 'name':'outline_shading_button' },
            { 'name':'solid_shading_button' },
            { 'name':'color_wheel'},
            { 'name':'line_width'},
            { 'name':'shrinkage_scale'}, # solid box shrinkage factor
            { 'name':'opacity'}, # of solid boxes
            { 'name':'show_dialog', 'save':2, 'initval':0},
            { 'name':'use_ghost_cells_checkbutton' },
            { 'name':'local_vtk_data' } # from vtk_grid
          ])
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls", "self", "master", "kw",
            "dep_dict" )
        self.configure( title="Grid detail" )
        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self)

    
    def _initForFirstHDF5( self ):
    
        self.local_vtk_data = self.vtk_grid.getLocalVtkData()

        # A Frame to hold the radiobuttons for "Nothing" etc, as well as the
        # Checkbutton for "Domain box".
        show_frame = Tkinter.Frame( self.interior(), borderwidth=2, 
                                    relief='groove' )
        show_frame.pack()

        #
        # Radiobuttons 'Nothing', 'Bounding boxes',
        # 'Slice cells', 'Face cells', 'All cells'
        #
        self.cell_detail_radiobuttons = anag_megawidgets.RadioSelect(
            show_frame,
            buttontype = 'radiobutton',
            orient = 'vertical',
            labelpos = 'nw',
            label_text = "Show:"
            )
        self.cell_detail_radiobuttons.configure(
            command = lambda button_tag, self=self:
                   self.vtk_grid.chooseCellDetail( button_tag ))
    
        butt_tags = vtk_grid.g_button_tags
        if not self.local_vtk_data.is2DMode():
            map( self.cell_detail_radiobuttons.add,
                 (butt_tags.Nothing, butt_tags.Bounding_boxes,
                  butt_tags.Face_cells, butt_tags.All_cells))
        else:
            map( self.cell_detail_radiobuttons.add,
                 (butt_tags.Nothing, butt_tags.Bounding_boxes,
                  butt_tags.All_cells))
    
        self.cell_detail_radiobuttons.pack()

        #
        # "Clipping" checkbutton. (Default is 0 (off).)
        #
        self.clipping_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Clip' )
        self.clipping_checkbutton.configure(
            command = lambda butt = self.clipping_checkbutton, self=self:
                          self.vtk_grid.setIsClipped( butt.get() ),
            state = 'normal' )
        if not self.local_vtk_data.is2DMode():
            self.clipping_checkbutton.pack( anchor='w' )


        #
        # "Use ghost cells" checkbutton.
        #
        self.use_ghost_cells_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Use ghost cells' )
        self.use_ghost_cells_checkbutton.configure(
            command = lambda butt = self.use_ghost_cells_checkbutton, self=self:
                          self.vtk_grid.setUseGhostCells( butt.get() ))
        self.use_ghost_cells_checkbutton.pack( anchor='w' )


        #
        # Line width
        #
        self.line_width = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Line width:',
            no_validation=1,
            discrete=1,
            resolution=1,
            from_=1, to=10,
            dep_dict = {'saved_states':self.saved_states},
            scale_callback = lambda n, self=self: self.vtk_grid.setLineWidth(
                int(n) ))
        self.line_width.pack( anchor=Tkinter.W )


        #
        # Solid box shrinkage factor
        #
        self.shrinkage_scale = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Box shrinkage:',
            no_validation=1,
            scale_normal_resolution = 0.01,
            from_=-10.0, to=0.0,
            scale_callback = lambda x, self=self:
                self.vtk_grid.setSolidBoxShrinkageFactor(x))
        if not self.local_vtk_data.is2DMode():
            self.shrinkage_scale.pack( anchor=Tkinter.W )


        #
        # Solid box opacity
        #
        self.opacity = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Box opacity:',
            no_validation=1,
            scale_normal_resolution = 0.01,
            from_=0.0, to=1.0,
            scale_callback = lambda x, self=self:
                self.vtk_grid.setOpacity(x))
        if not self.local_vtk_data.is2DMode():
            self.opacity.pack( anchor=Tkinter.W )


        # A horizontal rule to separate from the "domain box" checkbutton:
        anag_megawidgets.HorizRule( show_frame, width=115 ).pack()
    
        #
        # "Domain box" checkbutton.
        #
        self.show_domainbox_button = anag_megawidgets.Checkbutton(
            show_frame,
            text = "Domain box"
            )
        self.show_domainbox_button.configure(
            command = lambda self=self, butt = self.show_domainbox_button:
                          self.vtk_grid.showDomainBox( butt.get() ))
        self.show_domainbox_button.pack(anchor='w')


        #
        # "Tick marks" checkbutton.
        #
        self.show_tickmarks_button = anag_megawidgets.Checkbutton(
            show_frame,
            text = "Tick marks"
            )
        self.show_tickmarks_button.configure(
            command = lambda self=self, butt = self.show_tickmarks_button:
                          self.vtk_grid.showTickMarks( butt.get() ))
        self.show_tickmarks_button.pack(anchor='w')


        #
        # "Slice cells" checkbutton.
        #
        self.show_slicecells_button = anag_megawidgets.Checkbutton(
            show_frame,
            text = "Slice cells"
            )
        self.show_slicecells_button.configure(
            command = lambda self=self, butt = self.show_slicecells_button:
                          self.vtk_grid.showSliceCells( butt.get() ),
            state = 'disabled' )
        if not self.local_vtk_data.is2DMode():
            self.show_slicecells_button.pack( anchor='w')


        #
        # "Solid boxes" checkbutton.
        #
        self.show_solidboxes_button = anag_megawidgets.Checkbutton(
            show_frame,
            text = "Solid boxes"
            )
        self.show_solidboxes_button.configure(
            command = lambda self=self, butt = self.show_solidboxes_button:
                          self.vtk_grid.setShowSolidBoxes( int(butt.get()) ))
        if not self.local_vtk_data.is2DMode():
            self.show_solidboxes_button.pack( anchor='w')


        color_frame = Tkinter.Frame( self.interior() )
        color_frame.pack(anchor='w')

        #
        # Shading checkbuttons.
        #
        self.outline_shading_button = anag_megawidgets.Checkbutton(
            color_frame,
            text = 'Shade box outlines by level'  )
        self.outline_shading_button.configure(
            command = lambda self=self, butt= self.outline_shading_button:
                self.vtk_grid.setColorOutlinesByLevel(int(butt.get())))
        self.outline_shading_button.pack(anchor='w')

        self.solid_shading_button = anag_megawidgets.Checkbutton(
            color_frame,
            text = 'Shade solid boxes by level'  )
        self.solid_shading_button.configure(
            command = lambda self=self, butt= self.solid_shading_button:
                self.vtk_grid.setColorSolidsByLevel(int(butt.get())))
        if not self.local_vtk_data.is2DMode():
            self.solid_shading_button.pack(anchor='w')


        #
        # Colorwheel
        #
        self.color_wheel = anag_megawidgets.ColorWheel(
            master = color_frame,
            colorwheel_image = self.local_vtk_data.getChomboVisDataDir() +\
                "/ColorWheel.ppm",
            label_text = "Constant color",
            show_rgb_entries = 1,
            #rgb_initial_vals = {'r':1, 'g':1, 'b':1},
            command = self._colorWheelHandler,
            relief = Tkinter.GROOVE, borderwidth = 2
            )
        self.color_wheel.pack( padx=2, pady=2, anchor=Tkinter.N )

    def _colorWheelHandler( self, rgb ):
        """
        Set the constant color of the solid boxes only if solid box color-by-
        level is toggled off.  Set the color of everything else only if outline
        box color-by-level is toggled off.
        """
        anag_utils.funcTrace()
        if self.vtk_grid.getColorOutlinesByLevel() == 0:
            self.vtk_grid.setOutlineColor(rgb=rgb)
        if self.vtk_grid.getColorSolidsByLevel() == 0:
            self.vtk_grid.setSolidColor(rgb=rgb)


    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        use_ghost_cells = self.vtk_grid.getUseGhostCells()
        if( use_ghost_cells !=  self.use_ghost_cells_checkbutton.get() ):
            self.use_ghost_cells_checkbutton.set( use_ghost_cells )
        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        self.cell_detail_radiobuttons.setcurselection(
            self.vtk_grid.getCellDetailButtonTag())
        if( self.show_domainbox_button.get()
        !=  self.vtk_grid.getShowDomainBox() ):
            self.show_domainbox_button.toggle()
        if( self.show_tickmarks_button.get()
        !=  self.vtk_grid.getShowTickMarks() ):
            self.show_tickmarks_button.toggle()
        if( self.show_slicecells_button.get()
        !=  self.vtk_grid.getShowSliceCells() ):
            self.show_slicecells_button.toggle()
        if( self.show_solidboxes_button.get()
        !=  self.vtk_grid.getShowSolidBoxes() ):
            self.show_solidboxes_button.toggle()
        self.color_wheel.setRgbEntries( self.vtk_grid.getOutlineColor())
        self.clipping_checkbutton.set( self.vtk_grid.getIsClipped() )
        self.line_width.setScaleSterile( self.vtk_data.getGridLineWidth())
        self.shrinkage_scale.setScaleSterile(
            self.vtk_grid.getSolidBoxShrinkageFactor())
        self.opacity.setScaleSterile( self.vtk_grid.getOpacity() )
        self.outline_shading_button.set(
            self.vtk_grid.getColorOutlinesByLevel() )
        self.solid_shading_button.set(
            self.vtk_grid.getColorSolidsByLevel() )
        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        self.show_slicecells_button.configure( state = 'normal' )


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


    def _paddedBoxesCallback( self, on_off, dummy ):
        anag_utils.funcTrace()
        use_ghost_cells = self.vtk_grid.getUseGhostCells()
        if( use_ghost_cells !=  self.use_ghost_cells_checkbutton.get() ):
            self.use_ghost_cells_checkbutton.set( use_ghost_cells )


class _ShadingOptions:
    constant = 'Constant'
    by_level = 'By level'
    names = (constant, by_level)
g_shading_options = _ShadingOptions()
