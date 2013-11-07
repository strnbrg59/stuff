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

# File: vtk_vol.py
# Author: TDSternberg
# Created: 6/11/01

import anag_utils
from self_control import SelfControl

import vtkpython
import libVTKChomboPython

class VtkVol( SelfControl ):
    
    def __init__(self, dep_dict):
        anag_utils.funcTrace()


        #
        # Variable declarations
        #
        instance_data = [
            { 'name':'amr_volume' },
            { 'name':'xfer_fun' },

            { 'name':'xfer_points', 'save':1, 'get':1, 'set':2,
              'initval':[(30,1.0), (100,0.5), (230,0.0)]},
            { 'name':'is_visible', 'initval':0, 'save':1, 'get':1, 'set':2 },
            { 'name':'update_disabled', 'initval':0 },
            { 'name':'component', 'save':1, 'get':1, 'set':2 },  # string
            { 'name':'max_visible_level', 'save':1, 'initval':0, 'get':1,
              'set':2 },
            { 'name':'min_visible_level', 'save':1, 'initval':0, 'get':1,
              'set':2 },
            { 'name':'integration_step_factor', 'save':1, 'initval':1.0,
              'get':1, 'set':2 },
            { 'name':'local_vtk_data', 'get':1 }
        ]
        SelfControl.__init__( self,dep_dict, instance_data)
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( 'decls', 'dep_dict', 'instance_data' )

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls",
            "white_tfun",
            "color_xfer_function",
            "vol_prop",
            "composite_func"
            )

        if self.vtk_data.is2DMode():
            return

        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=0,
            follow_global_curcomponent=0,
            follow_global_padded_boxes=0,
            always_use_real_boxes=1,
            instance_identifier='volume' )

        white_tfun = vtkpython.vtkPiecewiseFunction()
        white_tfun.AddPoint( 0, 1 )
        white_tfun.AddPoint( 255, 1 )
        
        self.xfer_fun = vtkpython.vtkPiecewiseFunction()

        # color_xfer_function isn't used, yet.  The way we really want to use
        # it is not to have the user specify the color_xfer_function point by
        # point, but rather to just let some colormap (in a file) define it.
        color_xfer_function = vtkpython.vtkColorTransferFunction()
        color_xfer_function.AddRGBPoint( 0.0, 0.0, 0.0, 0.0 )
        color_xfer_function.AddRGBPoint( 64.0, 1.0, 0.0, 0.0 )
        color_xfer_function.AddRGBPoint( 128.0, 0.0, 1.0, 0.0 )
        color_xfer_function.AddRGBPoint( 192.0, 0.0, 0.0, 1.0 )
        color_xfer_function.AddRGBPoint( 255.0, 0.0, 0.0, 0.2 )
    
        vol_prop = vtkpython.vtkVolumeProperty()
        vol_prop.SetColor( white_tfun )
