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

# File: vtk_slice.py
# Author: TDSternberg
# Created: 6/04/01

""" Slice manipulation.

    In the Tcl version of ChomboVis, we had three files vtkSliceX.tcl,
    vtkSliceY.tcl and vtkSliceZ.tcl.  Here, we've condensed them into just one
    class of which we make three instances -- one for x, one for y, one for z.
"""

import os
import math

import tkMessageBox
import vtkpython
import libVTKChomboPython

import anag_utils
import algorithms
import anag_box_clipper
from vtk_clip import g_clip_modes
from self_control import SelfControl
from vtk_grid import g_detail_modes
g_grid_detail_modes = g_detail_modes

g_decls = anag_utils.Declarations('g_decls', 'g_grid_detail_modes')

class VtkSlice( SelfControl ):
    """ 
    The real action is in class _SlicingPlane.  This class holds three
    instances of that.  Note there's a class _SlicingPlane in control_slice
    too: not to be confused.
    """

    def __init__(self, dep_dict):
        anag_utils.funcTrace()

        SelfControl.__init__( self,dep_dict,
          metadata = [
            { 'name':'slicing_planes', 'get':1, 'initval':{}},
            { 'name':'n_instantiated_planes', 'get':1,
                'initval':{"x":0, "y":0, "z":0}},
            { 'name':'planar_probe' },
            { 'name':'clip_mode','get':1,'set':2,
                'initval':g_clip_modes.none, 'notify':1, 'save':5},
            { 'name':'isoclip_component','get':1,'set':1, 'save':5},
            { 'name':'local_vtk_data', 'get':1},
            { 'name':'use_ghost_cells', 'get':1, 'set':2, 'save':1 },
            { 'name':'grid_plane_offset', 'get':1, 'set':2, 'save':1 }
          ])
    

        if self.cmd_line.getNoVtk():
            return

        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'dep_dict' )

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            instance_identifier = 'slice')
        data_centering = self.local_vtk_data.getDataCentering(
            permute_in_reslice_mode=0) 
            # We permute in ChomboReader.  Makes no sense to permute here, as
            # this statement is reached just once, while we can change the
            # reslice direction from the GUI.

        self.local_vtk_data.reader.SetOriginShift(
            0.5*data_centering[0], 0.5*data_centering[1], 0.5*data_centering[2],
            -.5*data_centering[0], -.5*data_centering[1], -.5*data_centering[2])

        # Slicing planes -- x, y and z.
        if not self.local_vtk_data.is2DMode():
            for direction in 'x', 'y':
                self.newPlane( direction )
        self.newPlane( 'z' )

        # Set the gap between the grid planes to something reasonable.
        extents = self.local_vtk_data.getDomainExtentsXYZ()
        largest_domain_dimension = max( extents[3]-extents[0],
                                        extents[4]-extents[1],
                                        extents[5]-extents[2] )
        self.setGridPlaneOffset( largest_domain_dimension/1000.0 )

        if self.cmd_line.getNoVtk():
            return


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self ):
        """
        Overrides SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getNoVtk():
            return

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        if not self.use_ghost_cells:  # Hasn't been initialized.
            self.use_ghost_cells = self.local_vtk_data.getAlwaysUsePaddedBoxes()
        self.setUseGhostCells( self.use_ghost_cells )

        self.local_vtk_data.setSliceOffset(
            self.local_vtk_data.getSliceOffset(),
            force_update=1 )

        # FIXME: when we come up in reslice mode (after slicing in 3D mode)
        # the plane is visible iff the zplane was visible in 3D mode --
        # regardless of what the reslice direction is.  I think the problem is
        # that the multi_instance_qualifier thing isn't working, and in a state
        # restoration all the planes pick up the 'save':1 attributes of the last
        # plane saved, i.e. the z plane.  Notice:
        #if self.vtk_data.isResliceMode():
        #    anag_utils.info( self.vtk_data.getResliceDirection(), 
        #               "visible=",
        #               self.slicing_planes[self.vtk_data.getResliceDirection()
        #                 + '0' ].getSliceIsVisible() )
        # Therefore this...
        #   self.slicing_planes['z'].setSliceIsVisible(
        #       self.slicing_planes[self.vtk_data.getResliceDirection()
        #                          ].getSliceIsVisible() )
        # ...doesn't do any good.
        #
        # I've fixed this by forcing the slice to always be visible, in reslice
        # mode.

        self._gridLineWidthCallback( self.vtk_data.getGridLineWidth(), None )
        self.setGridPlaneOffset( self.grid_plane_offset )
        self._gridColorCallback( None, None )

        self.registerCallback(
            self.vtk_data.getNotifierSliceGridVisibility(),
            self._sliceGridVisibilityCallback )
        self.registerCallback(
            self.vtk_data.getNotifierGridColorNotifier(),
            self._gridColorCallback )
        self.registerCallback(
            self.vtk_data.getNotifierGridLineWidth(),
            self._gridLineWidthCallback )
        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )

        self.setClipMode( self.getClipMode(), self.isoclip_component )


    def updateVis( self ):
        if self.local_vtk_data.getReader().GetNumPieces() > 0:
            self.local_vtk_data.setCurComponent(
                self.local_vtk_data.getCurComponent(), force=1 )


    def cleanup( self ):
        anag_utils.funcTrace()
        for k in self.slicing_planes.keys():
            anag_utils.info( "Calling self.slicing_planes[k].cleanup()" )
            self.slicing_planes[k].cleanup()
            anag_utils.info( "self.slicing_planes.keys()=",
                              self.slicing_planes.keys().sort() )


    def setCurrentComponent( self, name ):
        """
        Called from eb_api, to set it to 'blank_covered_cells'.
        """
        anag_utils.funcTrace()
        self.local_vtk_data.setCurComponent( name )


    def constructPlanarProbe( self ):
        self.planar_probe = _PlanarProbe(
            dep_dict = { 'saved_states':self.saved_states,
                         'vtk_vtk':self.vtk_vtk,
                         'local_vtk_data':self.local_vtk_data,
                         'vtk_cmap':self.vtk_cmap })


    def newPlane( self, axis,
                  independent_vtk_data=False,
                  component=None,
                  data_tied_to_plane=None ):
        """
        Creates a slicing plane perpendicular to arg axis.  This function is
        for creating the 1 (2D) or 3 (3D) default planes as well as additional
        planes as needed (e.g. the fluid2 plane).

        Returns this plane's unique id number -- what you need to pass to some
        of the other SliceAPI methods, if you want to manipulate this plane.

        If you skip the optional arguments, you get a plane that shares the
        local_vtk_data, and therefore the currcomponent and visible levels
        known to class VtkSlice (as opposed to the possible special ones of a
        specific _SlicingPlane).

        Set arg independent_vtk_data to True and your new plane has its own
        local_vtk_data.  Its visible levels are tied to those of your other
        planes but its component is not.  You can indicate its component with
        arg component, and change it later with setCurrentComponent().

        Use arg data_tied_to_plane to indicate another plane, whose
        local_vtk_data (and by implication curcomponent) you want this plane
        to share.  Do not attempt to set arg component, unless you want to
        change the curcomponent of whatever other plane whose local_vtk_data
        you're sharing.  If you use arg data_tied_to_plane, then leave
        independent_vtk_data as False.
        """
        anag_utils.funcTrace()

        assert( not (independent_vtk_data and data_tied_to_plane) )

        if   independent_vtk_data:
            lvd = self.vtk_data.makeLocalVtkData(
                follow_global_curcomponent=0,
                instance_identifier = 'slice')
            data_centering = lvd.getDataCentering( permute_in_reslice_mode=0 )
            lvd.reader.SetOriginShift(
               .5*data_centering[0], .5*data_centering[1],.5*data_centering[2],
              -.5*data_centering[0],-.5*data_centering[1],-.5*data_centering[2])
        elif data_tied_to_plane:
            lvd = data_tied_to_plane.local_vtk_data
        else:
            lvd = self.local_vtk_data

        uniqizer = self.n_instantiated_planes[axis]
        self.n_instantiated_planes[axis] = self.n_instantiated_planes[axis] + 1
        self.slicing_planes[axis + str(uniqizer)] = _SlicingPlane(
                                           axis,
                                           uniqizer,
                                           dep_dict = {
                                           'saved_states':self.saved_states,
                                           'vtk_vtk':self.vtk_vtk,
                                           'local_vtk_data':lvd,
                                           'global_vtk_data':self.vtk_data,
                                           'vtk_cmap':self.vtk_cmap,
                                           'vtk_clip':self.vtk_clip,
                                           'cmd_line':self.cmd_line })

        new_plane = self.getSlicingPlanes()[axis+str(uniqizer)]
        if independent_vtk_data:
            new_plane.setCurrentComponent( component )

        if (lvd != self.local_vtk_data) or (uniqizer > 0):
            new_plane._initForFirstHDF5()    
            new_plane._refresh()
            new_plane.toggleSliceVisibility( on_off=1 )

        return uniqizer


    def setClipMode( self, mode, component=None ):
        """
        Arg mode should be a field of g_clip_modes.
        Arg component should be non-None when mode==g_clip_modes.eb, and should
          indicate the name of the component whose along whose zero isocontour
          we need to clip.
        """
        anag_utils.funcTrace()
        self.clip_mode.set( mode )
        if mode == g_clip_modes.eb:
            self.setUseGhostCells(0)
        self.isoclip_component = component
        for plane in self.slicing_planes.values():
            plane.setClipMode( mode, component )


    def setOffset( self, x ):
        anag_utils.funcTrace()
        if( (x != 0.0)
        and (self.slicing_planes.values()[0].clipping_vtk_data) ):
            anag_utils.warning( "Can't change offset when clipping against an "
                "isocontour." )
            return
        self.local_vtk_data.setSliceOffset( x )
        for plane in self.slicing_planes.values():
            if plane.clipping_vtk_data:
                plane.clipping_vtk_data.setSliceOffset( x )


    def setGridPlaneOffset( self, x ):
        anag_utils.funcTrace()
        extents = self.local_vtk_data.getDomainExtentsXYZ()
        largest_domain_dimension = max( extents[3]-extents[0],
                                        extents[4]-extents[1],
                                        extents[5]-extents[2] )
        if( self.grid_plane_offset
        and (algorithms.floatEquals(x, self.grid_plane_offset, 
                                  largest_domain_dimension/10000.0 ))):
            return
        self.grid_plane_offset = x
        for p in self.slicing_planes.values():
            p.grid_plane_source.SetSliceCellsDelta( x )
            p.local_vtk_data.vtkUpdate( source='slice_' + p.axis )

    def getOffset( self ):
        anag_utils.funcTrace()
        return self.local_vtk_data.getSliceOffset()


    def getAllLocalVtkDatas( self ):
        """
        Returns a tuple with all the VtkData or LocalVtkData objects to which
        any class in this module holds a reference.
        """
        anag_utils.funcTrace()
        d = {self.local_vtk_data:1}
        for p in self.slicing_planes.values():
            d[p.local_vtk_data] = 1
            if p.clipping_vtk_data:
                d[p.clipping_vtk_data] = 1
        return d.keys()
        

    def _paddedBoxesCallback( self, yes_no, dummy ):
        anag_utils.funcTrace()
        self.setUseGhostCells( yes_no )


    def _gridLineWidthCallback( self, width, dummy ):
        """
        Lets slice grids' width change in sync with the other kinds of grids
        (bounding boxes, face cells, all cells, domain box -- which are owned
        by vtk_grid).
        """
        anag_utils.funcTrace()
        for plane in self.slicing_planes.values():
            plane.setGridLineWidth( width )


    def _gridColorCallback( self, dummy1, dummy2 ):
        """
        Callback for global_vtk_data.grid_color_notifier.  Enables us to react
        to grid-color indications coming from vtk_grid.
        """
        anag_utils.funcTrace()
        for plane in self.slicing_planes.values():
            plane.setGridColor( self.vtk_data.getGridColor()[:] )

    def _sliceGridVisibilityCallback( self, on_off, dummy ):
        """
        Callback for global_vtk_data.slice_grid_visibility, which is controlled
        from vtk_grid.  Thus, vtk_grid determines if the plane cells are to be
        visible, but it's up to the individual _SlicingPlane objects here to
        actually draw those plane cells.
        """
        anag_utils.funcTrace()
        for plane in self.slicing_planes.values():
            plane.toggleSliceGridVisibility( on_off )
        self.vtk_vtk.render()


    def setUseGhostCells( self, on_off ):
        anag_utils.funcTrace()
        if self.use_ghost_cells != on_off:
            self.use_ghost_cells = on_off
            self.local_vtk_data.setAlwaysUsePaddedBoxes( on_off )
            for axis in 'x', 'y', 'z':
                self.local_vtk_data.vtkUpdate( source = 'slice_' + axis )

    
    def setDiffuse( self, x ):
        """
        Sets the Vtk diffuse lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.funcTrace()
        for s in self.slicing_planes.values():
            s.setDiffuse( x )
    def setAmbient( self, x ):
        """
        Sets the Vtk ambient lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.funcTrace()
        for s in self.slicing_planes.values():
            s.setAmbient( x )
    def setSpecular( self, x ):
        """
        Sets the Vtk specular lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.funcTrace()
        for s in self.slicing_planes.values():
            s.setSpecular( x )


    # Other getters
    def getSlice( self, name ):
        """
        Arg name is 'x', 'y' or 'z' plus a uniqizing integer.  See
        self.n_instantiated_planes.
        """
        return self.slicing_planes[ name ]

    def unitTest(self):
        anag_utils.funcTrace()
        return
        if self.saved_states.getNumHDF5sLoaded() > 0:
            self.slicing_planes['z0'].movePlane( 1.23456 )
            self.slicing_planes['z0'].toggleSliceVisibility( 0 )


