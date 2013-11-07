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

# File: vtk_cameras.py
# Author: TDSternberg
# Created: 8/23/01

import anag_utils
from self_control import SelfControl

class VtkCameras( SelfControl ):
    """
    A lot of camera control takes place in vol_tk_interactor, so one might ask
    about the intended division of labor between that module and this one.  The
    answer is that here we only initialize the cameras, while all manipulation
    (other than through control_cameras' GUI) takes place in vol_tk_interactor.

    This state of affairs is not ideal; it reflects the fact that at one time
    there was no vtk_cameras or control_cameras, only vol_tk_interactor.
    Control_cameras came to be when we introduced the camera parameters GUI
    (that lets the user enter his own choices for the clipping planes into a
    text entry widget).  Vtk_cameras came soon after; every control_ module
    is supposed to have its corresponding vtk_ module after all.

    A better design, which we should move to eventually, would locate all
    VTK-level camera control in vtk_cameras, with vol_tk_interactor left to
    call vtk_camera methods, when it needs to set camera parameters as necessary
    following zoom, pan and rotate operations.
    """
    def __init__(self, dep_dict):
        SelfControl.__init__( self, dep_dict,
            [
                {'name':'camera', 'get':1 },
                {'name':'light', 'get':1 },
                {'name':'axes_camera', 'get':1 },
                {'name':'axes_light', 'get':1 }
            ] )
        self.decls = anag_utils.Declarations( "decls", instance=self  )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "self", "decls", "is_new_hdf5",
            "lights", "range_", "minrange", "maxrange", "mid"
            )

        if self.cmd_line.getNoVtk():
            return

        self.vtk_vtk.getRenWin().Render() # Gotta call this, or else light==None
        self.camera = self.vtk_vtk.getRenderer().GetActiveCamera()

        lights = self.vtk_vtk.getRenderer().GetLights()
        lights.InitTraversal()
        self.light = lights.GetNextItem()
        self.camera.SetPosition( 0, 0, 1000 )
        self.camera.SetViewUp( 0, 1, 0 )
        self.camera.SetFocalPoint( 0, 0, 0 )
        self.camera.ComputeViewPlaneNormal()
        self.vtk_vtk.getRenderer().ResetCamera()

        range_ = self.camera.GetClippingRange()
        if( self.vtk_data.is2DMode() ):
            self.camera.ParallelProjectionOn()
            self.camera.SetParallelScale( 0.18
                                          * ( range_[0] * range_[1] ) ** 0.5 )
        else:
            self.camera.Elevation( 20.0 )
            self.camera.Azimuth( -10.0 )
            self.camera.Dolly( 1.2 )

        if( (not self.vtk_data.is2DMode())
        and (self.cmd_line.getUseRenderWidget() == 1)):
            self._initAxes()

        # Broaden the clipping range some, from what VTK wants to set it to.
        # Gotta do this here, after the call to _initAxes(), since that thing
        # calls ResetCamera().
        self.camera.SetClippingRange( range_[0]/4.0, range_[1]*4.0 )

        decls.memberFunctionAudit(self)


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.cmd_line.getNoVtk():
            return
 

    def cleanup( self ):
        anag_utils.funcTrace()


    def _initAxes( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'self', 'decls', 'lights', 'range_',
            'camera_focus')

        self.vtk_vtk.getAxesRenWin().Render()

        self.axes_camera = self.vtk_vtk.getAxesRenderer().GetActiveCamera()
        lights = self.vtk_vtk.getAxesRenderer().GetLights()
        lights.InitTraversal()

        self.axes_light = lights.GetNextItem()

        camera_focus = self.axes_camera.GetFocalPoint()
        self.axes_camera.SetFocalPoint( camera_focus )
        self.axes_camera.SetPosition( camera_focus[0], camera_focus[1], 1000 )
        self.axes_camera.SetViewUp( 0, 1, 0 )
        self.axes_camera.ComputeViewPlaneNormal()
        self.vtk_vtk.getRenderer().ResetCamera()
        
        self.axes_camera.Elevation( 20.0 )
        self.axes_camera.Azimuth( -10.0 )
        self.axes_camera.Dolly( 109 )
        
        self.axes_light.SetPosition( self.axes_camera.GetPosition() )
        self.axes_light.SetFocalPoint( self.axes_camera.GetFocalPoint() )

        self.axes_camera.SetClippingRange( 0.1, 20 )

        decls.memberFunctionAudit(self)


    def setLightIntensity( self, x ):
        assert(0<=x<=1)
        self.light.SetIntensity(x)
