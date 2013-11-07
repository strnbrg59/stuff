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

# File: vtk_vtk.py
# Author: TDSternberg
# Created: 8/20/01

"""
Owns the core vtk objects -- the renderers, the renderwidget, the renderwindow.
Also owns the top-level windows.
"""

import os
import sys
import Tkinter
import vtk
import vtkRenderWidget
import release_number
import libVTKChomboPython

import anag_utils
from self_control import SelfControl

class VtkVtk( SelfControl ):
    """
    Owns the core vtk objects -- the renderers, the renderwidget, the
    renderwindow.
    Also owns the top-level windows.

    Depends on nothing but self_control.
    """
    def __init__(self, dep_dict):
        instance_data = [
            { 'name':'top_window', 'get':1 },
            { 'name':'menu_frame', 'get':1 }, # For gui
            { 'name':'axes_frame', 'get':1 },
            { 'name':'renderer', 'get':1 },
            { 'name':'axes_renderer', 'get':1 },
            { 'name':'autorender', 'initval':1, 'get':1, 'set':2, 'save':1},
            { 'name':'ren_widget', 'get':1 },
            { 'name':'axes_ren_widget', 'get':1 },
            { 'name':'ren_win',    'get':1 },
            { 'name':'ren_widget_size', 'get':1, 'set':2, 'initval':(640,640),
                'save':2 },
            { 'name':'ren_win_size_restricted_to_multiples_of_16',
                'get':1, 'set':1, 'initval':1, 'save':2 },
            { 'name':'axes_ren_win', 'get':1 },
            { 'name':'actor_list', 'initval':[] },

            { 'name':'new_HDF5_notifier', 'notify':1 }
                # See ChomboVis.vtkvtkNewHDF5NotifierHandler()
        ]
        SelfControl.__init__( self, dep_dict, instance_data )
        self.decls = anag_utils.Declarations( 'decls',
            instance=self
            )

        if self.cmd_line.getOffScreen() == 1:
            self._initTopWindowsForOffscreen()
        else:
            self._initTopWindows()


    def _initTopWindowsForOffscreen(self):
        """
        Set up the top-level widgets for off-screen mode.  No menus, no
        Tkinter.Tk(), etc.
        """
        anag_utils.funcTrace()

        self.ren_win = self.newRenderWindow()
        self.ren_win.OffScreenRenderingOn()
        self.renderer = self.newRenderer()
        self.ren_win.AddRenderer( self.renderer )

        self.axes_ren_win = self.newRenderWindow()
        self.axes_renderer = self.newRenderer()
        self.axes_ren_win.AddRenderer( self.axes_renderer )

        self._refresh()


    def setTopWindowTitle( self, str ):
        anag_utils.funcTrace()
        self.top_window.wm_title( str )
        

    def _initTopWindows(self):
        """ Set up the top-level widgets, including vtkRenderWidget """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls' )

        self.top_window = Tkinter.Tk()
        if self.cmd_line.getNoVtk():
            self.top_window.withdraw()
            return

        self.setTopWindowTitle( "ChomboVis " + release_number.the_number )
        self.top_window.bind( '<Destroy>', self._kill )
        #self.top_window.bind( '<Key-q>', lambda e : sys.exit(2) )
    
        # Now this is strange.
        # We have a file vtkTkRenderWidget in /usr/local/lib,
        # but what we have to import is vtkRenderWidget, which in turn seems to
        # have under it an object named vtkTkRenderWidget.
        # In any case, constructing an object of that type results in a
        # core dump.
        # There's some discussion of this in the Vtk mailing archive.
    
        if self.cmd_line.getUseRenderWidget() == 1:
            self.ren_widget = vtkRenderWidget.vtkTkRenderWidget(self.top_window)
            # Core dump here?  Try export VTK_TK_WIDGET_PATH=$VTK_HOME/python
            # and make sure you configured vtk "--with-shared".
            # Or put vtkTkRenderWidget and vtkRenderWidget.py into /usr/lib.
            self.ren_win = self.ren_widget.GetRenderWindow()
        else:
            self.ren_win = self.newRenderWindow()
        self.renderer = self.newRenderer()