class _ChomboTexture( SelfControl ):
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        SelfControl.__init__( self, dep_dict, metadata=[
            {'name':'vtk_texture', 'get':1},  # vtkTexture -- C++
            {'name':'memory', 'get':1},       # texture memory
            {'name':'memory_bounds', 'get':1},
            {'name':'vtk_chombo_texture' },# vtkChomboTexture--C++

             # Optimizations for reslice mode:
            {'name':'last_partition_direction'},
            {'name':'last_visible_levels'}
            ])

        self.decls = anag_utils.Declarations( "decls", instance=self )
        # self.vtk_texture
        self.vtk_texture = self.vtk_vtk.newTexture()
        self.vtk_texture.InterpolateOff()
        self.vtk_texture.MapColorScalarsThroughLookupTableOn()
        self.memory = vtkpython.vtkStructuredPoints()
        self.vtk_chombo_texture = libVTKChomboPython.vtkChomboTexture()


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        self.vtk_chombo_texture.SetChomboReader(self.local_vtk_data.getReader())
        self.updateColormap( self.vtk_cmap.getActiveColormap() )


    def _refresh( self ):
        anag_utils.funcTrace()

    def cleanup( self ):
        anag_utils.funcTrace()
        self.vtk_texture.SetInput( None )
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def getMemoryPartition( self, box_num ):
        """
        Arg box_num is an index into the "visible pieces".
        Return value is list of pairs -- the coordinates, in the index
        space of the texture memory, of the box's four corners at the box's
        packed location -- as a fraction of the packed domain's width and
        height.
        """
        anag_utils.funcTrace()
        raw = self.vtk_chombo_texture.GetMemoryPartition( box_num )
        result = [[raw[0],raw[1]],
                  [raw[2],raw[3]],
                  [raw[4],raw[5]],
                  [raw[6],raw[7]]]
        # We return lists, rather than tuples, because later we will make
        # some adjustments when the centering is not "cell".

        return result


    def updateColormap( self, cmap ):
        anag_utils.funcTrace()
        self.vtk_texture.SetLookupTable( cmap )
        self.vtk_vtk.render()


    def updateTextureMemory( self, direction, position, force=0 ):
        anag_utils.funcTrace()

        levels = (self.local_vtk_data.getMinVisibleLevel(),
                  self.local_vtk_data.getMaxVisibleLevel())
        if( self.last_partition_direction != direction
        or  self.last_visible_levels != levels
        or  force == 1 ):
            self.vtk_chombo_texture.UpdateMemoryPartition(direction)
            self.last_partition_direction = direction
            self.last_visible_levels = levels
            #anag_utils.info( "Recomputed partition" )
        self.memory = self.vtk_chombo_texture.MakeTextureMemory( direction,
                                                                 position)
        self.memory_bounds = self.memory.GetBounds()
        self.vtk_texture.SetInput( self.memory )


