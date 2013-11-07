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

# File: control_slice.py
# Author: TDSternberg
# Created: 6/04/01

""" Creates the control options area for the slice viewer. """

import Tkinter
import Pmw
import math

import vtk_slice
import anag_utils
import anag_megawidgets
import algorithms
from self_control import SelfControl


class ControlSlice( anag_megawidgets.SelfDescribingDialog, SelfControl ):

    def getShortDescription(self):
        if self.local_vtk_data.is2DMode(): return "Slice"
        else:                         return "Slices"
    def getLongDescription(self):
        if self.local_vtk_data.is2DMode(): return "Slicing plane"
        else:                         return "Slicing planes"

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )

        SelfControl.__init__( self, dep_dict,
          [ { 'name':'offset'},  # EntryScale
            { 'name':'grid_plane_offset'}, # EntryScale
            { 'name':'slicing_planes', 'initval':{}},
            { 'name':'clipping_checkbutton' },
            { 'name':'show_dialog', 'save':6, 'initval':0},
            { 'name':'local_vtk_data' },  # from vtk_slice
            { 'name':'use_ghost_cells_checkbutton' }
          ] )
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'master', 'kw',
            'dep_dict',  'axis', '_offsetHandler', 'uniqizer' )
        self.configure( title = 'Slices' )

        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        # Lay out the slicing planes.
        # We'll create all three, regardless of the dimensionality of the
        # data.  This is so we don't need to call vtk_data.is2DMode() here --
        # before we've even loaded an hdf5, possibly.  There's no harm in
        # having a useless slice constructed, as long as we don't try to
        # use it.  In _refresh(), we make sure not to pack the x and y
        # planes if is2DMode().
        uniqizer = 0
        self.local_vtk_data = self.vtk_slice.getLocalVtkData()
        if not self.local_vtk_data.is2DMode():
            for axis in 'x', 'y':
                self.slicing_planes[axis+str(uniqizer)] = _SlicingPlane(
                               self.interior(),
                               axis,
                               uniqizer,
                               dep_dict = {
                                 'local_vtk_data':self.local_vtk_data,
                                 'vtk_slice':self.vtk_slice,
                                 'saved_states':self.saved_states },
                                 relief=Tkinter.GROOVE, borderwidth=2 )
        axis='z'
        self.slicing_planes[axis+str(uniqizer)] = _SlicingPlane(
                           self.interior(),
                           axis,
                           uniqizer,
                           dep_dict = {
                             'local_vtk_data':self.local_vtk_data,
                             'vtk_slice':self.vtk_slice,
                             'saved_states':self.saved_states },
                              relief=Tkinter.GROOVE, borderwidth=2 )

        if not self.local_vtk_data.is2DMode():
            self.slicing_planes['x0'].pack()
            self.slicing_planes['y0'].pack()
        self.slicing_planes['z0'].pack()


        #
        # "Clipping" checkbutton. (Default is 0 (off).)
        #
        self.clipping_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Clip' )
        self.clipping_checkbutton.configure(
            command = lambda butt = self.clipping_checkbutton, self=self:
                          self.vtk_slice.setClipMode(
                            butt.get() * vtk_slice.g_clip_modes.plane ))
            # FIXME: ugly hack, replace with two checkbuttons, one for plane
            # clipping and one for EB clipping.

        self.clipping_checkbutton.pack( anchor=Tkinter.W )
    

        #
        # "Use ghost cells" checkbutton. (Default is 1 (on).)
        #
        self.use_ghost_cells_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Use ghost cells' )
        self.use_ghost_cells_checkbutton.configure(
            command = lambda butt = self.use_ghost_cells_checkbutton, self=self:
                          self.vtk_slice.setUseGhostCells( butt.get() ))
        self.use_ghost_cells_checkbutton.pack( anchor='w' )


        # "Offset" sliders: one for distance between planes, the other for
        # distance between pairs of grid planes.
        if not self.local_vtk_data.is2DMode():
            def _offsetHandler( self, x ):
                self.vtk_slice.setOffset( x ) # OK, no int'd step.
            self.offset = anag_megawidgets.EntryScale(
                self.interior(),
                button_text = 'Offset between levels:',
                length = 100,
                scale_normal_resolution = 0.01,
                scale_callback = lambda x, self=self, f = _offsetHandler:
                                     f(self,x),
                dep_dict = {'saved_states':self.saved_states }
                )
            self.offset.configure( from_=-1.0, to=1.0 )
            self.offset.pack()

            def _gridPlaneOffsetHandler( self, x ):
                self.vtk_slice.setGridPlaneOffset( x )
            self.grid_plane_offset = anag_megawidgets.EntryScale(
                self.interior(),
                button_text = 'Slice cells offset:',
                length = 100,
                scale_normal_resolution = 0.01,
                scale_callback = lambda x, self=self,
                                    f = _gridPlaneOffsetHandler: f(self,x),
                dep_dict = {'saved_states':self.saved_states }
                )
            self.grid_plane_offset.configure( from_=0.0, to=1.0 )
            self.grid_plane_offset.setScaleValue(
                self.vtk_slice.getGridPlaneOffset() )
            self.grid_plane_offset.pack()


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()

        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        use_ghost_cells = self.vtk_slice.getUseGhostCells()
        if( use_ghost_cells !=  self.use_ghost_cells_checkbutton.get() ):
            self.use_ghost_cells_checkbutton.set( use_ghost_cells )

        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )

        # Call refresh() on each slicing plane.  This is superfluous during
        # a restore, but quite necessary if we want chombovis.vtkUpdate() to
        # work (because chombovis.vtkUpdate() just goes through the list of
        # classes it knows about).
        for k in self.slicing_planes.keys():
            self.slicing_planes[k]._refresh()

        if not self.local_vtk_data.is2DMode():
            self.offset.setScaleSterile(
                self.vtk_slice.getOffset() )

        # Disable ghost cells and (plane-) clip buttons if we're in eb-clip
        # mode.  Eb-clip mode needs ghost cells to be off.
        self.registerCallback(
            self.vtk_slice.getNotifierClipMode(),
            self._ebClipModeButtonDisabler )


    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()


    def cleanup( self ):
        anag_utils.funcTrace()
        for k in self.slicing_planes.keys():
            self.slicing_planes[k].cleanup()
        self.dialog_inventory.unregisterDialog( self )
        self.withdraw()


    def _ebClipModeButtonDisabler( self, clip_mode, dummy ):
        """
        Disable the 'ghost cells' and 'clip' buttons, when we're in eb-clip
        mode.
        """
        anag_utils.funcTrace()
        if clip_mode == vtk_slice.g_clip_modes.eb:
            self.use_ghost_cells_checkbutton.configure( state='disabled' )
            self.clipping_checkbutton.configure( state='disabled' )
        else:
            self.use_ghost_cells_checkbutton.configure( state='normal' )
            self.clipping_checkbutton.configure( state='normal' )

    
    def _paddedBoxesCallback( self, on_off, dummy ):
        anag_utils.funcTrace()
        use_ghost_cells = self.vtk_slice.getUseGhostCells()
        if( use_ghost_cells !=  self.use_ghost_cells_checkbutton.get() ):
            self.use_ghost_cells_checkbutton.set( use_ghost_cells )