#       This was an experiment.  It doesn't work as one would expect.
#       self.renderer.SetLightFollowCamera(0)
#       self.renderer.SetTwoSidedLighting(0)

        self.ren_win.AddRenderer( self.renderer )

        # Leave room for the menubar.  We're not nearly ready to initialize
        # the menubar at this point, though.    
        self.menu_frame = Tkinter.Frame( self.top_window )
        self.menu_frame.pack(fill=Tkinter.X)

        if self.cmd_line.getUseRenderWidget() == 1:
            self.ren_widget.pack( expand=1, fill=Tkinter.BOTH )
            # Deal with the little axes display.
            self._initAxes()
    
        self._refresh()

        decls.memberFunctionAudit(self)


    def _initAxes( self ):
        """
        Called from _initTopWindows(), to handle setup and rendering of the
        little axes display.
        """
        anag_utils.funcTrace()

        self.axes_frame = Tkinter.Frame( self.top_window,
                                         relief='groove', borderwidth=1 )
        if self.cmd_line.getUseRenderWidget() == 1:
            self.axes_ren_widget = vtkRenderWidget.vtkTkRenderWidget(
                                            self.axes_frame)
            self.axes_ren_win = self.axes_ren_widget.GetRenderWindow()
            self.axes_ren_widget.pack( expand=1, fill=Tkinter.BOTH )
        else:
            self.axes_ren_win = self.newRenderWindow()
        self.axes_renderer = self.newRenderer()
        self.axes_ren_win.AddRenderer( self.axes_renderer )
        self.axes_frame.pack( side='left' )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()


    def refresh( self ):
        """ 
        Called from ChomboVis. Don't want to call _refresh() from outside this
        module.
        """
        anag_utils.funcTrace()
        self._refresh()
        self.registerForRestoration() # Defined in SelfControl


    def _refresh( self ):
        """
        Overrides SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()

        if not self.cmd_line.getNoVtk():
            if self.cmd_line.getUseRenderWidget() == 1:
                self.ren_widget.configure( width  = self.getRenWidgetSize()[0],
                                           height = self.getRenWidgetSize()[1] )
                self.axes_ren_widget.configure( width = 80, height = 80 )
            else:
                self.ren_win.SetSize( self.ren_widget_size[0],
                                      self.ren_widget_size[1] )

        self.registerCallback(
            self.saved_states.getNotifierAutorenderOverride(), self.render )


    def preStateSaveHook( self ):
        """
        Overrides SelfControl.preStateSaveHook().
        """
        anag_utils.funcTrace()
        self.ren_widget_size = self.ren_win.GetSize()
        # That saves the size if the user stretched the window with his mouse.

        if self.ren_win_size_restricted_to_multiples_of_16 == 1:
            width  = 16 * (self.ren_widget_size[0]/16)
            height = 16 * (self.ren_widget_size[1]/16)
            self.ren_widget_size = (width,height)


    def setDependency( self, class_name, class_instance ):
        """
        This class depends on some others -- those above it in
        Chombovis.init_data.  Here, we assign those.  This function is called
        when loading a new hdf5 file (an old one having just been discarded).
        Loading a new hdf5 file means re-constructing all classes except VtkVtk,
        but even in VtkVtk we need to reset references to stale dependencies.
        """
        anag_utils.funcTrace()
        self.__dict__[class_name] = class_instance


    def cleanup( self ):
        """
        Overrides SelfControl.cleanup().  Unlike other classes, here we want to
        keep a lot of the stuff around, for use with the next hdf5 file.
        """
        anag_utils.funcTrace()


    def zeroSelfVariables( self ):
        """
        Overrides version in SelfControl.  We emphatically do not want to blow
        away our self.* variables in this class.
        """
        anag_utils.funcTrace()


    def setRenWidgetSize( self, width, height ):
        """
        Rounds width and height to multiples of 16 as
        self.ren_win_size_restricted_to_multiples_of_16 is 1 or 0.  This is
        because some movie players don't like frames of other sizes.
        """
        anag_utils.funcTrace()

        if self.ren_win_size_restricted_to_multiples_of_16 == 1:
            width  = 16 * (width/16)
            height = 16 * (height/16)


        if self.cmd_line.getOffScreen() == 1:
            self.ren_win.SetSize( width, height )
        else:
            if self.cmd_line.getUseRenderWidget() == 1:
                self.ren_widget.configure( width=width, height=height )
            else:
                self.ren_win.SetSize( width, height )
        self.ren_widget_size = (width,height)



    def forgetAxesFrame( self ):
        """
        We don't want to display axes if we're in 2D mode.  Unfortunately, 
        vtk_vtk doesn't know if we're in 2D mode.  So this function is for
        a higher-level module to call.
        """
        anag_utils.funcTrace()
        if( (not self.cmd_line.getNoVtk())
        and (not self.cmd_line.getOffScreen())
        and (self.cmd_line.getUseRenderWidget()==1)):
            self.axes_frame.forget()


    def _kill( self, x ):
        """
        Attempt to enable graceful exit when user closes the X window.  So far,
        I can't get it to avoid the ugly warning about destroying a
        TkRenderWidget before its associated vtkRenderWindow is destroyed.
        Nor does it work to have this function just "return".

        The worst thing that can happen by not exiting gracefully is
        that the saved states file doesn't get flushed and, very likely, gets
        corrupted.  See below for why I can't even fix that.
        """
        anag_utils.funcTrace()
        return
        # WARNING: If you say "exit()" here, you'll exit even when file-
        # selection dialogs (like the ones for picking a new colormap or a
        # file to save a state to) close.  Bad scene.

        # Here are some unsuccessful strategies I've tried...

        # This would protect the saved-states file from corruption.
        # Unfortunately, the <Destroy> event (which gets us here) also occurs
        # when we load a new colormap (!?)
        self.saved_states.flushPickles()
        sys.exit(1)

        # This never closes:
        #tkMessageBox.showerror( title='Bad operation',
        #    message='The preferred way to exit the application is through ' \
        #           +'the File->Exit menu option.' )

        # This Delete() is not recognized as an attribute of self.ren_win,
        # despite appearances from the vtk documentation and indeed the vtk
        # source code.
        # self.ren_win.Delete()

        # These two result in lots of bad things (like negative reference
        # counts) that end with a segfault.
        self.ren_win.UnRegister( 0 )
        sys.exit(0)


    def render( self, notifier=None, extra_info=None ):
        """
        Cause the VTK RenderWindow to render itself, if self.autorender==1
        and if also we haven't shut it off (AutorenderOverride()) for purposes
        of expediting a state-restore.

        This function is called many times, but only once with its optional
        arguments.
        """
        anag_utils.funcTrace()

        if self.cmd_line.getNoVtk():
            return

        if( self.getAutorender() == 1
        and self.saved_states.getAutorenderOverride() == 1 ):
            self.ren_win.Render()
            if( (self.cmd_line.getOffScreen() != 1)
            and (self.cmd_line.getUseRenderWidget() ==1) ):
                self.axes_ren_win.Render()


    def setAutorender( self, on_off ):
        """
        With autorender off, modifications to the positions of slices, the
        value of an isosurface, etc aren't immediately rendered.
        
        However, they are rendered if we move the mouse from the renderwindow
        to the menubar and back, or click anywhere in the renderwindow.
        That apparently trips the <Expose> event in vol_tk_interactor.
        But we can't disable vol_tk_interactor's handling of <Expose> because
        then we get all kinds of ghosts left on the window.
        """

        anag_utils.funcTrace()
        assert( on_off == 0 or on_off == 1 )

        self.autorender = on_off
        if on_off == 1:
            self.render()


    def addActor( self, actor, renderer=None ):
        """
        Add this actor to the vtkRenderer, if this actor isn't already there.
        If it's already there that's cool; it doesn't indicate any kind of
        programming error.  The dual level of control -- vtk_* and control_* --
        over many of the actors creates situations where we do need to avoid
        adding the same actor twice (or else, when we try to turn it off, we end
        up removing just one of them and we don't see any effect).  We could
        keep track of that with a whole set of auxiliary "prev_*" variables.
        Or we can just check here that the actor isn't already there.
    
        First check that this actor isn't already there.
        """
        anag_utils.funcTrace()

        if self.cmd_line.getNoVtk():
            return

        if not renderer:
            renderer = self.getRenderer()
        if not renderer.GetActors().IsItemPresent( actor ):
            renderer.AddActor( actor )
            self.actor_list.append( actor )
    

    def addAxesActor( self, actor ):
        """
        Like addActor(), but works on the special renderer for the little axes.
        """
        anag_utils.funcTrace()
        self.addActor( actor, renderer = self.getAxesRenderer() )

    
    def removeActor( self, actor, renderer=None, force=0 ):
        """
        First check that this actor is there at all.
    
        About the check that the actor is there, see my comments under
        addActor().  But we probably don't even need to check for that here;
        if the actor isn't there, RemoveActor() seems to just do nothing.

        Arg force is necessary for vtk_cmap.bar_actor because IsItemPresent()
        doesn't seem to find it even if it's there.
        """
        anag_utils.funcTrace()
        if (not actor) or (self.cmd_line.getNoVtk()==1):
            return
        if not renderer:
            renderer = self.getRenderer()
        if actor in self.actor_list or force==1:
            renderer.RemoveActor( actor )
            if actor in self.actor_list:
                self.actor_list.remove( actor )


    def removeAxesActor( self, actor ):
        """
        Like removeActor(), but works on the special renderer for the little
        axes.
        """        
        anag_utils.funcTrace()
        self.removeActor( actor, renderer = self.getAxesRenderer() )

#
#   Factories for vtk[Mesa]Actor, vtk[Mesa]Renderer, vtk[Mesa]RenderWindow.
#   Because we want to support offscreen rendering (which OpenGL does not, but
#   Mesa does, support) we need to be able to run with either the straight
#   OpenGL objects, or their Mesa specializations.
#
    def newActor( self ):
        """
        Returns a vtkActor or a vtkMesaActor, as appropriate.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getMesa() == 1:
            return vtk.vtkMesaActor()
        else:
            return vtk.vtkActor()

    def newProperty( self ):
        """
        Returns a vtkProperty or a vtkMesaProperty, as appropriate.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getMesa() == 1:
            return vtk.vtkMesaProperty()
        else:
            return vtk.vtkProperty()

    def newMapper( self ):
        """
        Returns a vtkPolyDataMapper or a vtkMesaPolyDataMapper, as appropriate.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getMesa() == 1:
            return vtk.vtkMesaPolyDataMapper()
        else:
            return vtk.vtkPolyDataMapper()

    def newRenderWindow( self ):
        """
        Returns a vtkRenderWindow or a vtkMesaRenderWindow, as appropriate.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getMesa() == 1:
            return vtk.vtkXMesaRenderWindow()
        else:
            return vtk.vtkRenderWindow()

    def newRenderer( self ):
        """
        Returns a vtkRenderer or a vtkMesaRenderer, as appropriate.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getMesa() == 1:
            return vtk.vtkMesaRenderer()
        else:
            return vtk.vtkRenderer()

    def newTexture( self ):
        """
        Returns a vtkTexture or a vtkMesaTexture, as appropriate.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getMesa() == 1:
            return vtk.vtkMesaTexture()
        else:
            return vtk.vtkTexture()
        

    def unitTest( self ):
        pass
