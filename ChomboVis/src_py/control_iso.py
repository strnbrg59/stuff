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

# File: control_iso.py
# Author: TDSternberg
# Created: 6/06/01

import algorithms
import anag_utils
import anag_megawidgets
import vtk_iso  # for g_constant_color, g_self_color
from self_control import SelfControl

import Tkinter
import Pmw
import math


class ControlIso( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """ 
    Isosurface GUI.
    """

    def getShortDescription(self):
        if self.vtk_data.is2DMode() == 1: return 'Isocontours'
        else:                         return 'Isosurfaces'
    def getLongDescription(self):
        if self.vtk_data.is2DMode() == 1: return 'Isocontours'
        else:                         return 'Isosurfaces'


    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            { 'name':'minmax' },
            { 'name':'visible_checkbutton' },
            { 'name':'clipping_checkbutton' },
            { 'name':'use_ghost_cells_checkbutton' },
            { 'name':'color_wheel' },
            { 'name':'show_dialog', 'save':2, 'initval':0},
            { 'name':'component_selection'}, # ScrolledListBox, for cross-color
            { 'name':'line_width'},
            { 'name':'opacity_scale'},
            { 'name':'local_vtk_data' }  # from vtk_iso
          ])
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls", "master", "kw",
            "dep_dict" )
        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls',
            'button_names', 'component_names',
            '_colorWheelButtonHandler' )

        self.local_vtk_data = self.vtk_iso.getLocalVtkData()

        if self.local_vtk_data.is2DMode()==1:
            self.configure( title = 'Isocontours' )
        else:
            self.configure( title = 'Isosurfaces' )

        #
        # 'Visible' checkbutton. (Default is 0 (off).)
        #
        self.visible_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Make visible' )
        self.visible_checkbutton.configure(
            command = lambda butt = self.visible_checkbutton, self=self:
                          self.vtk_iso.setDoShowIsosurfaces( butt.get() ))
        self.visible_checkbutton.pack( anchor='w' )
                    

        #
        # "Clipping" checkbutton. (Default is 0 (off).)
        #
        self.clipping_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Clip' )
        self.clipping_checkbutton.configure(
            command = lambda butt = self.clipping_checkbutton, self=self:
                          self.vtk_iso.setIsClipped( butt.get() ))
        self.clipping_checkbutton.pack( anchor='w' )


        #
        # "Use ghost cells" checkbutton. (Default is 1 (on).)
        #
        self.use_ghost_cells_checkbutton = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'Use ghost cells' )
        self.use_ghost_cells_checkbutton.configure(
            command = lambda butt = self.use_ghost_cells_checkbutton, self=self:
                          self.vtk_iso.setUseGhostCells( butt.get() ))
        self.use_ghost_cells_checkbutton.pack( anchor='w' )
    
        #
        # 'Value(s)' (min/max/num)
        #
        self.minmax = _MinMaxNum( master=self.interior(),
            dep_dict={'vtk_iso':self.vtk_iso,
                      'local_vtk_data':self.local_vtk_data, 
                      'saved_states':self.saved_states},
                        relief='groove', borderwidth=1 )
        self.minmax.pack( expand=1, fill=Tkinter.X )
        # More minmax configuration in _refresh().

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
                self.vtk_iso.setOpacity( x )
            )
        if not self.local_vtk_data.is2DMode():
            self.opacity_scale.pack(anchor='e')

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
            scale_callback = lambda n, self=self: self.vtk_iso.setLineWidth(
                int(n) ))
        if self.local_vtk_data.is2DMode():
            self.line_width.pack( anchor=Tkinter.W )
        
    
        #
        # Shading
        #
        self.component_selection = Pmw.ScrolledListBox(
            self.interior(), labelpos='nw', label_text='Shading:' )
        component_names = (self.local_vtk_data.getComponentList())[:]
        self.component_selection.setlist(
            [vtk_iso.g_constant_color, vtk_iso.g_self_color] +
            component_names )
        self.component_selection.component('listbox').configure(
            height = min(10, 2+len(component_names)), exportselection=0 )
        self.component_selection.configure(
            selectioncommand = lambda self=self:
                self._componentSelectionHandler(self.component_selection))
        self.component_selection.pack( padx=22,pady=2, anchor=Tkinter.W )

        #
        # Colorwheel
        # 
        self.color_wheel = anag_megawidgets.ColorWheel(
            master = self.interior(),
            colorwheel_image = self.local_vtk_data.getChomboVisDataDir() +\
                '/ColorWheel.ppm',
            label_text = 'Constant color',
            show_rgb_entries = 1,
            command = lambda rgb, self=self, f=self._colorWheelButtonHandler :
                          f( rgb ),
            relief = Tkinter.GROOVE, borderwidth = 2
            )
        self.color_wheel.pack( padx=2, pady=2, anchor=Tkinter.N )

        decls.memberFunctionAudit( self )


    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'self', 'decls', 'cur_selection' )
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        if( self.vtk_iso.getDoShowIsosurfaces()
        !=  self.visible_checkbutton.get() ):
            self.visible_checkbutton.toggle()
        self.clipping_checkbutton.set( self.vtk_iso.getIsClipped() )

        use_ghost_cells = self.vtk_iso.getUseGhostCells()
        if( use_ghost_cells !=  self.use_ghost_cells_checkbutton.get() ):
            self.use_ghost_cells_checkbutton.set( use_ghost_cells )
        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )

        self.minmax.reconfigure()

        # Cross-coloring component menu:
        cur_selection = \
            self.component_selection.component('listbox').curselection()
        if cur_selection != ():
            self.component_selection.component('listbox').select_clear(
                                                             cur_selection)
        if   self.vtk_iso.getCrossColorer() == vtk_iso.g_constant_color:
            self.component_selection.select_set(0)
        elif( (self.vtk_iso.getCrossColorer() == vtk_iso.g_self_color)
        or    (self.vtk_iso.getCrossColorer() ==
                   self.local_vtk_data.getCurComponent())):
            self.component_selection.select_set(1)
        else:
            self.component_selection.select_set(
                2 +  # 2 because first two items are '[constant]' and '[self]'.
                self.local_vtk_data.getComponentSerialNum(
                    self.vtk_iso.getCrossColorer()))

        self.color_wheel.setRgbEntries( self.vtk_iso.getTheConstantColor())
        self.line_width.setScaleSterile( self.vtk_iso.getLineWidth() )
        if not self.local_vtk_data.is2DMode():
            self.opacity_scale.setScaleSterile( self.vtk_iso.getOpacity() )

        # Register callbacks with vtk_data.visible_range_min|max.
        # Need this to enforce the colormap-constrains-isorange thing.
        map( lambda notifier, self=self :
            self.registerCallback( notifier, self.cmappedRangeCallback ),
            self.local_vtk_data.getCmappedRangeMinNotifiers()
           +self.local_vtk_data.getCmappedRangeMaxNotifiers()
           +[self.local_vtk_data.getNotifierMinVisibleLevel(),
             self.local_vtk_data.getNotifierMaxVisibleLevel()] )


        self.registerCallback(
            self.local_vtk_data.getNotifierCurComponent(),
            self.minmax.reconfigure )

        self.registerCallback( self.vtk_data.getNotifierNewComponentNotifier(),
                               self._newComponentNotifierHandler )



    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _newComponentNotifierHandler( self, compname, unused ):
        anag_utils.funcTrace()
        anag_megawidgets.appendItemToPmwScrolledListBox(
            compname, self.component_selection, 10 )


    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()

    def cleanup( self ):
        anag_utils.funcTrace()
        self.minmax.cleanup()
        self.dialog_inventory.unregisterDialog( self )
        self.withdraw()


    def _paddedBoxesCallback( self, on_off, dummy ):
        anag_utils.funcTrace()
        use_ghost_cells = self.vtk_iso.getUseGhostCells()
        if( use_ghost_cells !=  self.use_ghost_cells_checkbutton.get() ):
            self.use_ghost_cells_checkbutton.set( use_ghost_cells )


    def _componentSelectionHandler( self, component_selection ):
        """ Handler for selection events in the component-choice menu """
        anag_utils.funcTrace()
        choice = component_selection.getcurselection()[0]
        self.vtk_iso.changeColor( choice )


    def cmappedRangeCallback( self, visible_range, extra_info ):
        anag_utils.funcTrace()
        self.minmax.reconfigure()


    def _colorWheelButtonHandler( self, rgb ):
        """ Handler for <Button-1> on the ColorWheel.
            Arg rgb is the (r,g,b) coordinates of the point clicked on the
                colorwheel.
        """
        anag_utils.funcTrace()
        self.vtk_iso.changeColor( self.vtk_iso.getCrossColorer(), rgb )