class _SlicingPlane( Tkinter.Frame, SelfControl ):
    """ 
    Defines class for a slice (or slices -- see VtkSlice.newPlane())
    corresponding to a fixed level on one axis -- X, Y or Z.
 
    The Tcl version had three files, controlSliceX.tcl, controlSliceY.tcl
    and controlSliceZ.tcl.  In Python, I've folded them into this one class,
    of which three instances are created -- one for 'x', 'y', and 'z'.
    """

    def __init__(self, master, axis, uniqizer, dep_dict, **kw ):
        """ arg axis must be 'x', 'y' or 'z'. """
        anag_utils.funcTrace()

        Tkinter.Frame.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            {'name':'axis'},
            {'name':'slewscale'},
            {'name':'show_slice_button'},
            {'name':'launch2D_button'},
            {'name':'multi_instance_qualifier'}, # To make unique state keys.
            {'name':'initforfirsthdf5_called'}
          ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls',
            'self', 'master', 'axis', 'dep_dict', 'kw', '_slewscaleHandler',
            'uniqizer'
        )
        
        assert( axis=='x' or axis=='y' or axis=='z' )
        self.axis = axis
        self.multi_instance_qualifier = axis + str(uniqizer)

        #
        # Entry and Scale
        #
        self.slewscale = anag_megawidgets.EntryScale(
            self,
            button_text = axis.upper(),
            dep_dict = {'saved_states':self.saved_states },
            length = 100 )

        # "Visible" checkbutton.
        self.show_slice_button = anag_megawidgets.Checkbutton(
            self, text = 'visible' )

        # "Launch 2D" button.
        self.launch2D_button = Tkinter.Button(
            self, text="Launch 2D",
            command = lambda self=self:
            self.vtk_slice.getSlice(self.axis+'0').launch2DViewer() )


        decls.memberFunctionAudit( self )


    def movePlane( self, x ):
        """
        Move all the planes (usually just one) that are normal to the self.axis
        direction.
        """
        anag_utils.funcTrace()
        for k in self.vtk_slice.getSlicingPlanes().keys():
            if k[0] == self.axis:
                 self.vtk_slice.getSlicingPlanes()[k].movePlane( x )

    def toggleSliceVisibility( self, on_off ):
        """
        On/off for all the planes that are normal to the self.axis direction.
        """
        anag_utils.funcTrace()
        assert( on_off==0  or  on_off==1 )
        for k in self.vtk_slice.getSlicingPlanes().keys():
            if k[0] == self.axis:
                 self.vtk_slice.getSlicingPlanes()[k].toggleSliceVisibility(
                    on_off)


    def _configureSlewscaleRange( self ):
        anag_utils.funcTrace()
        if not self.local_vtk_data.is2DMode():
            max_level = self.local_vtk_data.getMaxAvailableLevel()
            smallest_dx = min(self.local_vtk_data.getLevelDx( max_level ))
            dx0 = max(self.local_vtk_data.getLevelDx(0))
            axis_num = {'x':0, 'y':1, 'z':2}[self.axis]
            centering = self.local_vtk_data.getDataCentering()
            axis_min = self.local_vtk_data.getDomainMin( [self.axis] ) \
                     + 0.5*dx0*centering[axis_num]
            axis_max = self.local_vtk_data.getDomainMax( [self.axis] ) \
                     - 0.5*dx0*centering[axis_num]
            resolution = algorithms.roundDown10( smallest_dx/5.0 )
            self.slewscale.configure(
                scale_normal_resolution = resolution,
                from_ = algorithms.roundUp( axis_min,
                                            -math.log10(resolution) ),
                to    = algorithms.roundDown( axis_max,
                                            -math.log10(resolution) ))

    def _anisotropicFactorsCallback( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._configureSlewscaleRange()


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        self.slewscale.configure( scale_callback = self.movePlane )
        self._configureSlewscaleRange()
        if not self.local_vtk_data.is2DMode():
            self.slewscale.pack()

        self.show_slice_button.configure(
            command = lambda self=self, butt=self.show_slice_button:
                self.toggleSliceVisibility(butt.get()))
        self.show_slice_button.pack()

        if not self.local_vtk_data.is2DMode():
            self.launch2D_button.pack( padx=22, side=Tkinter.TOP,
                                       expand=1, fill=Tkinter.X )

        self.initforfirsthdf5_called = 1


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        if self.initforfirsthdf5_called == None:
            return

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        if not self.local_vtk_data.is2DMode():
            self.slewscale.setScaleSterile(
                self.vtk_slice.getSlicingPlanes()[self.axis+'0'
                                                 ].getPlanePosition() )

        if( self.vtk_slice.getSlicingPlanes()[self.axis+'0'
                 ].getSliceIsVisible() != self.show_slice_button.get() ):
            self.show_slice_button.toggle()

        self.registerCallback(
            self.local_vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicFactorsCallback )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()
