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

# Author: TDSternberg
# Created: 5/29/01

"""
Can this be replaced with a vtkRenderWindowInteractor?
My experiments to that end have so far been unsuccessful, because the
program hangs on vtkRenderWindowInteractor.Start().
See ../experi/renderwidget_plus_interactor.py.
"""

import sys
import anag_utils
import Tkinter
import Pmw
import tkMessageBox
import vtkpython
from self_control import SelfControl

class VolTkInteractor( SelfControl ):

    def __init__(self, dep_dict):
        SelfControl.__init__( self,dep_dict,
          [
            { 'name':'active_renderer_found', 'initval':0 },
            { 'name':'current_camera', 'initval':None},
            { 'name':'current_axes_camera', 'initval':None},
            { 'name':'current_light', 'initval':None},
            { 'name':'current_render_window', 'initval':None},
            { 'name':'current_axes_render_window', 'initval':None},
            { 'name':'current_renderer', 'initval':None},
            { 'name':'current_axes_renderer', 'initval':None},
            { 'name':'window_centerX'},
            { 'name':'window_centerY'},
            { 'name':'lastX'},
            { 'name':'lastY'},
            { 'name':'camera_position', 'save':5, 'get':1, 'initval':None,
              'notify':1 },
            { 'name':'axes_camera_position', 'save':5, 'get':1,'initval':None,
              'notify':1 },
            { 'name':'focal_point', 'save':5, 'get':1, 'initval':None,
              'notify':1},
            { 'name':'clipping_range', 'save':5, 'get':1, 'set':2,
              'initval':None, 'notify':1},
            { 'name':'world_point', 'save':5, 'get':1, 'initval':None,
              'notify':1},
            { 'name':'display_point', 'save':5, 'get':1, 'initval':None,
              'notify':1},
            { 'name':'view_up',      'get':1, 'save':5, 'initval':None,
               'notify':1},
            { 'name':'axes_view_up', 'get':1, 'save':5, 'initval':None,
               'notify':1},
            { 'name':'parallel_scale', 'save':5, 'get':1, 'initval':None,
              'notify':1},

            { 'name':'motion_is_disabled', 'initval':0},
            { 'name':'event_bindings'},
            { 'name':'picker'}
          ])
        self.decls = anag_utils.Declarations( "decls", instance=self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        if self.cmd_line.getNoVtk():
            return

        # FIXME: self._updateRenderer() is set up to deal with multiple
        # vtkRenderers.  For now, we don't have more than one, but when and
        # if we do (Terry's little x-y-z axis display?) we'll have to
        # change this.
        self.current_camera = self.vtk_vtk.getRenderer().GetActiveCamera()

        self.picker = vtkpython.vtkCellPicker()
        if self.vtk_data.getNumParticles() != 0:
            self.picker.AddObserver("EndPickEvent", self._endPickFunc )

        if self.cmd_line.getUseRenderWidget() == 1:
            self.current_axes_camera =\
                self.vtk_vtk.getAxesRenderer().GetActiveCamera()
        self.current_renderer = self.vtk_vtk.getRenderer()
        self.current_axes_renderer = self.vtk_vtk.getRenderer()

        # Initialize things so we can do an adequate restore of the initial
        # position.
        self._setSelfCameraParams()

        # Define key and mouse event bindings.  The corresponding rules apply
        # to the main VTK window.  For the little axes window, we set every
        # rule to 0, i.e. we disable all independent control over the little
        # axes.
        self.event_bindings = {
            '<Any-ButtonPress>': lambda e, self=self:
                self._startMotion( e.x, e.y ),
    
            '<Any-ButtonRelease>': lambda e, self=self:
                self._endMotion( e.x, e.y ),
    
            '<ButtonPress-1>': lambda e, self=self:
                self._startMotion( e.x, e.y ),
            '<ButtonPress-2>': lambda e, self=self:
                self._startMotion( e.x, e.y ),
            '<ButtonPress-3>': lambda e, self=self:
                self._startMotion( e.x, e.y ),
    
            '<B1-Motion>': lambda e, self=self: self._rotate( e.x, e.y ),
            '<B2-Motion>': lambda e, self=self: self._pan( e.x, e.y ),
            '<B3-Motion>': lambda e, self=self: self._zoom( e.x, e.y ),
    
            '<ButtonRelease-1>': lambda e, self=self:
                self._endMotion( e.x, e.y ),
            '<ButtonRelease-2>': lambda e, self=self:
                self._endMotion( e.x, e.y ),
            '<ButtonRelease-3>': lambda e, self=self:
                self._endMotion( e.x, e.y ),
    
            '<Shift-ButtonPress-1>': lambda e, self=self:
                self._startMotion( e.x, e.y ),
            '<Shift-ButtonPress-2>': 0,
            '<Shift-ButtonPress-3>': 0,
    
            '<Shift-B1-Motion>': lambda e, self=self:
                self._pan( e.x, e.y),
            '<Shift-B2-Motion>': 0,
            '<Shift-B3-Motion>': 0,
    
            '<Shift-ButtonRelease-1>': lambda e, self=self:
                self._endMotion( e.x, e.y ),
            '<Shift-ButtonRelease-2>': 0,
            '<Shift-ButtonRelease-3>': 0,
    
            '<Control-ButtonPress-1>': self._handleCtrlButt1,
            '<Control-ButtonPress-2>': self._handleCtrlButt2,
            '<Control-ButtonPress-3>': self._handleCtrlButt3,
    
            '<Control-B1-Motion>': 0,
            '<Control-B2-Motion>': 0,
            '<Control-B3-Motion>': 0,
    
            '<Control-ButtonRelease-1>': lambda e, self=self:
                self._enableMotion(),
            '<Control-ButtonRelease-2>': lambda e, self=self:
                self._enableMotion(),
            '<Control-ButtonRelease-3>': lambda e, self=self:
                self._enableMotion(),
    
            '<Control-Shift-ButtonPress-1>': self._handleCtrlButt2,
            '<Control-Shift-ButtonPress-2>': 0,
            '<Control-Shift-ButtonPress-3>': 0,
    
            '<Control-Shift-B1-Motion>': 0,
            '<Control-Shift-B2-Motion>': 0,
            '<Control-Shift-B3-Motion>': 0,
    
            '<Control-Shift-ButtonRelease-1>': lambda e, self=self:
                self._enableMotion(),
            '<Control-Shift-ButtonRelease-2>': 0,
            '<Control-Shift-ButtonRelease-3>': 0,
    
            '<Enter>': lambda e, self=self:
                self._updateRenderer( e.widget, e.x, e.y ),
            '<Expose>': lambda e, self=self: self._expose( e.widget )
        }
    
        # We don't want any bindings unless there's a loaded hdf5, because
        # some of the bindings try to interact with a rendering:
        if self.cmd_line.getOffScreen() != 1:
            if self.cmd_line.getUseRenderWidget() == 1:
                self._disableBindings( self.vtk_vtk.getAxesRenWidget() )
                self._bindTkRenderWidget( self.vtk_vtk.getRenWidget() )
            else:
                t = Tkinter.Text( self.vtk_vtk.getTopWindow() )
                t.pack()
                self._bindTkRenderWidget( t )


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.cmd_line.getNoVtk():
            return

        if self.getCameraPosition():
            # Restored from state dictionary; initval=None.
            self.current_camera.SetPosition( self.getCameraPosition()[0],
                                             self.getCameraPosition()[1],
                                             self.getCameraPosition()[2] )
            if self.cmd_line.getUseRenderWidget() == 1:
                self.current_axes_camera.SetPosition(
                                    self.getAxesCameraPosition()[0],
                                    self.getAxesCameraPosition()[1],
                                    self.getAxesCameraPosition()[2] )
            if self.getFocalPoint():
                self.current_camera.SetFocalPoint( self.getFocalPoint()[0],
                                                   self.getFocalPoint()[1],
                                                   self.getFocalPoint()[2] )

            # Omit: causes problems when loading 3D state into reslice mode,
            # and doesn't seem necessary anyway:
            if self.getClippingRange():
                self.current_camera.SetClippingRange( self.getClippingRange() )

            if self.getWorldPoint() and self.current_renderer:
                self.current_renderer.SetWorldPoint(
                    self.getWorldPoint()[0], self.getWorldPoint()[1],
                    self.getWorldPoint()[2], self.getWorldPoint()[3] )
            if self.getDisplayPoint() and self.current_renderer:
                self.current_renderer.SetDisplayPoint(
                    self.getDisplayPoint()[0],
                    self.getDisplayPoint()[1],
                    self.getDisplayPoint()[2] )
            if self.getViewUp():
                self.current_camera.SetViewUp( self.getViewUp()[0],
                                               self.getViewUp()[1],
                                               self.getViewUp()[2])
                if self.cmd_line.getUseRenderWidget() == 1:
                    self.current_axes_camera.SetViewUp( self.getAxesViewUp()[0],
                                                   self.getAxesViewUp()[1],
                                                   self.getAxesViewUp()[2])
            if self.getParallelScale():
                self.current_camera.SetParallelScale( self.getParallelScale())
        else:
            self.setCameraPosition( self.current_camera.GetPosition() )
            if self.cmd_line.getUseRenderWidget() == 1:
                self.setAxesCameraPosition(
                                        self.current_axes_camera.GetPosition() )

        self.current_camera.OrthogonalizeViewUp()
        if self.cmd_line.getUseRenderWidget() == 1:
            self.current_axes_camera.OrthogonalizeViewUp()


    def refresh( self ):
        """ For calling from outside. """
        self._refresh()
        self.vtk_vtk.render()


    def cleanup( self ):
        anag_utils.funcTrace()
#       This raises an exception...
#       self.picker.RemoveObserver(self._endPickFunc)


    def _bindTkRenderWidget( self, widget ):
        anag_utils.funcTrace()

        for k in self.event_bindings.keys():
            widget.bind( k, self.event_bindings[k] )


    def _disableBindings( self, widget ):
        """ Disable any direct interaction with the little axes. """
        anag_utils.funcTrace()
        for k in self.event_bindings.keys():
            if k in ( '<B1-Motion>', '<B2-Motion>', '<B3-Motion>'):
                widget.bind( k, 0 )
            else:
                widget.bind( k, self.event_bindings[k] )

    
    """
    Deal with ctrl-button-1, 2 and 3 -- the buttons that control the
    selector skewer and box cycling.  Each button does something unique,
    but what they have in common is that we want to disable motion (else
    some people tend to zoom involutarily when operating ctrl-butt-3).
    """
    def _handleCtrlButt1( self, event ):
        anag_utils.funcTrace()
        self._disableMotion()
        renwinsize = self.vtk_vtk.getRenWin().GetSize()

        # Trigger self._endPickFunc():
        #self.picker.SetTolerance(0.01)  #FIXME: should be adaptive
        self.picker.Pick( event.x, renwinsize[1] - event.y, 0,
                          self.vtk_vtk.getRenderer() )

        if(    (not self.picker.GetActor())
            or (self.picker.GetActor() != self.vtk_particles.getActor()) ):
            self.selector.turnOn( event.x, event.y )
        self._render()


    def _handleCtrlButt2( self, event ):
        anag_utils.funcTrace()
        self._disableMotion()
        self.vtk_particles.unhighlightParticle()
        self.selector.turnOff(),
        self._render()


    def _handleCtrlButt3( self, event ):
        anag_utils.funcTrace()
        self._disableMotion()
        self.selector.cycle()
        self._render()


    def _disableMotion( self ):
        return # This was more trouble than it was worth; it locked up the
               # display too often, and perplexingly so.
        self.motion_is_disabled = 1
    def _enableMotion( self ):
        return
        anag_utils.funcTrace()
        self.motion_is_disabled = 0


    def _expose( self, widget ):
        """ This is a much-trimmed-down version of what's in the Tcl code.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getUseRenderWidget() == 0:
            return
    
        widget.update()
        widget.GetRenderWindow().Render()

    
    def _render( self ):
        anag_utils.funcTrace()

        if self.cmd_line.getUseRenderWidget() == 0:
            self.vtk_vtk.render()
            return
     
        if( self.current_camera == None
        or  self.current_light == None
        or  self.current_render_window == None ):
            return

        self.current_render_window.Render()
        self.current_axes_render_window.Render()

    
    def _updateRenderer( self, widget, x_coord, y_coord ):
        anag_utils.funcTrace()
    
        decls = anag_utils.Declarations( 'decls', 'self',
            'widget', 'x_coord', 'y_coord',
            'windowX', 'windowY',
            'renderers',
            'num_renderers',
            'vx', 'vy',
            'viewport',
            'vpxmin', 'vpymin', 'vpxmax', 'vpymax',
            'lights',
            'i', 'win_size'
        )

        # We'd like to update vtk_vtk.ren_widget_size -- which we save to state
        # files -- so it tracks the actual size of the render widget, should the
        # user use the mouse to stretch that.  Unfortunately, I don't see how
        # to obtain the render widget's size.  It doesn't have a GetSize()
        # method, nor do __getitem__() or cget() -- despite the documentation --
        # work.  And winfo_height = winfo_width = 1 at all times.
        """
        self.vtk_vtk.setRenWidgetSize(
            self.vtk_vtk.getRenWidget().winfo_height(),
            self.vtk_vtk.getRenWidget().winfo_width() )
        """

        win_size = self.vtk_vtk.getRenWin().GetSize()
        (self.window_centerX, self.window_centerY) =\
            (win_size[0]/2, win_size[1]/2)

        self.lastX = x_coord
        self.lastY = y_coord
    
        if self.cmd_line.getUseRenderWidget() == 0:
            self.vtk_vtk.render()
            self.active_renderer_found = 1
            return

        # Get the renderer window dimensions
        windowX = int(widget.configure()['width'][4])
        windowY = int(widget.configure()['height'][4])
    
        # Find which renderer the event has occurred in
        self.current_render_window = widget.GetRenderWindow()
        self.current_axes_render_window = self.vtk_vtk.getAxesRenWin()
        renderers = self.current_render_window.GetRenderers()
        num_renderers = renderers.GetNumberOfItems()
    
        renderers.InitTraversal()
        self.active_renderer_found = 0
    
        for i in range(0, num_renderers):
    
            self.current_renderer = renderers.GetNextItem()
            vx = (x_coord + 0.0)/windowX
            vy = (windowY - (y_coord+0.0)) /windowY
            viewport = self.current_renderer.GetViewport()
            vpxmin = viewport[0]
            vpymin = viewport[1]
            vpxmax = viewport[2]
            vpymax = viewport[3]
    
            if (vpxmin <= vx <= vpxmax) and (vpymin <= vy <= vpymax) :
                self.active_renderer_found = 1
                self.window_centerX = 0.5 * windowX * (vpxmax + vpxmin)
                self.window_centerY = 0.5 * windowY * (vpymax + vpymin)
    
                break
    
        self.current_camera = self.current_renderer.GetActiveCamera()
        lights = self.current_renderer.GetLights()
        lights.InitTraversal()
        self.current_light = lights.GetNextItem()
    
        decls.memberFunctionAudit(self)
    

    def _startMotion( self, x, y ):
        anag_utils.funcTrace()
    
        widget = self.vtk_vtk.getRenWidget()
        self._updateRenderer( widget, x, y )
        if( not self.active_renderer_found
        or (self.cmd_line.getUseRenderWidget()==0)):
            return
    
        self.current_render_window.SetDesiredUpdateRate( 2 )

        # Hack!  Piercing all kinds of encapsulation with this...
        self.network_api.sendCommand( 'c.vol_tk_interactor._startMotion('
            + str(x) + ',' + str(y) + ')' )

    
    def _endMotion( self, x, y ):
        anag_utils.funcTrace()
    
        if not self.active_renderer_found:
            return
        widget = self.vtk_vtk.getRenWidget()

        if self.cmd_line.getUseRenderWidget() == 1:    
            self.current_render_window.SetDesiredUpdateRate( 0.01 )
        self._setSelfCameraParams()
        self._render()

        # Hack!  Piercing all kinds of encapsulation with this...
        self.network_api.sendCommand( 'c.vol_tk_interactor._endMotion('
            + str(x) + ',' + str(y) + ')' )


    def _setSelfCameraParams( self ):
        self.setCameraPosition( self.current_camera.GetPosition() )
        if self.cmd_line.getUseRenderWidget() == 1:
            self.setAxesCameraPosition( self.current_axes_camera.GetPosition() )
        self.setFocalPoint( self.current_camera.GetFocalPoint() )
        self.setClippingRange( self.current_camera.GetClippingRange() )
        self.setViewUp( self.current_camera.GetViewUp() )
        if self.cmd_line.getUseRenderWidget() == 1:
            self.setAxesViewUp( self.current_axes_camera.GetViewUp() )
        self.setWorldPoint( self.current_renderer.GetWorldPoint() )
        self.setDisplayPoint( self.current_renderer.GetDisplayPoint() )
        self.setParallelScale( self.current_camera.GetParallelScale() )


    def printParams(self):
        """ Called from ControlCameras. """
        anag_utils.funcTrace()
        sys.stdout.write(
            'camera_position=' + str(self.getCameraPosition()) + ',\n' +
            'axes_camera_position=' + str(self.getAxesCameraPosition()) +',\n' +
            'focal_point=' + str(self.getFocalPoint())         + ',\n' +
            'world_point=' + str(self.getWorldPoint())         + ',\n' +
            'display_point=' + str(self.getDisplayPoint())     + ',\n' +
            'view_up=' + str(self.getViewUp())                 + ',\n' +
            'parallel_scale=' + str(self.getParallelScale())   + ',\n' +
            'clipping_range=' + str(self.getClippingRange())   + '\n')


    def _endPickFunc( self, dummy1, dummy2 ):
        """
        Things to do when a particle has been picked.
        """
        anag_utils.funcTrace()
        if self.picker.GetActor() != self.vtk_particles.actor:
            anag_utils.info( "Not a particles actor." )
        elif self.vtk_particles.getOpacity() != 1.0:
            tkMessageBox.showerror( message = "Please set opacity to 1.0.\n"
                "Particle picking does not work otherwise." )
        else:
            cell_id = self.picker.GetCellId()
            anag_utils.info(
                "Num cells = ",
                self.vtk_particles.glyph.GetOutput().GetNumberOfCells())
            cells_per_glyph = self.vtk_particles.getCellsPerGlyph()
            anag_utils.info( "cell_id=", cell_id )
            anag_utils.info( "cells_per_glyph=", cells_per_glyph )
            picked_particle_num = cell_id/cells_per_glyph
            anag_utils.info( "Picked particle no.", picked_particle_num )
            self.vtk_particles.highlightParticle( picked_particle_num )
            self.vtk_data.setPickedParticle(  # Notifier
                self.vtk_particles.getSelectedParticleNumber(
                    picked_particle_num ) )


    def _rotate( self, x_coord, y_coord ):
        """
        Ctrl-Button1 rotates the display in 3D mode.  In 2D mode, you can't
        rotate unless the debug level is at least 4.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self",
            "widget", "x_coord", "y_coord", "azimuth_incr", "elevation_incr" )

        widget = self.vtk_vtk.getRenWidget()

        if(((not self.active_renderer_found)
        or  self.vtk_data.is2DMode()
        or  (self.motion_is_disabled == 1))
        and ( anag_utils.getDebugLevel() < 4 )):
            return

        azimuth_incr = (self.lastX - x_coord) / 3.0
        elevation_incr = (y_coord - self.lastY) / 3.0
        self.current_camera.Azimuth( azimuth_incr )
        self.current_camera.Elevation( elevation_incr )
        self.current_camera.OrthogonalizeViewUp()
        if self.cmd_line.getUseRenderWidget() == 1:
            self.current_axes_camera.Azimuth( azimuth_incr )
            self.current_axes_camera.Elevation( elevation_incr )
            self.current_axes_camera.OrthogonalizeViewUp()

        self.lastX = x_coord
        self.lastY = y_coord
    
        self._render()
        decls.memberFunctionAudit(self)
        
        # Hack!  Piercing all kinds of encapsulation with this...
        self.network_api.sendCommand( 'c.vol_tk_interactor._rotate('
            + str(x_coord) + ',' + str(y_coord) + ')' )
    

    def _pan( self, x_coord, y_coord ):
        anag_utils.funcTrace()
    
        decls = anag_utils.Declarations( "decls", "self",
            "widget", "x_coord", "y_coord",
            "f_point",
            "p_point",
            "d_point",
            "focal_depth",
            "a_point",
            "r_point",
            "winsize"
        )

        widget = self.vtk_vtk.getRenWidget()

        # Gotta recompute the window center, in case the user used the mouse to
        # stretch the window.  If we don't, attempts to pan will cause the
        # image to "fly" away.
        winsize=self.vtk_vtk.getRenWin().GetSize()
    
        if self.cmd_line.getUseRenderWidget() == 0:
            self.window_centerX = winsize[0]/2
            self.window_centerY = winsize[1]/2

        if( (not self.active_renderer_found)
        or  (self.motion_is_disabled == 1) ):
            return
    
        f_point = self.current_camera.GetFocalPoint()
    
        p_point = self.current_camera.GetPosition()
    
        self.current_renderer.SetWorldPoint( f_point[0],
                                             f_point[1],
                                             f_point[2],
                                             1.0 )
        self.current_renderer.WorldToDisplay()
        d_point = self.current_renderer.GetDisplayPoint()
        focal_depth = d_point[2]

        self.window_centerX = winsize[0]/2
        self.window_centerY = winsize[1]/2

        a_point = [ self.window_centerX + 2*(x_coord - self.lastX),
                    self.window_centerY - 2*(y_coord - self.lastY) ]
    
        self.current_renderer.SetDisplayPoint( a_point[0],
                                               a_point[1],
                                               focal_depth )
        self.current_renderer.DisplayToWorld()
    
        r_point = list(self.current_renderer.GetWorldPoint())
        if r_point[3] != 0.0 :
            r_point[0] = r_point[0] / r_point[3]
            r_point[1] = r_point[1] / r_point[3]
            r_point[2] = r_point[2] / r_point[3]
    
    
        self.current_camera.SetFocalPoint(
            (f_point[0] - r_point[0])/2.0 + f_point[0],
            (f_point[1] - r_point[1])/2.0 + f_point[1],      
            (f_point[2] - r_point[2])/2.0 + f_point[2] )
    
        self.current_camera.SetPosition(
            (f_point[0] - r_point[0])/2.0 + p_point[0],
            (f_point[1] - r_point[1])/2.0 + p_point[1],
            (f_point[2] - r_point[2])/2.0 + p_point[2] )
    
        self.lastX = x_coord
        self.lastY = y_coord
    
        self._render()
        decls.memberFunctionAudit(self)

        # Hack!  Piercing all kinds of encapsulation with this...
        self.network_api.sendCommand( 'c.vol_tk_interactor._pan('
            + str(x_coord) + ',' + str(y_coord) + ')' )


    def _zoom( self, x_coord, y_coord ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self",
            "widget", "x_coord", "y_coord",
            "zoom_factor",
            "scale",
            "clipping_range",
            "min_range",
            "max_range"
        )
    
        if( (not self.active_renderer_found)
        or  (self.motion_is_disabled == 1) ):
            return

        widget = self.vtk_vtk.getRenWidget()
    
        zoom_factor = 1.01 ** (y_coord - self.lastY)
        if self.vtk_data.is2DMode() :
            scale = self.current_camera.GetParallelScale()
            self.current_camera.SetParallelScale( scale / zoom_factor )
            self.setParallelScale( scale / zoom_factor )
        else:
            clipping_range = self.current_camera.GetClippingRange()
            min_range  = clipping_range[0]
            max_range = clipping_range[1]
            self.current_camera.SetClippingRange( min_range / zoom_factor, 
                                               max_range / zoom_factor )
            self.current_camera.Dolly( zoom_factor )
    
        self.lastX = x_coord
        self.lastY = y_coord
    
        self._render()
    
        decls.memberFunctionAudit(self)

        # Hack!  Piercing all kinds of encapsulation with this...
        self.network_api.sendCommand( 'c.vol_tk_interactor._zoom('
            + str(x_coord) + ',' + str(y_coord) + ')' )


    def setClippingRange( self, r, suppress_callbacks=0 ):
        anag_utils.funcTrace()
        self.clipping_range.set( r, suppress_callbacks=suppress_callbacks )
        self.current_camera.SetClippingRange( r[0], r[1] )
        self._render()