class _SlicingPlane( SelfControl ):
    def __init__(self, axis, uniqizer, dep_dict ):
        """ arg axis must be 'x', 'y' or 'z'. """
        anag_utils.funcTrace()
        instance_data = [
           { 'name':'axis', 'get':1 },
           { 'name':'slice_prop'},
           { 'name':'matt_slicer', 'get':1 },
           { 'name':'accumulator', 'get':1 },
           { 'name':'xform_texture'},
           { 'name':'xform_polydata'},
           { 'name':'texture_map_to_plane'},
           { 'name':'clipmode_inside_out', 'set':1, 'initval':0},
           { 'name':'box_clipper' },
           { 'name':'slice_mapper', 'get':1 },
           { 'name':'slice_actor', 'get':1 },
           { 'name':'slice_is_visible', 'get':1, 'set':1, 'save':1 },
           { 'name':'plane_position', 'get':1, 'save':5, 'notify':1 },
           { 'name':'multi_instance_qualifier'}, # To make unique state keys.
           { 'name':'chombo_texture', 'get':1},
           { 'name':'poly_source'},
           { 'name':'clip_mode', 'get':1, 'set':2},
           { 'name':'prev_clip_mode'},
           { 'name':'resliced_vtk_data' },
           { 'name':'s_scalars'},
           { 'name':'num_visible_pieces'},
           { 'name':'clipping_vtk_data'},

           { 'name':'grid_plane_source'},
           { 'name':'grid_plane_property'},
           { 'name':'grid_plane_accumulator'},
           { 'name':'grid_plane_mapper'},
           { 'name':'grid_plane_actor'},
           { 'name':'grid_plane_is_visible', 'save':1, 'initval':0}
        ]
        
        SelfControl.__init__( self, dep_dict, instance_data)

        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'axis', 'axis_num',
            'instance_data', 'dep_dict', 'uniqizer' )

        assert( axis=='x' or axis=='y' or axis=='z' )

        self.multi_instance_qualifier = axis + str(uniqizer)
        self.axis = axis
        self.slice_is_visible = 1

        # We construct matt_slicers and accumulator here because we need them
        # in class VtkSlice's constructor.
        self.matt_slicer = libVTKChomboPython.vtkMattSlicer()
        self.accumulator = libVTKChomboPython.vtkChomboAccumulatePolyData()

        self.chombo_texture = _ChomboTexture(
            dep_dict={'saved_states':self.saved_states,
                      'matt_slicer':self.matt_slicer,
                      'vtk_vtk':self.vtk_vtk,
                      'local_vtk_data':self.local_vtk_data,
                      'vtk_cmap':self.vtk_cmap})
        # This is for when we instantiate an extra (i.e. 4th, 5th, etc) slice
        # and we've already loaded our dataset:
        if self.saved_states.getNumHDF5sLoaded() == 1:
            self.chombo_texture._initForFirstHDF5()

        #
        # Set up vtk pipeline for grid_plane -- what produces the "slice cells"
        # option of control_grid.py.
        #
        self.grid_plane_source = libVTKChomboPython.vtkChomboOutlineSource()
        self.grid_plane_property = self.vtk_vtk.newProperty()
        self.grid_plane_accumulator =\
            libVTKChomboPython.vtkChomboAccumulatePolyData()
        self.grid_plane_mapper = self.vtk_vtk.newMapper()
        self.grid_plane_actor = self.vtk_vtk.newActor()

        self.grid_plane_source.SetPlaneNormal( self.axis )
        self.grid_plane_source.SetDetailMode( g_grid_detail_modes.slice_cells )
        self.grid_plane_property.SetColor( 1,1,1 )

        self.grid_plane_accumulator.SetInput(
            self.grid_plane_source.GetOutput() )
        self.grid_plane_mapper.SetInput(self.grid_plane_accumulator.GetOutput())
        self.grid_plane_actor.SetMapper( self.grid_plane_mapper )
        self.grid_plane_actor.SetProperty( self.grid_plane_property )

        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        self.grid_plane_source.SetChomboReader(
            self.local_vtk_data.getReader() )

        if self.local_vtk_data.textureOn():
            self.poly_source = libVTKChomboPython.vtkChomboPlaneSource()
                #Rectangle to texture.
            self.texture_map_to_plane = vtkpython.vtkTextureMapToPlane()
            self.texture_map_to_plane.SetInput( self.poly_source.GetOutput() )
            self.xform_texture = vtkpython.vtkTransformTextureCoords()
            self.xform_texture.SetInput( self.texture_map_to_plane.GetOutput() )
            self.xform_polydata = vtkpython.vtkTransformPolyDataFilter()
            self.xform_polydata.SetInput( self.xform_texture.GetOutput() )

        self.slice_prop = self.vtk_vtk.newProperty()
        self.slice_prop.SetInterpolationToFlat()

        self.slice_mapper = self.vtk_vtk.newMapper()
        self.slice_mapper.SetInput( self.accumulator.GetOutput() )
        self.slice_mapper.SetColorModeToMapScalars()
        if self.local_vtk_data.textureOn():
            self.slice_mapper.ScalarVisibilityOff()
        else:
            self.slice_mapper.ScalarVisibilityOn()
        self.slice_mapper.SetLookupTable( self.vtk_cmap.getActiveColormap() )
        self.slice_mapper.ImmediateModeRenderingOn()

        self.slice_actor = self.vtk_vtk.newActor()    
        self.slice_actor.SetProperty( self.slice_prop )
        self.slice_actor.SetMapper( self.slice_mapper )

        # Set up vtk pipeline for slice.
        if self.local_vtk_data.textureOn():
            self._setupTexturePipeline()
        else:
            self._setupNoTexturePipeline()

        domain_size = self.local_vtk_data.getDomainMax([self.axis]) \
                    - self.local_vtk_data.getDomainMin([self.axis])
        self.setPlanePosition( algorithms.roundDown(
                (self.local_vtk_data.getDomainMin([self.axis]) +
                 self.local_vtk_data.getDomainMax([self.axis]))/2.00,
                int(-math.log(domain_size))+5 ) )
        # We don't want to start exactly at the middle of the domain, as that
        # is liable to put us right at a box boundary, where occasionally we
        # get unpleasant interference effects.

        if self.local_vtk_data.is2DMode():
            self.setPlanePosition( 0 )

        # Did that because if we use getDomainMin|Max['z'], then the
        # higher-level slices never get rendered.
        
        if self.local_vtk_data.is2DMode():
            self.setDiffuse( 1.0 )
            self.setAmbient( 0.0 )
        else:
            self.setDiffuse( 0.85 )
            self.setAmbient( 0.15 )


    def unregisterFromNotifiers( self ):
        """
        Called from class Slice, as a prelude to destroying this instance
        altogether.
        """
        anag_utils.funcTrace()
        #self.saved_states.num_HDF5s_loaded.removeCallback(


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self ):
        """
        Overrides SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        # Reregister with Notifiers.
        self.registerCallback(
            self.vtk_cmap.getNotifierActiveColormap(), self.updateColormap )

        if not self.local_vtk_data.getTextureEnabled():
            map( lambda notifier, self=self :
                self.registerCallback( notifier,
                                       lambda d1,d2,self=self:
                                           self._cmappedRangeCallback() ),
                self.local_vtk_data.getCmappedRangeMinNotifiers()
               +self.local_vtk_data.getCmappedRangeMaxNotifiers() )

        self.registerCallback(
            self.local_vtk_data.getNotifierAccumStart(), self.updateAccumStart )
        self.registerCallback(
            self.local_vtk_data.getNotifierAccumMiddle(),self.updateAccumMiddle)
        self.registerCallback(
            self.local_vtk_data.getNotifierAccumEnd(), self.updateAccumEnd )

        self.registerCallback(
            self.local_vtk_data.getNotifierCurComponent(), self.changeComponent)

        self.registerCallback(
            self.local_vtk_data.getNotifierTextureEnabled(),
            lambda on_off, dummy, self=self, 
                handlers= (self._setupNoTexturePipeline,
                           self._setupTexturePipeline):
                apply( handlers[on_off], () ))

        # Forcing slice to be visible in reslice mode... (See my comments on
        # this under VtkSlice._refresh()).
        if( self.local_vtk_data.isResliceMode()
        and self.saved_states.getIs3DMode() ):
            self.toggleSliceVisibility( on_off=1 )
        else:
            self.toggleSliceVisibility( self.slice_is_visible )

        self.updateColormap( self.vtk_cmap.getActiveColormap(), None )
        self.movePlane( self.getPlanePosition(), force_update=1 )
        self.toggleSliceGridVisibility( self.grid_plane_is_visible )
        self.changeComponent()


    def _cmappedRangeCallback( self ):
        anag_utils.funcTrace()
        new_range = self.vtk_cmap.getDefactoRange()
        if self.slice_mapper.GetScalarRange() != new_range:
            apply( self.slice_mapper.SetScalarRange, new_range )
            self.vtk_vtk.render()


    def setClipMode( self, mode, component ):
        """
        See setClipMode() in class VtkSlice.
        """
        anag_utils.funcTrace()
        gc = g_clip_modes
        assert( (mode==gc.none) or (mode==gc.eb) or (mode==gc.plane) )
        self.prev_clip_mode = self.clip_mode
        self.clip_mode = mode

        if mode == g_clip_modes.eb:
            if not self.box_clipper:
                self.box_clipper = anag_box_clipper.AnagBoxClipper( self.axis )
            self.box_clipper.SetInput( self.xform_polydata.GetOutput() )
            self.accumulator.SetInput( self.box_clipper.GetOutput() )

            if not self.clipping_vtk_data:
                self.clipping_vtk_data = self.global_vtk_data.makeLocalVtkData(
                    follow_global_padded_boxes=0,
                    follow_global_curcomponent=0,
                    never_do_vtk_update=1,
                    instance_identifier='clipping_in_vtk_slice_' + self.axis  )
                self.clipping_vtk_data.setAlwaysUsePaddedBoxes(1)

            self.clipping_vtk_data.setCurComponent( component )
            if self.local_vtk_data.is2DMode():
                self.local_vtk_data.setSliceOffset( 0.0, force_update=1 )
                self.clipping_vtk_data.setSliceOffset( 0.0, force_update=1 )
                # Otherwise we layer the slices and, clipping against
                # isocontours, pieces of coarse boxes can be seen inside the
                # clipped region.

            if( (not self.local_vtk_data.is2DMode())
            or  (self.local_vtk_data.isResliceMode()) ):
                if self.local_vtk_data.isResliceMode():
                    axis = self.local_vtk_data.getResliceDirection()
                    position = self.local_vtk_data.getReslicePosition()[axis]
                else:
                    axis = self.axis
                    position = self.plane_position.get()
                self.clipping_vtk_data.setResliceDirection( axis )
                self.clipping_vtk_data.setReslicePosition( position, axis )
                self.clipping_vtk_data.setSliceOffset(
                    self.local_vtk_data.getSliceOffset(), force_update=1 )

            self.matt_slicer.UseTriangleStrips( 0 )
            self.vtk_clip.clipPipeline( 
                self.accumulator, self.slice_mapper,
                clip_against_isocontour=True,
                isocontour_clip_value =
                    self.global_vtk_data.isocontour_clip_value,
                inside_out = self.clipmode_inside_out )
        elif mode == gc.plane:
            self.vtk_clip.clipPipeline( self.accumulator, self.slice_mapper)
            self.vtk_clip.clipPipeline( self.grid_plane_accumulator,
                                            self.grid_plane_mapper )
            pos = self.getPlanePosition()
            self.movePlane( pos + 0.001 )
            self.movePlane( pos )

        else:
            self.matt_slicer.UseTriangleStrips( 1 )

            if   self.prev_clip_mode == gc.eb:
                self.vtk_clip.unclipPipeline(
                               self.accumulator, self.slice_mapper,
                               clip_against_isocontour=True )
                self.accumulator.SetInput( self.xform_polydata.GetOutput() )
            elif self.prev_clip_mode == gc.plane:
                self.vtk_clip.unclipPipeline(
                               self.accumulator, self.slice_mapper,
                               clip_against_isocontour=False )
                self.vtk_clip.unclipPipeline( self.grid_plane_accumulator,
                                              self.grid_plane_mapper )



    def cleanup( self ):
        anag_utils.funcTrace()
        self.toggleSliceGridVisibility( 0 )
        self.toggleSliceVisibility( 0 )

        self.grid_plane_accumulator.SetInput( None )
        self.grid_plane_mapper.SetInput( None )
        self.grid_plane_actor.SetMapper( None )
        self.grid_plane_actor.SetProperty( None )
        self.slice_mapper.SetInput( None )
        self.slice_actor.SetProperty( None )
        self.slice_actor.SetMapper( None )
        self.accumulator.SetInput( None )
        self.chombo_texture.getVtkTexture().SetInput( None )
        self.slice_actor.SetTexture( None )
        self.matt_slicer.SetInput( None )

        self.chombo_texture.cleanup()
        self.unregisterCallbacks()
        self.zeroSelfVariables()

        
    def setGridColor( self, rgb ):
        anag_utils.funcTrace()
        if self.grid_plane_property:
            apply( self.grid_plane_property.SetColor, rgb[:] )

    def setGridLineWidth( self, width ):
        anag_utils.funcTrace()
        if self.grid_plane_property:
            self.grid_plane_property.SetLineWidth( width )


    def _setupTexturePipeline( self ):
        """
        Not the whole pipeline, but just the part specific to support for
        texture mapping.
        """
        anag_utils.funcTrace()

        # FIXME.  The idea was to freely enable switching texture-mapping on
        # and off.  Unfortunately, it doesn't work well.  If we turn off texture
        # mapping, the colormap comes out wrong.  And if we initialize
        # vtk_data.texture_enabled to 0, and then try to set it to 1 at runtime,
        # we get a segfault.  So for now, let's go with a permanent setting, set
        # from the command line.

        self.accumulator.SetInput( self.xform_polydata.GetOutput() )
        self.chombo_texture.getVtkTexture().SetInput(
            self.chombo_texture.getMemory() )
        self.slice_actor.SetTexture( self.chombo_texture.getVtkTexture() )


    def _setupNoTexturePipeline( self ):
        """
        Not the whole pipeline, but just the part specific to support for
        texture mapping.
        """
        anag_utils.funcTrace()
        self.matt_slicer.SetInput( self.local_vtk_data.getReader().GetOutput() )
        axis_num = (self.axis=='y') + 2*(self.axis=='z')
        self.matt_slicer.SetAxis( axis_num )
        self.accumulator.SetInput( self.matt_slicer.GetOutput() )


    #
    # updateAccum* functions -- triggered by Notifier ________.
    #
    def updateAccumStart( self, dummy, extra_info ):
        """
        Arg dummy is the Notifier that triggered this call.  We just don't
        need it in this function.
        """
        anag_utils.funcTrace()
        if self.groundsToSkipUpdate( extra_info, doprint=1 ):
            return

        self.num_visible_pieces = 0

        if self.local_vtk_data.textureOn():
            if   not self.local_vtk_data.is2DMode():
                direction = self.getAxis()
                position =  self.getPlanePosition()
            elif self.local_vtk_data.isResliceMode():
                direction = self.local_vtk_data.getResliceDirection()
                position =  self.local_vtk_data.getReslicePosition()[direction]
            else: # plain 2D mode
                direction = 'z'
                position = 0.0
            self.getChomboTexture().updateTextureMemory(
                direction, position, force=1 )
            # The force=1 is for when we change the slice offset to or from 0.0
            # (and thus change the all-real-box regime).

        self.accumulator.StreamExecuteStart()
        self.grid_plane_accumulator.StreamExecuteStart()


    def updateAccumMiddle( self, box_num, extra_info ):
        """
        Arg box_num is what the Notifier that triggered this call was just
        set to.  It's an index into the current active box list.

        Assumes ChomboReader.SetPiece( box_num ) has just been called.
        """
        anag_utils.funcTrace()
        if self.groundsToSkipUpdate( extra_info, doprint=0 ):
            return

        level = self.local_vtk_data.getVisibleBoxLevel( box_num )
        box_in_level = self.local_vtk_data.getVisibleBoxNum( box_num )
#       if box_in_level != 0: return

        padded = self.local_vtk_data.currentPieceIsPadded()
        real   = self.local_vtk_data.currentPieceIsReal()
        multi = self.local_vtk_data.computeLevelOffset( box_num )

        if( not self.local_vtk_data.is2DMode()
        and self.local_vtk_data.planeIntersectsBox( level, box_in_level,
            self.axis, self.getPlanePosition()) ):
            self.grid_plane_source.SetPlanePosition(
                self.getPlanePosition()
              + 6*multi )
            self.grid_plane_source.UpdateOutput( level, box_in_level,
                                                 padded, real )
            self.grid_plane_accumulator.Append()

        if( self.local_vtk_data.is2DMode()
        or self.local_vtk_data.planeIntersectsBox(
                level, box_in_level, self.getAxis(),
                self.getPlanePosition())):
            if self.local_vtk_data.textureOn():
                self.updatePolySource( box_num, 
                                       self.getAxis(),
                                       self.getPlanePosition() + 6*multi )
            else:
                self.matt_slicer.SetOffset( multi )
                self.matt_slicer.Update()
            self.accumulator.Append()
            self.num_visible_pieces += 1


    def updateAccumEnd( self, dummy, extra_info ):
        anag_utils.funcTrace()
        if self.groundsToSkipUpdate( extra_info, doprint=0 ):
            return
        if self.num_visible_pieces == 0:
            self.vtk_vtk.removeActor( self.slice_actor )
            self.vtk_vtk.removeActor( self.grid_plane_actor )
            return
        else:
            self.vtk_vtk.addActor( self.slice_actor )
            if self.grid_plane_is_visible:
                self.vtk_vtk.addActor( self.grid_plane_actor )

        self.accumulator.StreamExecuteEnd()
        if self.grid_plane_is_visible:
            self.grid_plane_accumulator.StreamExecuteEnd()


    def groundsToSkipUpdate( self, update_originator, doprint=1 ):
        """
        Returns 1 if the conditions are such that we ought to skip the vtk
        update and accumulation.
        """
        anag_utils.funcTrace()
        result = 0

        # A slightly complex notFromHere... condition.  self_class_name is
        # 'slice_'+self.axis when the vtkUpdate originates in this slice.
        # It's 'slice' (no suffix) when it originates from
        # local_vtk_data.setSliceOffset() (or anywhere else the event is about
        # slicing but not specific to one slice).
        not_from_here_condition = \
            ( (self.notFromHereOrVtkData( update_originator,
                                          self_class_name='slice_'+self.axis ))
              and (self.notFromHereOrVtkData( update_originator,
                                          self_class_name='slice' ))
            )

        twoD_xy_condition = \
            (
            self.local_vtk_data.is2DMode()
            and (self.axis=='x' or self.axis=='y')
            )

        invisibility_condition = \
            (
            self.getSliceIsVisible() != 1
            )

        if( not_from_here_condition
        or  twoD_xy_condition 
        or  invisibility_condition ):
            result = 1

        """
        if doprint == 1:
            if not_from_here_condition:
                anag_utils.info( "not-from-here condition=",
                                  not_from_here_condition )
            if twoD_xy_condition:
                anag_utils.info( "2D x&y condition=", twoD_xy_condition )
            if invisibility_condition:
                anag_utils.info( "slice-invisible condition=",
                                  invisibility_condition )
        """

        return result


    def updatePolySource( self, box, plane_direction, plane_position ):
        """
        Called from accumulation loop.  self.poly_source is a rectangle which
        we color with a texture.  This function sets self.poly_source's
        polys, points, tcoords, and finally calls its VTK Update().
        """
        anag_utils.funcTrace()

        output = self.local_vtk_data.reader.GetOutput()
        output.Update()
        bounds = output.GetBounds()

        # bounds format: (xlo,xhi,ylo,yhi,zlo,zhi).  In reslice mode, zlo=zhi=0.
        spacing = output.GetSpacing()
        skosh = 0.0001
        half_spacing = (spacing[0]/(2.0+skosh),
                        spacing[1]/(2.0+skosh),
                        spacing[2]/(2.0+skosh))
            # Without the skosh component, adjacent boxes end up sharing texture
            # coordinates (or something like that), and the result is weird
            # jaggy distortions.

        #
        # Create a rectangle -- the slice.
        #
        if not self.local_vtk_data.is2DMode():
            # Permute coords to pick correct plane size.
            permute1 = {'x':(2,4),'y':(4,0),'z':(0,2)}[plane_direction]
            bounds = (bounds[permute1[0]],bounds[permute1[0]+1],
                      bounds[permute1[1]],bounds[permute1[1]+1])

        data_centering = self.local_vtk_data.getDataCentering()
        iso_clip = 1 * (self.clip_mode==g_clip_modes.eb)
                     # If we're clipping along an isocontour.  This grows the
                     # poly_source by a half-cell in each direction, so it's the
                     # size of the cum-ghost box *shrunken* so its point data
                     # falls in the cell centers.
        permute3 = {'x':(1,2), 'y':(2,0), 'z':(0,1)}[plane_direction]
        y_flip = -1 * (plane_direction=='y') \
               +  1 * (plane_direction=='x' or plane_direction=='z')
        h = (half_spacing[permute3[0]], half_spacing[permute3[1]])
        d = (data_centering[permute3[0]], data_centering[permute3[1]])
        point_coords = [
          [bounds[0] - 2*h[0]*(iso_clip) - h[0]*(1-d[0])*(1-iso_clip),
           bounds[2] - 2*h[1]*(iso_clip) - h[1]*(1-d[1])*(1-iso_clip),
           plane_position*y_flip],
          [bounds[1] + 2*h[0]*(iso_clip) + h[0]*(1-d[0])*(1-iso_clip),
           bounds[2] - 2*h[1]*(iso_clip) - h[1]*(1-d[1])*(1-iso_clip),
           plane_position*y_flip],
          [bounds[1] + 2*h[0]*(iso_clip) + h[0]*(1-d[0])*(1-iso_clip),
           bounds[3] + 2*h[1]*(iso_clip) + h[1]*(1-d[1])*(1-iso_clip),
           plane_position*y_flip],
          [bounds[0] - 2*h[0]*(iso_clip) - h[0]*(1-d[0])*(1-iso_clip),
           bounds[3] + 2*h[1]*(iso_clip) + h[1]*(1-d[1])*(1-iso_clip),
           plane_position*y_flip] ]

        # Permute coordinates to orient the rectangle correctly.
        permute2 = {'x':(2,0,1), 'y':(0,2,1), 'z':(0,1,2) }[plane_direction]
        for i in range(0,4):
            point_coords[i] = (point_coords[i][permute2[0]],
                               point_coords[i][permute2[1]],
                               point_coords[i][permute2[2]])

        self.poly_source.SetOrigin( point_coords[0] )
        self.poly_source.SetPoint1( point_coords[1] )
        self.poly_source.SetPoint2( point_coords[3] )

        # Trim away overhang--stuff outside box boundary -- when data centering
        # is other than cell.
        box_dims = [0,0]
        trim_factors = [0,0]
        for i in 0, 1:
            box_dims[i] = bounds[2*i+1] - bounds[2*i]
            trim_factors[i] = 1 \
                            - box_dims[i]/(spacing[permute3[i]]+box_dims[i])
        partition = self.chombo_texture.getMemoryPartition( box )
        partition_bounds = ( partition[1][0]-partition[0][0],
                             partition[2][1]-partition[0][1] )
        half = 0.5
        partition[0][0] += d[0] * half*partition_bounds[0]*trim_factors[0]
        partition[1][0] -= d[0] * half*partition_bounds[0]*trim_factors[0]
        partition[2][0] -= d[0] * half*partition_bounds[0]*trim_factors[0]
        partition[3][0] += d[0] * half*partition_bounds[0]*trim_factors[0]
        partition[0][1] += d[1] * half*partition_bounds[1]*trim_factors[1]
        partition[1][1] += d[1] * half*partition_bounds[1]*trim_factors[1]
        partition[2][1] -= d[1] * half*partition_bounds[1]*trim_factors[1]
        partition[3][1] -= d[1] * half*partition_bounds[1]*trim_factors[1]

        # Set x and y resolutions of self.poly_source.  Note this doubles the
        # running time of our time_test.sh, so leave the resolutions at (1,1)
        # unless you really need it otherwise -- which is the case when you
        # want to clip against an isocontour.
        #
        # When clipping against an isocontour, we use the cum-ghost box data to
        # assign scalars to the poly_source, and then shrink it all so those
        # points fall in the center, instead of at the corners, of the cells.
        # All this gets messed up if the slice is rendered with ghost cells;
        # either we generate an extra layer of ghost cells -- something that
        # we're not now able to do and which would require some significant
        # changes to ghost-cell and box-dimensions bookkeeping down in class
        # VisualizableDataset -- or we just require that ghost cells be turned
        # off in the slice, before any attempt at clipping it against an iso-
        # contour.  We adopt the latter approach.  Even then, there's a problem
        # if the ghost cells are supplied in the hdf5 file and they go more than
        # 1 layer deep.  Using ghost cells always defaults to using the user-
        # supplied ones, if available.  So if those are more than one layer deep
        # then that's what we use, which requires some adjustments to the
        # size of the poly_source and to how we clip off its fringes.
        if iso_clip == 1:
            ghost_depth = self.local_vtk_data.getGhostCellsDepth()
        else:
            ghost_depth = (1,1,1)
        x_res = iso_clip*2*ghost_depth[0] \
              + int( (bounds[1] - bounds[0])/spacing[permute3[0]] )
        y_res = iso_clip*2*ghost_depth[1] \
              + int( (bounds[3] - bounds[2])/spacing[permute3[1]] )

        if iso_clip == 1:
            self.poly_source.SetResolution( x_res, y_res )
            # That gives it (x_res+1)*(y_res+1) points.
        else:
            self.poly_source.SetResolution( 1,1 )

        texture_scale = ( partition[1][0] - partition[0][0],
                          partition[3][1] - partition[0][1] )
        if iso_clip == 1:
            texture_scale = \
             (texture_scale[0]*(x_res/(x_res-d[0]-1.0)),
              texture_scale[1]*(y_res/(y_res-d[1]-1.0)))
        self.xform_texture.SetScale( (texture_scale[0], texture_scale[1], 1.0) )

        if iso_clip == 1:
            dx_in_partition = ( partition_bounds[0]/(x_res-iso_clip),
                                partition_bounds[1]/(y_res-iso_clip) )
            self.xform_texture.SetPosition(
                (partition[0][0] - iso_clip*(1+d[0])*dx_in_partition[0]/2,
                 partition[0][1] - iso_clip*(1+d[1])*dx_in_partition[1]/2,
                 0) )
        else:
            self.xform_texture.SetPosition( (partition[0][0],
                                             partition[0][1],
                                             0) )

        self.xform_texture.SetOrigin(0,0,0)
        self.xform_texture.Update()

        if plane_direction == 'y':
            # We want to reflect through the 45-degree line.  VTK won't let us
            # do that directly, but it's equivalent to a 270-degree rotation
            # around the y axis followed by a 180-degree rotation about the
            # z axis.

            matrix = vtkpython.vtkMatrix4x4()

            #  (1) vtkMatrix4x4 for rotating 270 degrees about y axis:
            # 0  0 -1  0
            # 0  1  0  0
            # 1  0  0  0
            # 0  0  0  1 

            #  (2) vtkMatrix4x4 for rotating 180 degrees about z axis:
            # -1  0  0  0
            #  0 -1  0  0
            #  0  0  1  0
            #  0  0  0  1

            # Product of above two: gotta multiply the z matrix by the y
            # matrix, because internally VTK is postmultiplying the coordinates
            # by the Matrix4x4.
            # 0  0  1  0
            # 0 -1  0  0
            # 1  0  0  0
            # 0  0  0  1
            matrix.SetElement( 0,0,  0 )
            matrix.SetElement( 0,2,  1 )
            matrix.SetElement( 1,1, -1 )
            matrix.SetElement( 2,0,  1 )
            matrix.SetElement( 2,2,  0 )
            # Don't need to set other elements; vtkMatrix4x4 initializes to I_4.

            poly_xform = vtkpython.vtkMatrixToLinearTransform()
            poly_xform.SetInput( matrix )
        else:
            poly_xform = vtkpython.vtkIdentityTransform()
        self.xform_polydata.SetTransform( poly_xform )
        self.xform_polydata.Update()

        if iso_clip==1:

            # Set the poly_source's scalars.
            #
            self.clipping_vtk_data.setMaxVisibleLevel(
                self.local_vtk_data.getMaxVisibleLevel() )
            self.clipping_vtk_data.setMinVisibleLevel(
                self.local_vtk_data.getMinVisibleLevel() )
            if( self.local_vtk_data.is2DMode()
            and (not self.local_vtk_data.isResliceMode()) ):
                self.clipping_vtk_data.setPiece(
                    self.local_vtk_data.reader.GetCurrentPieceIndex() )
            else:
                if self.local_vtk_data.isResliceMode():
                    axis = self.local_vtk_data.getResliceDirection()
                    position = self.local_vtk_data.getReslicePosition()[axis]
                else:
                    axis = self.axis
                    position = self.plane_position.get()
                self.clipping_vtk_data.setReslicePosition( position, [axis,] )
                self.clipping_vtk_data.setPiece(
                    self.local_vtk_data.reader.GetCurrentPieceIndex(),
                    axis = axis,
                    axis_positionXYZ = position )
            self.clipping_vtk_data.reader.Execute()
            self.s_scalars = self.clipping_vtk_data.reader.GetOutput(
                ).GetPointData().GetScalars()

            if self.s_scalars: # None, if current FAB doesn't intersect slice
                if( self.s_scalars.GetNumberOfTuples()
                == self.poly_source.GetOutput().GetPointData(
                                                        ).GetNumberOfTuples() ):
                    self.xform_polydata.GetOutput().GetPointData(
                        ).SetScalars( self.s_scalars )
                    """
                    n = self.s_scalars.GetNumberOfTuples()
                    for i in range(0,n):
                        anag_utils.warning( "*** slice", self.axis,
                            ", scalar[",i,"]=", self.s_scalars.GetTuple1(i) )
                    """
                else:
                    anag_utils.error(
                        "|s_scalars|=",self.s_scalars.GetNumberOfTuples(),
                        "|poly_source|=",
                            self.poly_source.GetOutput().GetPointData(
                                                        ).GetNumberOfTuples())
            else:
                anag_utils.error( "s_scalars=None" )

        # Practice shrinking the polydata (presuming we've enlarged it
        # with ghost cells, so as to line up its points with the cell
        # centers.
        if iso_clip==1:
            input_bounds = self.xform_polydata.GetOutput().GetBounds()
            input_bounds = (input_bounds[permute3[0]*2],
                            input_bounds[permute3[0]*2+1],
                            input_bounds[permute3[1]*2],
                            input_bounds[permute3[1]*2+1] )

            clip_amount = ( h[0] * (1+d[0]),
                            h[1] * (1+d[1]) )
#           clip_amount = (-1000,-1000)
            self.box_clipper.SetClipBoxBounds(
                input_bounds[0]+clip_amount[0], input_bounds[1]-clip_amount[0],
                input_bounds[2]+clip_amount[1], input_bounds[3]-clip_amount[1] )
            self.box_clipper.Update()


    def updateColormap( self, new_colormap, extra_info ):
        """
        Callback registered with vtk_cmap.active_colormap.
        """
        anag_utils.funcTrace()

        self.slice_mapper.SetLookupTable( new_colormap )
        apply( self.slice_mapper.SetScalarRange,
               self.vtk_cmap.getDefactoRange() )
        if self.local_vtk_data.textureOn():
            self.getChomboTexture().updateColormap( new_colormap)


    def movePlane( self, position, force_update=None):
        """
        We don't call it setPlanePosition() because self.plane_position is a
        Notifier, and we need to reserve setPlanePosition() for the operation of
        just setting self.plane_position with no other side effects.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'axis_num_table', 'position',
            'force_update', 'axis_num', 'tol', 'centering', 'dx0' )

        axis_num_table = { 'x':0, 'y':1, 'z':2 }

        axis_num = axis_num_table[self.axis]
        centering = self.local_vtk_data.getDataCentering()[axis_num]
        dx0 = self.local_vtk_data.getReader().GetLevelDx(0)[axis_num]

        if self.local_vtk_data.textureOn():
            # We're positioning the plane in updatePolySource().
            pass
        else:
            self.matt_slicer.SetAxis( axis_num,
                                      position + centering*0.5*dx0 )

        tol = max(abs(position),abs(self.getPlanePosition()))/1000.0;
        if ( (not algorithms.floatEquals( position, self.getPlanePosition(),
                                          tol))
        or (force_update==1) ):
            self.setPlanePosition( position )
            self.local_vtk_data.vtkUpdate( source = 'slice_' + self.axis )

        decls.memberFunctionAudit( self )


    def toggleSliceGridVisibility( self, on_off ):
        """
        Toggle the slice of cell outlines that follows the colored slice.
        But don't show them unless the slice is visible too.
        """
        anag_utils.funcTrace()
        assert( on_off==0 or on_off==1 )

        self.grid_plane_is_visible = on_off

        if self.slice_is_visible==0  and  on_off==1:
            return

        if self.cmd_line.getNoVtk() == 1:
            return
        
        if on_off == 0:
            self.vtk_vtk.removeActor( self.grid_plane_actor )
        else:            
            self.vtk_vtk.addActor( self.grid_plane_actor )
        self.local_vtk_data.vtkUpdate( source = 'slice_' + self.axis )

        self.vtk_vtk.render()


    def toggleSliceVisibility(self, on_off):
        """ If arg on_off==1, then make slice visible.  Otherwise invisible. """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self", "on_off", "ret" )

        if on_off == 0:
            self.slice_is_visible = 0
            if self.axis == 'z':
                self.global_vtk_data.setZSliceIsVisible(0)  # Notifier
            self.vtk_vtk.removeActor( self.slice_actor )
            self.vtk_vtk.render()
        else:
            self.slice_is_visible = 1
            if self.axis == 'z':
                self.global_vtk_data.setZSliceIsVisible(1)  # Notifier
            self.vtk_vtk.addActor( self.slice_actor )
            self.local_vtk_data.vtkUpdate( source = 'slice_' + self.axis )

        self.toggleSliceGridVisibility( self.grid_plane_is_visible )

        decls.memberFunctionAudit( self )


    def changeComponent( self, dummy1=None, dummy2=None ):
        """ 
        What to do when the component changes.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self", "dummy1", "dummy2" )
    
#       if self.local_vtk_data.getUpdateIsDisabled() == 1:
#           return

        apply( self.slice_mapper.SetScalarRange,
               self.vtk_cmap.getDefactoRange() )
        if self.local_vtk_data.textureOn():
            self.vtk_cmap.refreshCmapRange()

        decls.memberFunctionAudit( self )


    def setDiffuse( self, x ):
        anag_utils.funcTrace()
        if self.slice_prop:  # None if no file loaded
            self.slice_prop.SetDiffuse( x )
            self.vtk_vtk.render()
    def setAmbient( self, x ):
        anag_utils.funcTrace()
        if self.slice_prop:
            self.slice_prop.SetAmbient( x )
            self.vtk_vtk.render()
    def setSpecular( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.slice_prop:  # None if no file loaded
            self.slice_prop.SetSpecular( x )
            self.vtk_vtk.render()


    def launch2DViewer(self):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'dummy', 'infile',
            'cmd', 'state_file', 'temp_filename', 'response' )
    
        (state_file,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
            'chombovis_state', '', create=0 )
        self.saved_states.saveState( state_file )

        # The user might have modified the data (added a component, cropped,
        # transformed, etc).  If that's the case, we'll need to make a whole
        # new copy of the dataset.  But that takes a lot of time, so we'll ask
        # the user if that's necessary.
        response = tkMessageBox.askquestion( message =
            "Have you modifed the dataset in-memory, since starting "
            "this ChomboVis session?\n"
            "(The answer is No, if you've only used the GUI; to modify the "
            "dataset you need to do ChomboVis API things with the "
            "BoxLayoutData or VisualizableDataset classes, and you'd know "
            "it if you've done that.)" )
        if response == 'yes':
            (temp_filename, dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
                    'chombovis_temp_file', 'hdf5', create=0, save=1 )
            self.local_vtk_data.saveToHDF5( temp_filename )
            infile = temp_filename
        else:
            infile = self.local_vtk_data.getInfileName()
        cmd = ( self.local_vtk_data.getEnvChomboVisHome() + '/bin/chombovis ' +
                'infile=' + infile +
                ' state_file=' + state_file +
                ' iso_eb=' + str(self.cmd_line.getIsoEb()) +
                ' texture=' + str(self.cmd_line.getTexture()) +
                ' slice_axis=' + self.axis +
                ' axis_position=' + str(self.getPlanePosition()) + '&' )
        anag_utils.info( 'cmd=',cmd )
        os.system( cmd )
        decls.memberFunctionAudit( self )


    def setCurrentComponent( self, component_name ):
        """
        Warning: this will change the component of all other slices that share
        this slice's local_vtk_data, which is to say all other slices, unless
        you called newPlane appropriately.
        """
        anag_utils.funcTrace()
        self.local_vtk_data.setCurComponent( component_name )


#= end of class _SlicingPlane


class _PlanarProbe( SelfControl ):
    def __init__(self, dep_dict ):
        anag_utils.funcTrace()
        instance_data = [
            {'name':'mapper'},
            {'name':'probe'},
            {'name':'probe_normal', 'initval':'z'},
            {'name':'probe_position', 'initval':0},
            {'name':'probe_angle0', 'initval':0},
            {'name':'probe_angle1', 'initval':0},
            {'name':'cast'}
        ]
        SelfControl.__init__( self, dep_dict, instance_data)
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        anag_utils.funcTrace()

        self.createProbe()
        self._refresh()


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()
        self.registerCallback(
            self.vtk_cmap.getNotifierActiveColormap(), self._updateColormap )
        self.registerCallback(
            self.local_vtk_data.getNotifierCmappedRangeMin(),
            self._cmappedRangeCallback )
        self.registerCallback(
            self.local_vtk_data.getNotifierCmappedRangeMax(),
            self._cmappedRangeCallback )
        self.registerCallback(
            self.local_vtk_data.getNotifierCurComponent(),
            self._cmappedRangeCallback )


    def _updateColormap( self, cmap, dummy ):
        anag_utils.funcTrace()
        self.mapper.SetLookupTable( cmap )
        self.vtk_vtk.render()        


    def _cmappedRangeCallback( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        new_range = self.vtk_cmap.getDefactoRange()
        if self.mapper.GetScalarRange() != new_range:
            self.mapper.SetScalarRange( new_range )
            self._updateVis()


    def _updateVis( self ):
        anag_utils.funcTrace()
        self.probe.Update()
        self.cast.Update()
        self.mapper.Update()
        self.vtk_vtk.render()


    def repositionProbe( self, normal, position, angle0, angle1 ):
        """
        See self.createPolygon() for explanation of arguments.
        """
        anag_utils.funcTrace()
        polygon = self.createPolygon( normal, position, angle0, angle1 )
        self.probe.SetInput( polygon.GetOutput() )
        self.vtk_vtk.render()


    def createProbe( self ):
        anag_utils.funcTrace()

        # Create the components
        polygon = self.createPolygon( self.probe_normal, self.probe_position,
                                      self.probe_angle0, self.probe_angle1 )
        self.probe = vtkpython.vtkProbeFilter()
        self.cast = vtkpython.vtkCastToConcrete()
    
        self.mapper = self.vtk_vtk.newMapper()
        self.mapper.SetColorModeToMapScalars()
        self.mapper.ScalarVisibilityOn()
        self.mapper.SetLookupTable( self.vtk_cmap.getActiveColormap() )
        apply( self.mapper.SetScalarRange,
               self.vtk_cmap.getDefactoRange() )
        self.mapper.ImmediateModeRenderingOn()
    
        actor = self.vtk_vtk.newActor()
    
        # Hook 'em up in a pipeline.
        self.probe.SetInput( polygon.GetOutput() )
        self.probe.SetSource( self.local_vtk_data.reader.GetOutput() )
        self.cast.SetInput( self.probe.GetOutput() )
        self.mapper.SetInput( self.cast.GetOutput() )
        actor.SetMapper( self.mapper )
    
        # Display
        self.vtk_vtk.addActor( actor )
        self._updateVis()
    
    
    def createPolygon( self, normal, position, angle0, angle1 ):
        """
        Define and return a polygon that's going to be our probe

        Args normal, position, angle0 and angle1 determined the polygon's
        position and orientation.  We start with the polygon perpendicular to
        arg normal ('x','y', or 'z') at arg position along that axis.  Then
        we rotate polygon, by angle0 and angle1 degrees, around the other two
        axes.  These axes are the horizontal and vertical axes, respectively,
        when the axis indicated by arg normal is pointing out of the screen in
        a right-hand coordinate system, i.e.:
               normal     angle0-axis    angle1-axis
                 x             y              z
                 y             z              x
                 z             x              y
        """
        anag_utils.funcTrace()
    
        plane = libVTKChomboPython.vtkChomboPlaneSource()
        plane.SetResolution( 50, 50 )
        
        domain_midpoint = {}
        domain_size = {}
        for a in 'x','y','z':
            domain_midpoint[a] = 0.5 *\
            (self.local_vtk_data.getDomainMin()[a] +\
             self.local_vtk_data.getDomainMax()[a])
            domain_size[a] = 1.2 *\
            (self.local_vtk_data.getDomainMax()[a] -\
             self.local_vtk_data.getDomainMin()[a])
                 # The factor is so we can test clipping.

        transf = vtkpython.vtkTransform()
        axis_rotators = { 'x' : (transf.RotateY,transf.RotateX), #strangebuttrue
                          'y' : (transf.RotateZ,transf.RotateX),
                          'z' : (transf.RotateX,transf.RotateY) }[ normal ]
        angle_offsets = { 'x' : (90,0),
                          'y' : (0,90),
                          'z' : (0,0) }[ normal ]
        axis_position = { 'x' : (position,0,0),
                          'y' : (0,position,0),
                          'z' : (0,0,position) }[ normal ]
        apply( transf.Translate, axis_position )
        transf.Scale(domain_size['x'],domain_size['y'],domain_size['z'])
        apply( axis_rotators[0], (angle0 + angle_offsets[0],) )
        apply( axis_rotators[1], (angle1 + angle_offsets[1],) )

        tpd = vtkpython.vtkTransformPolyDataFilter()
        tpd.SetInput( plane.GetOutput() )
        tpd.SetTransform( transf )
        tpd.Update()
    
        return tpd