class _MinMaxNum( Tkinter.Frame, SelfControl ):
    """
    Min, Max and Num Scales.
    """

    def getShortDescription(self):
        return "Min, Max and Num scales"
    def getLongDescription(self):
        return "Min, Max and Num scales"

    def __init__( self, dep_dict, master, **kw ):
        """ 
        Lay out the widgets.

        After calling this constructor, you'll want to call self.reconfigure().
        Otherwise everything will be initialized to zero and there won't be
        any input checking.
        """
        anag_utils.funcTrace()

        Tkinter.Frame.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            {'name':'range'},
            {'name':'min_scale'},
            {'name':'max_scale'},
            {'name':'num_scale'},
            {'name':'legal_min'},
            {'name':'legal_max'},
            {'name':'range_lock', 'set':1, 'save':2, 'initval':0},
            {'name':'range_lock_button'},
            {'name':'cmap_constrains_range', 'set':1, 'save':2, 'initval':0 },
            {'name':'cmap_constrains_range_button'}
          ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )

        decls = anag_utils.Declarations( 'decls', 'self', 'dep_dict', 'master',
            'vtk_data', 'vtk_iso', 'kw', 'saved_states',
            'min_frame', 'num_scale_label', 'num_scale_frame' )

        self.range_lock_button = anag_megawidgets.Checkbutton(
            self,
            text = 'range lock' )
        self.range_lock_button.configure( 
            command = lambda self=self, butt=self.range_lock_button:
                self.setRangeLock( butt.get() ))
        self.range_lock_button.set( self.range_lock )

        self.cmap_constrains_range_button = anag_megawidgets.Checkbutton(
            self,
            text = 'cmap constrains range' )
        self.cmap_constrains_range_button.configure( 
            command = self._handleCmapConstrainsRangeButton )
        self.cmap_constrains_range_button.set( self.cmap_constrains_range )

    
        self.min_scale = anag_megawidgets.EntryScale(
            self,
            length=102,
            button_text = 'Min',
            scale_callback = self._minScaleHandler,
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1
            )

        self.max_scale = anag_megawidgets.EntryScale(
            self,
            length=102,
            button_text='Max',
            scale_callback = self._maxScaleHandler,
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1
            )

        self.num_scale = anag_megawidgets.EntryScale(
            self,
            length=102,
            button_text = 'Num',
            from_=1, to=100, discrete=1,
            dep_dict = {'saved_states':self.saved_states },
            scale_callback = self._numScaleHandler,
            no_validation=1
            )
        
        
        #
        # Packing
        #
        self.num_scale.pack(expand=1,fill=Tkinter.X, padx=20, anchor=Tkinter.W);
        self.min_scale.pack(expand=1,fill=Tkinter.X, padx=20, anchor=Tkinter.W);
        self.max_scale.pack(expand=1,fill=Tkinter.X, anchor=Tkinter.W, padx=20);
        self.range_lock_button.pack( anchor=Tkinter.W, padx=20)
        self.cmap_constrains_range_button.pack( anchor=Tkinter.W, padx=20)


        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()
        self.range_lock_button.set( self.range_lock )
        self.cmap_constrains_range_button.set( self.cmap_constrains_range )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def _handleCmapConstrainsRangeButton( self ):
        anag_utils.funcTrace()
        self.setCmapConstrainsRange( self.cmap_constrains_range_button.get() )
        self.reconfigure()


    def reconfigure( self, dummy1=None, dummy2=None ):
        """
        Heavy-handed no-questions-asked reset of all the widgets.  Useful for
        component-switches.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'absdiff', 'precision',
            'self', 'min_val', 'max_val', 'dummy1', 'dummy2',
            'range_min', 'range_max', 'range_resolution', 'range_precision',
            'scale_resolution', 'scale_soft_max'
            )

        if self.local_vtk_data.getCurComponent() == None:
            return                                         # Early return

        absdiff = self.local_vtk_data.getRangeMax() -\
                  self.local_vtk_data.getRangeMin()
        if absdiff != 0:
            precision = 3 - int(math.floor(math.log10( absdiff )))
        else:
            precision = 3
        if self.cmap_constrains_range == 1:
            self.legal_min = algorithms.roundDown(
                            self.local_vtk_data.getCmappedRangeMin(),
                            precision )
            self.legal_max = algorithms.roundUp(
                            self.local_vtk_data.getCmappedRangeMax(),
                            precision )
        else:
            # Rounding to k_min_scale_resolution_order prevents segfault in
            # Tk.Scale widget, which happens if resolution is under 1E-80 or so.
            self.legal_min = round( self.local_vtk_data.getRangeMin(),
                                    self.min_scale.k_min_scale_resolution_order)
            self.legal_max = round( self.local_vtk_data.getRangeMax(),
                                    self.min_scale.k_min_scale_resolution_order)

#       Don't round too much, or else the first time you pop up this dialog,
#       vtk_iso will do a vtk update, thinking the min and max values have
#       changed.  (But rounding to a tiny number like
#       k_min_scale_resolution_order is fine.
        min_val = round(self.vtk_iso.getMinIso(),
                        self.min_scale.k_min_scale_resolution_order)
        max_val = round(self.vtk_iso.getMaxIso(),
                        self.max_scale.k_min_scale_resolution_order)

        self.num_scale.setScaleSterile( self.vtk_iso.getNumIsos() )

        range_resolution = min(
            algorithms.findResolution( min_val ),
            algorithms.findResolution( max_val ),
            algorithms.findResolution( (self.legal_max-self.legal_min)/50.0 ))

        if range_resolution == 0:
            range_precision = 1
        else:
            range_precision = int(-math.floor(math.log10( range_resolution )))
        self.range = round( max_val - min_val, range_precision )
        range_min = algorithms.roundDown( self.local_vtk_data.getRangeMin(),
                                          range_precision )
        range_max = algorithms.roundUp(   self.local_vtk_data.getRangeMax(),
                                          range_precision )
        if range_max <= range_min:
            scale_resolution = 1
        else:
            scale_resolution =\
                algorithms.roundDown10( ( range_max - range_min )/50.0 )

        if self.range_lock == 1:
            scale_soft_max = self.legal_max - self.range
        else:
            scale_soft_max = self.legal_max
        self.min_scale.configure(
            scale_normal_resolution = scale_resolution,
            resolution = scale_resolution,
            from_=range_min, to=range_max,
            scale_soft_min = self.legal_min,
            scale_soft_max = scale_soft_max )
        self.min_scale.setScaleSterile( min_val )

        self.max_scale.configure(
            scale_normal_resolution = scale_resolution,
            resolution = scale_resolution,
            from_=range_min, to=range_max,
            scale_soft_min = self.legal_min,
            scale_soft_max = self.legal_max )
        self.max_scale.setScaleSterile( max_val )

        # The full left-to-right range of the Scale never changes (it's
        # always range_min and range_max).  But the farthest right and left it
        # can go (soft_max and soft_min) is determined by (1) the difference
        # between the (max) Entry and the (min) Scale, and (2) further
        # constraints imposed from the cmap module, where the user can restrict
        # the range of values that span the entire colormap range (activated
        # by self.cmap_constrains_range).

        decls.memberFunctionAudit(self)


    def _minScaleHandler( self, val ):
        """ Arg val is the setting of the Scale component of the EntryScale.
        
            Move the Max entry to reflect the new min, i.e. the value on the
            scale.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self", "val" )

        val = float(val)
        if algorithms.floatEquals( val, self.vtk_iso.getMinIso(),
          (self.local_vtk_data.getRangeMax()-self.local_vtk_data.getRangeMin())/
          1000.0 ):
            return

        constrained_val = max( min( self._getMaxScaleValue(), val ),
                               self.legal_min )
        self.min_scale.setScaleSterile( constrained_val )
        if self.range_lock == 1:
            self.max_scale.setScaleSterile(
                            min( self.legal_max,
                                max( self._getMinScaleValue() + self.range,
                                     self.legal_min )))

        self.vtk_iso.changeContours( int(self._getNumScaleValue()),
                                     self._getMinScaleValue(),
                                     self._getMaxScaleValue() )
        decls.memberFunctionAudit( self )


    def _maxScaleHandler( self, val ):
        """ Arg val is the contents of the EntryField.
            
            Set the range.  Don't worry about the legal max, as that will have
            been enforced by EntryField's input checking (its validate config
            option).
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self',
            'val', 'constrained_val', 'epsilon' )

        if algorithms.floatEquals( val, self.vtk_iso.getMaxIso(),
          (self.local_vtk_data.getRangeMax()-self.local_vtk_data.getRangeMin())/
          1000.0 ):
            return

        # Abide by the constraints.
        constrained_val = min( max( self._getMinScaleValue(), val ),
                               self.legal_max )
        self.max_scale.setScaleSterile( constrained_val )
        self.range = constrained_val - self._getMinScaleValue()
        if self.range_lock == 1:
            self.min_scale.configure(
                scale_soft_max = self.legal_max - self.range )
        else:
            self.min_scale.configure(
                scale_soft_max = self.legal_max )

        self.vtk_iso.changeContours( int(self._getNumScaleValue()),
                                     self._getMinScaleValue(),
                                     self._getMaxScaleValue() )

        decls.memberFunctionAudit( self )


    def _numScaleHandler( self, n ):
        anag_utils.funcTrace()
        if n == self.vtk_iso.getNumIsos():
            return
        self.vtk_iso.changeContours( int(self._getNumScaleValue()),
                                     self._getMinScaleValue(),
                                     self._getMaxScaleValue() )


    def _getMinScaleValue( self ):
        return self.min_scale.getScaleValue()
    def _setMinScaleValue( self, x ):
        self.min_scale.setScaleValue( x )
    def _getMaxScaleValue( self ):
        return self.max_scale.getScaleValue()
    def _setMaxScaleValue( self, x ):
        self.max_scale.setScaleValue( x )
    def _getNumScaleValue( self ):
        return self.num_scale.getScaleValue()

# End of class _MinMaxNum

###
### Unit test for _MinMaxNum class.
###
if __name__ == '__main__':
    root = Tkinter.Tk()
    iw = _MinMaxNum( root )
    iw.reconfigure( -10, 20, 10, 0 )
    iw.updateWidgets( 0 )
    iw.pack()
    root.mainloop()
