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

# File: vtk_axes.py
# Author: TDSternberg
# Created: 10/22/01

import anag_utils
from self_control import SelfControl

import vtkpython

class VtkAxes( SelfControl ):
    """
    Displays the x, y and z axes, for help in orienting oneself.
    """

    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        SelfControl.__init__( self,dep_dict, metadata=
          [ {'name':'axes_actor'},
            {'name':'text_actors', 'initval':{}},
            {'name':'is_visible', 'get':1}, # See below for default
          ] )
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls","self" )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        if self.cmd_line.getUseRenderWidget() == 0:
            return
        if self.cmd_line.getNoVtk():
            return

        if self.vtk_data.is2DMode():
            self.is_visible=0
        else:
            self.is_visible=1

        axes_source = vtkpython.vtkAxes()
        axes_source.SetOrigin( 0,0,0 )

        tubes = vtkpython.vtkTubeFilter()
        tubes.SetInput( axes_source.GetOutput() )
        tubes.SetRadius( axes_source.GetScaleFactor()/10.0 )
        tubes.SetNumberOfSides( 6 )

        axes_mapper = self.vtk_vtk.newMapper()
        axes_mapper.SetInput( tubes.GetOutput() )
        axes_mapper.ImmediateModeRenderingOn()

        self.axes_actor = self.vtk_vtk.newActor()
        self.axes_actor.SetMapper( axes_mapper )
        self.axes_actor.PickableOff()
        self.axes_actor.DragableOff()
        self.axes_actor.GetProperty().SetAmbient( 0.3 )
        self.axes_actor.GetProperty().SetDiffuse( 0.7 )

        #
        # Labels (x,y,z)
        #
        text_mappers = {}
        text_sources = {}

        axis_colors = {'x':(1,0,0), 'y':(1,1,0), 'z':(0,1,0)}
        axis_label_positions = {'x':(  0.8, -0.5, -0.5 ),
                                'y':( -0.5,  1.0, -0.5 ),
                                'z':( -0.5, -0.5,  0.8 ) }
        axis_label_origins = {'x':(  0.6,  0.5,  0.5 ),
                              'y':(  0.5,  0.6,  0.5 ),
                              'z':(  0.5,  0.5,  0.6 ) }
            
        for axis in 'x','y','z':
            text_sources[axis] = vtkpython.vtkVectorText()
            text_sources[axis].SetText( axis )

            text_mappers[axis] = self.vtk_vtk.newMapper()
            text_mappers[axis].SetInput( text_sources[axis].GetOutput() )
            text_mappers[axis].ImmediateModeRenderingOn()

            self.text_actors[axis] = vtkpython.vtkFollower()
            self.text_actors[axis].SetMapper( text_mappers[axis] )
            self.text_actors[axis].SetCamera( self.vtk_cameras.getAxesCamera() )
            self.text_actors[axis].SetScale( 0.6, 0.6, 0.6 )
            self.text_actors[axis].SetOrigin( axis_label_origins[axis] )
            apply( self.text_actors[axis].GetProperty().SetColor,
                   axis_colors[axis] )
            self.text_actors[axis].GetProperty().SetAmbient( 0.3 )
            self.text_actors[axis].GetProperty().SetDiffuse( 0.7 )
            self.text_actors[axis].SetPosition( axis_label_positions[axis] )

        if self.cmd_line.getUseRenderWidget() == 1:
            self.vtk_vtk.getAxesRenderer().SetViewport( 0.0, 0.0, 1.0, 1.0 )

        #
        # Add actors.
        #
        self.vtk_vtk.addAxesActor( self.axes_actor )
        for axis in 'x','y','z':
            self.vtk_vtk.addAxesActor( self.text_actors[axis] )


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.cmd_line.getNoVtk():
            return


    def cleanup( self ):
        anag_utils.funcTrace()