#       vol_prop.SetColor( color_xfer_function )
        vol_prop.SetScalarOpacity( self.xfer_fun )
        vol_prop.SetInterpolationTypeToLinear()
        vol_prop.ShadeOn()
    
        composite_func = vtkpython.vtkVolumeRayCastCompositeFunction()
    
        self.amr_volume = libVTKChomboPython.vtkAMRVolume()
        self.amr_volume.SetChomboReader( self.local_vtk_data.getReader() )
        self.amr_volume.SetRayCastFunction( composite_func )
        self.amr_volume.SetRenderer( self.vtk_vtk.getRenderer() )
        self.amr_volume.SetProperty( vol_prop )

        self.component = self.local_vtk_data.getCurComponent()

        decls.memberFunctionAudit(self)


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.vtk_data.is2DMode():
            return

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        self.update_disabled = 1
        self.min_visible_level =\
             min( max( self.local_vtk_data.getMinAvailableLevel(),
                       self.min_visible_level ),
                  self.local_vtk_data.getMaxAvailableLevel() )
        self.max_visible_level =\
            max( min( self.local_vtk_data.getMaxAvailableLevel(),
                      self.max_visible_level ),
                 self.local_vtk_data.getMinAvailableLevel() )

        #timer = anag_utils.Timer()
        #timer.on()

        self.setMaxVisibleLevel( self.max_visible_level, force=1 )
        self.setMinVisibleLevel( self.min_visible_level, force=1 )
        if self.component:
            self.setComponent( self.component, force=1 )
        self.setXferPoints( self.xfer_points )
        self.update_disabled = 0
        self.setIsVisible( self.is_visible )

        self.registerCallback(
            self.local_vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicCallback )
    
        #timer.stop( msg='Time in vtk_vol._refresh(): ' )


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def cleanup( self ):
        anag_utils.funcTrace()
        if not self.vtk_data.is2DMode():
            self.setIsVisible( 0 )


    def _anisotropicCallback( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._updateVis()


    def setComponent( self, name, force=0 ):
        """ Component to volume-render, specified by its string name. """
        anag_utils.funcTrace()
        if (name != self.component) or (force == 1):
            self.component = name
            component_num = self.local_vtk_data.getComponentSerialNum( name )
            self.amr_volume.SetComponentNum( component_num )
            self._updateVis()


    def setMaxVisibleLevel( self, k, force=0 ):
        """
        Set the maximum level whose data we want to represent volumetrically.
        """
        anag_utils.funcTrace()
        if (k != self.max_visible_level) or (force == 1):
            self.max_visible_level = k
            if k < self.min_visible_level:
                self.min_visible_level = k
                self.amr_volume.SetMinLevel( k )
            self.amr_volume.SetMaxLevel( k )
            self._updateVis()

    def setMinVisibleLevel( self, k, force=0 ):
        """
        Set the minimum level whose data we want to represent volumetrically.
        """
        anag_utils.funcTrace()
        if (k != self.min_visible_level) or (force == 1):
            self.min_visible_level = k
            if k > self.max_visible_level:
                self.max_visible_level = k
                self.amr_volume.SetMaxLevel( k )
            self.amr_volume.SetMinLevel( k )

            self._updateVis()
        
    
    def setIsVisible( self, on_off ):
        anag_utils.funcTrace()
        assert( on_off == 0  or  on_off == 1 )
        anag_utils.funcTrace()

        self.is_visible = on_off
        self._updateVis()


    def setXferPoints( self, xfer_points ):
        """
        Arg xfer_points is a list of pairs.  Each pair specifies a point on
        domain [0,255], range [0.0,1.0].  The points define a step function
        that defines opacity.  The step function should decrease monotonically.
        """
        anag_utils.funcTrace()

        self.xfer_fun.RemoveAllPoints()
        for p in xfer_points:
            self.xfer_fun.AddPoint( p[0], p[1] )

        if xfer_points != self.xfer_points:
            self.xfer_points = []
            for p in xfer_points:
                self.xfer_points.append( p[:] )

        self._updateVis()


    def setIntegrationStepFactor( self, x ):
        """
        Factor applied to arg for amr_volume.SetSampleDistance().
        """
        anag_utils.funcTrace()
        if x != self.integration_step_factor:
            self.integration_step_factor = x
        self._updateVis()


    def _updateVis( self ):
        anag_utils.funcTrace()
        if self.update_disabled == 1:
            return
        if self.is_visible == 0:
            self.amr_volume.RemoveVolumesFromRenderer()
        else:
            dx = self.local_vtk_data.getLevelDx( self.max_visible_level )
            self.amr_volume.SetSampleDistance( min(dx)
                                             * self.integration_step_factor)
            self.xfer_fun.RemoveAllPoints()
            for p in self.xfer_points: self.xfer_fun.AddPoint( p[0], p[1] )
            self.amr_volume.BuildVolumes()
            self.amr_volume.AddVolumesToRenderer()
        self.vtk_vtk.render()
