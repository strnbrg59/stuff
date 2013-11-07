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

# File: misc_api.py
# Author: TDSternberg
# Created: 10/3/2003

import anag_utils
import visualizable_dataset
import vtk_line_plot
import libVTKChomboPython
import Tkinter
import anag_megawidgets
from self_control import SelfControl

class MiscApi( SelfControl ):
#Cut to here
    """ Methods so far unclassifiable under other API classes. """
#Cut from here
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        instance_vars = [
            {'name':'enable_alpha_features', 'notify':1},
            {'name':'vtk_update', 'notify':1},
            {'name':'fog_enabled', 'initval':0}
          ]
        SelfControl.__init__( self, dep_dict, instance_vars)
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls','self','instance_vars',
            'dep_dict' )

        decls.memberFunctionAudit(self)

    #
    # Functions required by base class SelfControl.
    #
    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()
    def cleanup( self ):
        anag_utils.funcTrace()

#Cut to here

    def getTopWindow( self ):
        """
        Return the top-level window -- the one that holds the menubar and the
        VTK display.  Call this function if you want to add some buttons of
        your own.
        """
        result = self.vtk_vtk.getTopWindow()
        return result


    def vtkUpdate( self ):
        """
        Updates the Vtk pipelines.  It's necessary to call this at the end of
        a user script (but nowhere else).
        """
        anag_utils.apiTrace()
        if self.saved_states.getNumHDF5sLoaded() == 0:
            return
        self.setVtkUpdate(1) # notifier


    def guiUpdate( self ):
        """
        Brings all the GUI controls to a state consistent with that of the rest
        of ChomboVis.
        """
        anag_utils.apiTrace()
        self.vtk_data.disableUpdate()
        self.vtkUpdate()
        self.vtk_data.enableUpdate()


    def mainloop(self):
        """
        Starts the GUI event loop.  Call this at the end of a user script to
        hand off control to the GUI.
        """
        anag_utils.apiTrace()
        self.vtk_vtk.getTopWindow().mainloop()


    def tkUpdate(self):
        """
        Causes Tkinter to render your widgets.  Not necessary if you call
        mainloop().
        """
        anag_utils.apiTrace()
        self.vtk_vtk.getTopWindow().update()


    def setDebugLevel(self,n):
        """
        Results in varying amounts of debug output.  Of interest to developers,
        mostly.  Legal values for arg n are 0 (nothing), 1 (fatal errors only
        print), 2 (errors and fatal errors), 3 (warnings, errors and fatal
        errors), 4 (info, warnings, errors and fatal errors), 5 (trace, info,
        ...).  Affects the Python layer and, if hdf5 data has been loaded, the
        C++ layer as well.
        """
        anag_utils.apiTrace()
        self.vtk_data.setDebugLevel( n )


    def getDebugLevel(self):
        """ See comments under setDebugLevel(). """
        anag_utils.apiTrace()
        result = self.vtk_data.getDebugLevel()
        return result


    def setRenderWindowSizeRestrictedToMultiplesOf16( self, b ):
        """
        If b==1, restricts the size of the render window to a multiple
        of 16.  If b==0, size is unrestricted.  Certain movie players balk at
        playing movies made from frames whose size are not multiples of 16
        pixels.
        """
        anag_utils.apiTrace()
        self.vtk_vtk.setRenWinSizeRestrictedToMultiplesOf16( b )


    def saveState( self, file_name ):
        """
        Saves, to a file, the state of the visualization.  See also
        restoreState().
        """
        anag_utils.apiTrace()
        self.saved_states.saveState( file_name )


    def restoreState( self, file_name, gui_only=0 ):
        """
        Restores, from file, a visualization state previously saved with
        saveState().

        If arg gui_only is 1, then we restore only GUI-specific state (e.g.
        whether dialogs were open, whether EntryScales were in scale or entry
        mode, etc.)
        """
        anag_utils.apiTrace()
        state = self.saved_states.loadState( file_name, gui_only )


    def hardCopy( self, outfile_name, mag_factor=1, format_extension='ppm' ):
        """
        Produces a file with an image of what is currently in the Vtk window.
        Arg mag_factor can be any integer.
        Arg format_extension can be 'ppm', 'bmp', 'tiff', or 'eps'.  Setting
        it to 'eps' results in a whole different kind of picture -- an encapsulated
        postscript thing with vector graphics (for some of what's been rendered
        anyway, and mag_factor doesn't do anything in this case).
        When in offscreen=1 mode, args mag_factor and format_extension have no
        effect.  To specify the size in offscreen mode, you need to invoke
        misc.setRenderWidgetSize() in your script as early as possible.
        """
        anag_utils.apiTrace()

        if self.cmd_line.getOffScreen() == 1:
            self.vtkUpdate()
        else:
            self.tkUpdate()
        self.vtk_print.hardCopy( outfile_name, mag_factor, format_extension )


    def getViewParamsObj( self ):
        """
        Returns an object suitable for passing to self.setViewParamsOO().
        """
        anag_utils.apiTrace()
        v = self.vol_tk_interactor
        class Obj:
            pass
        obj = Obj()
        obj.camera_position      = v.getCameraPosition()
        obj.axes_camera_position = v.getAxesCameraPosition()
        obj.focal_point          = v.getFocalPoint()
        obj.world_point          = v.getWorldPoint()
        obj.display_point        = v.getDisplayPoint()
        obj.view_up              = v.getViewUp()
        obj.clipping_range       = v.getClippingRange()
        obj.parallel_scale       = v.getParallelScale()
        return obj


    def setViewParamsOO( self, obj ):
        """
        The "OO" is for "object-oriented".  Same as setViewParams() except
        instead of passing all the params in one by one we wrap them up in
        a single object.  You can obtain such an object from getViewParamsObj().
        """
        anag_utils.apiTrace()
        self.setViewParams(
            obj.camera_position,
            obj.axes_camera_position,
            obj.focal_point,
            obj.world_point,
            obj.display_point,
            obj.view_up,
            obj.clipping_range,
            obj.parallel_scale )


    def setViewParams( self, camera_position, axes_camera_position,
        focal_point, world_point, display_point, view_up, clipping_range,
        parallel_scale ):
        """
        Specifies all the parameters needed to position the image, zoom it, etc.
        In the GUI, you can get a printout of your current view parameters by
        selecting tools-->camera parameters and then clicking the "dump" button
        on the dialog that comes up.
        """
        anag_utils.apiTrace()

        v = self.vol_tk_interactor

        v.setCameraPosition( camera_position )
        v.setAxesCameraPosition( axes_camera_position )
        v.setFocalPoint( focal_point )
        v.setWorldPoint( world_point )
        v.setDisplayPoint( display_point )
        v.setViewUp( view_up )
        v.setAxesViewUp( view_up )
        v.setClippingRange( clipping_range )
        v.setParallelScale( parallel_scale )
        v.refresh()

    def parallelProjection( self ):
        # Parallel, as opposed to perspective, projection.
        # FIXME: Can't zoom in and out.  Study the zoom() function in
        # vol_tk_interactor to figure out how to do this.
        anag_utils.apiTrace()

        camera = self.vtk_vtk.getRenderer().GetActiveCamera()
        camera.ParallelProjectionOn()
        range_ = camera.GetClippingRange()
        self.vtk_cameras.setClippingRange( range_[0] / 4.0, range_[1] * 4.0 )


    def setRenderWidgetSize( self, width, height ):
        """
        The render widget is the complete window, comprising the VTK display(s)
        plus the Tk widgets that, optionally, lie beneath.

        Width and height get rounded to even numbers, as mpeg2encode doesn't
        like odd numbers.
        """
        anag_utils.apiTrace()
        self.vtk_vtk.setRenWidgetSize( width, height )


    #
    # Databrowser
    #
    def showDatabrowserLauncher( self ):
        """ Pop up the databrowser launcher dialog. """
        anag_utils.apiTrace()
        self.control_fab_tables.showGUI()


    def hideDatabrowserLauncher( self ):
        """ Close the databrowser launcher dialog. """
        anag_utils.apiTrace()
        self.control_fab_tables.withdrawGUI()


    def makeDatabrowser( self, component, level, box_num ):
        """
        Display a data browser.  If arg component is set to '', then we bring
        up a data browser for the first component.
        """
        anag_utils.apiTrace()
        if component == '':
            component = self.vtk_data.getComponentList()[0]
        self.control_fab_tables.makeDatabrowser( component, level, box_num )


    def withdrawDatabrowser( self, component, level, box_num ):
        """
        Close a data browser.
        """
        anag_utils.apiTrace()
        self.control_fab_tables.withdrawDatabrowser( component, level, box_num )


    def setDatabrowserFontFamily( self, family ):
        """
        The set of recognized font family names varies from one computer system
        to the next, but good bets are 'Arial', 'Courier', 'Times' and 'System'.
        Default is 'System'.
        """
        anag_utils.apiTrace()
        self.control_fab_tables.setFontFamily( family )

    def setDatabrowserFontStyle( self, style ):
        """
        Legal arguments are 'normal', 'bold', 'roman', 'italic', 'underline',
        and 'overstrike'.
        """
        anag_utils.apiTrace()
        self.control_fab_tables.setFontStyle( style )
        

    def setDatabrowserFontSize( self, points ):
        """
        Argument's units is in points.
        """
        anag_utils.apiTrace()
        self.control_fab_tables.setFontSize( points )


    #
    # End of databrowser section
    #


    def closeAllDialogs( self ):
        """
        Closes all the dialogs -- data, grids, iso, slices, etc.
        """
        anag_utils.apiTrace()
        self.dialog_inventory.closeAllDialogs()


    def reopenDialogs( self ):
        """
        Reopen the dialogs that were open the last time closeAllDialogs() was
        called.
        """
        anag_utils.apiTrace()
        self.dialog_inventory.reopenDialogs()


    def flashDialogs( self ):
        """
        Briefly close all dialogs, then reopen them.
        """
        anag_utils.apiTrace()
        self.dialog_inventory.flashDialogs()


    def enableAlphaFeatures( self, on_off ):
        """
        Alpha features are volume and embedded boundaries.
        """
        anag_utils.apiTrace()
        anag_utils.warning( 'Deprecated' )


    def makeLinePlot( self, p0, p1, n, component, finest_level ):
        """
        Return a VtkLinePlot object (see vtk_line_plot.py) which encapsulates n
        equally spaced points on the line connecting p0 and p1 (inclusive).
        In each point the first element is the fraction of
        the distance along the line (0,1/(n-1), 2/(n-1),...,1), and the second
        element is the value, at that point, of the indicated field component.
     
        Args p0 and p1 are points in 2D or 3D space, represented as tuples.
        Arg n must be greater than 1.
        Arg finest_level is the finest level at which to look for cells; for
        each point on the line, we look first at finest_level, then (if there's
        no box there at that level) we move down through the levels, all the
        way to the zero-th level.

        If any of the coordinates fall out of bounds, the LinePlot object will
        just omit it and the data will have a gap.

        The position of the lineplot's domain is indicated with a yellow segment
        drawn in the VTK display.

        To turn off the yellow segment, call the LinePlot object's clear()
        function.
        """
        anag_utils.apiTrace()
        visdat = self.vtk_data.getVisualizableDataset()
        result = vtk_line_plot.VtkLinePlot(
            visdat.getLinePlot( p0,p1,n,component,finest_level), p0, p1,
            dep_dict = {'vtk_vtk':self.vtk_vtk} )
        result.turnOn()
        return result


    def setOptimizationMode( self, mode ):
        """
        Choices are "memory" and "speed".
        If arg is "speed", then no FABs are ever deleted.  (We'll still only
        load them as needed, but once they're loaded, they're never deleted as,
        otherwise, they would be as they appear to not be needed.)

        Changing the optimization mode from speed to memory will not,
        unfortunately, delete any already-loaded FABs; the effect will only be
        to delete (as they appear to be unneeded) FABs not loaded to this point.
        
        By default, ChomboVis comes up in speed-optimization mode.
        """
        anag_utils.apiTrace()
        assert( (mode=='memory') or (mode=='speed') )

        self.vtk_data.setOptimizationMode( {'memory':0, 'speed':1}[ mode ] )

    
    def getReleaseNumber( self ):
        anag_utils.apiTrace()
        return vtk_data.getReleaseNumber()


    def enableFog( self ):
        """
        Turn on OpenGL fog.  See other fog-related functions here for control
        over fog parameters.
        """
        anag_utils.apiTrace()
        raw = libVTKChomboPython.vtkChomboRawGL()
        renwin=self.vtk_vtk.getRenWin()
        renwin.MakeCurrent()
        raw.SetFog(1)
        bkgrnd = list( self.vtk_cmap.getBackgroundColor() )
        color_alpha = 1.0
        raw.SetFogColor( bkgrnd[0], bkgrnd[1], bkgrnd[2], color_alpha )
        self.vtk_vtk.render()
        self.fog_enabled = 1


    def disableFog( self ):
        """
        Turn off OpenGL fog.
        """
        anag_utils.apiTrace()
        raw = libVTKChomboPython.vtkChomboRawGL()
        renwin=self.vtk_vtk.getRenWin()
        renwin.MakeCurrent()
        raw.SetFog(0)
        self.vtk_vtk.render()
        self.fog_enabled = 0


    def setFogStart( self, x ):
        """
        The density of (linear) fog grows from 0 at the "start" point to 1.0
        (zero visibility) at the "end" point.  This function sets the "start"
        point as a distance from the graphics system viewpoint.
        """
        anag_utils.apiTrace()
        if self.fog_enabled == 0:
            self.enableFog()
        raw = libVTKChomboPython.vtkChomboRawGL()
        renwin=self.vtk_vtk.getRenWin()
        renwin.MakeCurrent()
        raw.SetFogStart( float(x) )
        self.vtk_vtk.render()

    def setFogEnd( self, x ):
        """
        The density of (linear) fog grows from 0 at the "start" point to 1.0
        (zero visibility) at the "end" point.  This function sets the "end"
        point as a distance from the graphics system viewpoint.
        """
        anag_utils.apiTrace()
        if self.fog_enabled == 0:
            self.enableFog()
        raw = libVTKChomboPython.vtkChomboRawGL()
        renwin=self.vtk_vtk.getRenWin()
        renwin.MakeCurrent()
        raw.SetFogEnd( float(x) )
        self.vtk_vtk.render()

    def _toggleFog( self, b ):
        if b == 0:
            self.disableFog()
        else:
            self.enableFog()


    def showFogScales( self ):
        """
        Pop up two scale widgets to control fog begin and end with.
        """
        anag_utils.apiTrace()
 
        root = Tkinter.Tk()
        root.wm_title( "Linear fog" )

        toggle = anag_megawidgets.Checkbutton( root, text='activated' )
        toggle.configure( command = lambda butt = toggle, self=self:
            self._toggleFog( butt.get() ) )
        toggle.pack()

        fog_frame = Tkinter.Frame( root,
            relief='groove', borderwidth=2 )
        fog_start=anag_megawidgets.EntryScale(
            fog_frame, button_text='start',
            from_=0.1,to=50.0, scale_normal_resolution=0.1, resolution=0.1,
            no_validation=1 )
        fog_start.configure(
            scale_callback = lambda x, self=self: self.setFogStart(float(x)) )
        fog_start.pack(  side='top')
        fog_start.setScaleSterile( 10.0 )
        fog_end=anag_megawidgets.EntryScale(
            fog_frame, button_text='end',
            from_=0.1,to=50.0, scale_normal_resolution=0.1, resolution=0.1,
            no_validation=1 )
        fog_end.configure(
            scale_callback = lambda x, self=self: self.setFogEnd(float(x)) )
        fog_end.pack(  side='top')
        fog_end.setScaleSterile( 30.0 )
        fog_frame.pack( anchor='w', side='left' )
