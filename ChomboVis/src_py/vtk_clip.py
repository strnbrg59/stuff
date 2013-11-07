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

# File: vtk_clip.py
# Author: TDSternberg
# Created: 9/26/02

import anag_utils
from self_control import SelfControl

import libVTKChomboPython

import vtkpython

import math

g_decls = anag_utils.Declarations( 'g_decls', 'g_clip_modes' )

class VtkClip( SelfControl ):
    
    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        instance_data = [
            {'name':'inside_out_states', 'get':1, 'initval':{} },
            {'name':'inside_out_indicator', 'get':1, 'set':2, 'initval':0,
                'save':1 },
            {'name':'plane'},
            {'name':'clippers', 'initval':{}},
            {'name':'direction', 'get':1, 'initval':'x', 'save':1},
            {'name':'position', 'get':1, 'save':1, 'initval':0},
            {'name':'angle0', 'get':1, 'initval':0, 'save':1},
            {'name':'angle1', 'get':1, 'initval':0, 'save':1},
            {'name':'visible_plane_is_visible', 'initval':0, 'get':1, 'set':2,
                'save':1},
            {'name':'visible_plane'},
            {'name':'visible_plane_contour'},
            {'name':'visible_plane_mapper'},
            {'name':'visible_plane_property' },
            {'name':'visible_plane_actor' }
        ]
        SelfControl.__init__( self,dep_dict, instance_data)
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'instance_data', 'dep_dict' )
        anag_utils.funcTrace()

        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace(0)
        self.position = (  self.vtk_data.getDomainMin()[self.direction]
                         + self.vtk_data.getDomainMax()[self.direction] )/3.0

    def _refresh( self ):
        anag_utils.funcTrace(0)
        self.defineClipPlane( direction=self.direction,
                              position=self.position,
                              angle0=self.angle0,
                              angle1=self.angle1 )
        self.setVisiblePlaneIsVisible( self.visible_plane_is_visible )
        self.setInsideOutIndicator( self.inside_out_indicator )


    def cleanup( self ):
        anag_utils.funcTrace()


    def clipPipeline( self, pre_clipper, post_clipper,
                      clip_against_isocontour=False,
                      isocontour_clip_value=None,
                      inside_out=0 ):
        """
        Args pre_clipper and post_clipper (typically an accumulator and a
        mapper) are elements of some pipeline.  Here we interpose a
        vtkClipPolyData(), whose clip function is a plane defined by the
        current state of self.direction, self.position, self.angle0 and
        self.angle1.

        Arg inside_out is specific to the new vtkClipPolyData we create in this
        function.  The actual inside-out state we request from VTK is the xor
        of this argument and self.inside_out_indicator.
        We store an inside_out state for each vtkClipPolyData in
        self.inside_out.  They can all be flipped with setInsideOutIndicator().
        """
        anag_utils.funcTrace()

        if self.vtk_data.is2DMode() and (not clip_against_isocontour):
            return

        # We maintain a separate vtkClipPolyData for each pipeline.  And the
        # pre_clipper is simply the most convenient way to uniquely identify
        # the pipelines.
        self.clippers[pre_clipper] = vtkpython.vtkClipPolyData()
        self.clippers[pre_clipper].SetInput( pre_clipper.GetOutput() )
        self.clippers[pre_clipper].SetInsideOut(  inside_out
                                                ^ self.inside_out_indicator )
        self.inside_out_states[pre_clipper] = inside_out \
                                            ^ self.inside_out_indicator

        if not clip_against_isocontour:
            self.clippers[pre_clipper].SetClipFunction( self.plane )
            self.clippers[pre_clipper].SetValue( 0.0 )
        else:
            assert( isocontour_clip_value != None )
            self.clippers[pre_clipper].SetValue( isocontour_clip_value )

        if( (not clip_against_isocontour)
        or  (pre_clipper.GetOutput().GetPointData().GetScalars()) ):
            post_clipper.SetInput(self.clippers[pre_clipper].GetOutput())
            self.vtk_vtk.render()
        # FIXME: strange control structure here


    def unclipPipeline( self, pre_clipper, post_clipper,
                        clip_against_isocontour=False):
        """
        Undo the effect of self.clipPipeline().
        """
        anag_utils.funcTrace()
        if self.vtk_data.is2DMode() and clip_against_isocontour==False:
            return

        post_clipper.SetInput( pre_clipper.GetOutput() )
        self.vtk_vtk.render()


    def setInsideOutIndicator( self, on_off ):
        """
        Individual pipelines' inside-out states, from the point of view of VTK,
        can be different from one another.  The role of inside_out_indicator is
        to let us flip all the inside-out states.
        """
        anag_utils.funcTrace()
        assert( on_off==0  or  on_off==1 )
        if on_off != self.inside_out_indicator:
            for k in self.clippers.keys():
                self.inside_out_states[k] = int(not self.inside_out_states[k])
                self.clippers[k].SetInsideOut( self.inside_out_states[k] )
            self.vtk_vtk.render()
            self.inside_out_indicator = on_off


    def defineClipPlane( self, direction=None, position=None,
                               angle0=None, angle1=None ):
        """
        Define a clipping plane that will set the boundary
        between a region we will render, and a region we won't render.

        Args direction, position, angle0 and angle1 determined the plane's
        position and orientation.  We start with the plane perpendicular to
        arg direction ('x','y', or 'z') at arg position along that axis.  Then
        we rotate the plane, by angle0 and angle1 degrees, around the other two
        axes.  These axes are the horizontal and vertical axes, respectively,
        when the axis indicated by arg direction is pointing out of the screen
        in a right-hand coordinate system, i.e.:
               direction     angle0-axis    angle1-axis
                 x             y              z
                 y             z              x
                 z             x              y

        If an arg is None, then we use its self.* counterpart.
        """
        anag_utils.funcTrace()

        if direction!=None:
            self.direction = direction
        if position!=None:
            self.position = position
        if angle0!=None:
            self.angle0 = angle0
        if angle1!=None:
            self.angle1 = angle1
        direction=self.direction
        position=self.position
        angle0=self.angle0
        angle1=self.angle1

        # Convert from (direction,position,angle0,angle1) coordinates, to
        # (origin,normal) coordinates.
        plane_origin = {'x':(position,0,0),
                        'y':(0,position,0),
                        'z':(0,0,position)}[ direction ]
        rad_deg = math.pi/180

        # Add offsets to the angles, to achieve correct orientation when
        # direction is not 'z'.
        if direction == 'x':
            angle0 += 180
            angle1 += 270
        if direction == 'y':
            angle0 += 90 # Still not quite right.


        normal_components = ( math.sin(angle0*rad_deg),
                              math.cos(angle0*rad_deg)*math.sin(angle1*rad_deg),
                              math.cos(angle0*rad_deg)*math.cos(angle1*rad_deg))
        normal_picker = { 'x':(1,2,0), 'y':(2,0,1), 'z':(0,1,2) }[direction]
        plane_normal = (normal_components[normal_picker[0]],
                        normal_components[normal_picker[1]],
                        normal_components[normal_picker[2]])

        if not self.plane:
            self.plane = vtkpython.vtkPlane()

            # The rest is for rendering an image of the clipping plane
            self.visible_plane = vtkpython.vtkSampleFunction()
            self.visible_plane.SetImplicitFunction( self.plane )
            extents = self.vtk_data.getDomainExtentsXYZ()
            self.visible_plane.SetModelBounds( extents[0], extents[3],
                                         extents[1], extents[4],
                                         extents[2], extents[5] )
            self.visible_plane.SetSampleDimensions(2,2,2)
            self.visible_plane.ComputeNormalsOff()
        
            self.visible_plane_contour = vtkpython.vtkContourFilter()
            self.visible_plane_contour.SetInput( self.visible_plane.GetOutput() )
            self.visible_plane_contour.SetValue( 0, 0.0 )

            self.visible_plane_mapper = self.vtk_vtk.newMapper()
            self.visible_plane_mapper.SetInput(
                self.visible_plane_contour.GetOutput() )
            self.visible_plane_actor = self.vtk_vtk.newActor()
            self.visible_plane_actor.SetMapper( self.visible_plane_mapper )

            self.visible_plane_property = self.vtk_vtk.newProperty()
            self.visible_plane_property.SetColor( (0.0,1.0,1.0) )
            self.visible_plane_property.SetOpacity( 0.5 )
            self.visible_plane_actor.SetProperty( self.visible_plane_property )


        self.plane.SetOrigin( plane_origin )
        self.plane.SetNormal( plane_normal )
        self.vtk_vtk.render()


    def setVisiblePlaneIsVisible( self, yes_no ):
        """
        Toggle visibility.
        """
        anag_utils.funcTrace()
        assert( yes_no==0  or  yes_no==1 )
        self.visible_plane_is_visible = yes_no
        if yes_no == 0:
            self.vtk_vtk.removeActor( self.visible_plane_actor )
        else:
            self.vtk_vtk.addActor( self.visible_plane_actor )
        self.vtk_vtk.render()


class _ClipModes:
    none  = 0
    plane = 1
    eb    = 2
    # FIXME: plane and eb clipping shouldn't be mutually exclusive
g_clip_modes = _ClipModes()
