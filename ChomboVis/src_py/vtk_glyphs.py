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

# File: vtk_glyphs.py
# Author: TDSternberg
# Created: 9/07/04

import libVTKChomboPython
import anag_utils
from self_control import SelfControl

import vtkpython

g_decls = anag_utils.Declarations('g_decls', 'g_glyph_types')

class VtkGlyphs( SelfControl ):
#Cut to here
    """
    You can place glyphs -- little colorable, orientable pyramids -- on embedded
    boundaries, isosurfaces, and the seed points of streamlines.  This class
    gives you control over those glyphs.

    To turn on glyphs, call either c.eb.showGlyphs(), c.iso.showGlyphs() or
    c.stream.showGlyphs() (and turn them off with a corresponding hideGlyphs()).

    The minimum and maximum AMR refinement level the glyphs reflect is not
    settable from this class, but is rather the same as the one in effect for
    the geometry (isosurface, embedded boundary, streamlines) the glyphs are
    attached to.

    Example:
    >>> c.iso.showGlyphs()
    >>> gi = c.iso.getGlyphs()
    >>> gi.setOrientationalComponents(['xmomentum','ymomentum','zmomentum'])
    >>> gi.setColoringComponent('energy')
    >>> gi.setDecimationFactor(0.3)
    >>> c.stream.showGlyphs()
    >>> gs = c.stream.getGlyphs()
    >>> gs.setScaleFactor(1.5)
    """
#Cut from here
    def __init__(self, dep_dict, multi_instance_qualifier, caller_updater=None):
        anag_utils.funcTrace()
        instance_data = [
           { 'name':'multi_instance_qualifier'}, # To make unique state keys.
           { 'name':'glyph' },
           { 'name':'glyph_source' },
           { 'name':'accum' },
           { 'name':'actor' },
           { 'name':'mapper' },
           { 'name':'coloring_component', 'get':1, 'set':2, 'save':1 },
           { 'name':'constant_color', 'set':2, 'save':1 },
           { 'name':'orientational_components', 'initval':[], 'get':1, 'set':2,
                'save':1 },
           { 'name':'scale_factor', 'get':1, 'set':2, 'save':1, 'initval':1.0 },
           { 'name':'base_scale_factor' },
           { 'name':'decimation_factor', 'set':2, 'save':1, 'initval':0.0 },
           { 'name':'local_vtk_data', 'initval':{} },
           { 'name':'caller_updater'}
        ]
        SelfControl.__init__( self, dep_dict, instance_data )
        self.multi_instance_qualifier = multi_instance_qualifier
        self.caller_updater = caller_updater
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls", "instance_data", "dep_dict" )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        # For grabbing field data to use as input for the vtkProbeFilter.
        # The 'c' one is for coloring, the others are for orienting the
        # glyphs.  We use a separate local_vtk_data for each so as to economize
        # on the expensive setCurComponent() call.
        for purpose in 'c', 'x', 'y', 'z':
            self.local_vtk_data[purpose] = self.vtk_data.makeLocalVtkData(
                follow_global_levels=0,
                follow_global_curcomponent=0,
                follow_global_padded_boxes=0,
                always_use_real_boxes=0,
                instance_identifier=self.multi_instance_qualifier + '_glyphs_'
                                   +purpose )

        self.accum = libVTKChomboPython.vtkChomboAccumulatePolyData()
        self.mapper = self.vtk_vtk.newMapper()
        self.props = self.vtk_vtk.newProperty()
        self.actor = self.vtk_vtk.newActor()

        if self.local_vtk_data['c'].is2DMode():
            self.setGlyphType( 'arrow', do_update=False )
        else:
            self.setGlyphType( 'cone', do_update=False )

        self.mapper.ScalarVisibilityOn()
        self.mapper.SetInput( self.accum.GetOutput() )
        self.actor.SetMapper( self.mapper )
        self.actor.SetProperty( self.props )

    
    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()

        if self.coloring_component == None:
            comp_names = self.local_vtk_data['c'].getComponentList()
            n_comps = self.local_vtk_data['c'].getNumComponents()
            self.coloring_component = comp_names[0]
            for i in range(0, self.local_vtk_data['c'].getNumDims()):
                self.orientational_components.append( comp_names[i%n_comps] )

        self._refresh()


    def _refresh( self ):
        anag_utils.funcTrace()

        self.registerCallback(
            self.local_vtk_data['c'].getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data['c'].getCurComponent():
            return   # Will come back here upon loading first component.

        self.registerCallback(
            self.vtk_cmap.getNotifierActiveColormap(),
            lambda new_colormap, dummy : self._updateColormap( new_colormap ))
        map( lambda notifier, self=self :
            self.registerCallback( notifier,
                                   lambda d1,d2,self=self:
                                       self._cmappedRangeCallback() ),
            self.local_vtk_data['c'].getCmappedRangeMinNotifiers()
           +self.local_vtk_data['c'].getCmappedRangeMaxNotifiers() )

        self.glyph.SetScaleFactor( self.base_scale_factor * self.scale_factor)
        

    def _updateColormap( self, new_colormap ):
        anag_utils.funcTrace()
        self.mapper.SetLookupTable( new_colormap )
        self.mapper.SetScalarRange( self.vtk_cmap.getDefactoRange() )
        

    def _cmappedRangeCallback( self ):
        anag_utils.funcTrace()
        new_range = self.vtk_cmap.getDefactoRange()
        if self.mapper.GetScalarRange() != new_range:
            apply( self.mapper.SetScalarRange, new_range )
            self.vtk_vtk.render()


    def _getProbedComponent( self, polydata, local_vtk_data, piece ):
        """
        Arg local_vtk_data should already be set to the appropriate component.
        Returns a vtkDataArray giving the values of at that component of
        the points of arg polydata.
        """
        anag_utils.funcTrace()
#       timer = anag_utils.Timer( 'probe' ); timer.on()

        if local_vtk_data.isResliceMode():
            direction = local_vtk_data.getResliceDirection()
            position =  local_vtk_data.getReslicePosition( [direction,] )
            local_vtk_data.setPiece( piece, direction, position)
        else:
            local_vtk_data.setPiece( piece )
        local_vtk_data.reader.Update()

        probe = vtkpython.vtkProbeFilter()
        field_data = local_vtk_data.getReader().GetOutput()
        probe.SetSource( field_data )

        probe.SetInput( polydata )
        probe.Update()
        
        result = probe.GetOutput().GetPointData().GetScalars()
#       timer.stop()
        return result



    def updateAccumStart( self, min_level, max_level, padded ):
        anag_utils.funcTrace()

        for purpose in 'c', 'x', 'y', 'z':
            self.local_vtk_data[purpose].setAlwaysUsePaddedBoxes( padded )
            self.local_vtk_data[purpose].setMinVisibleLevel( min_level )
            self.local_vtk_data[purpose].setMaxVisibleLevel( max_level )

        if self.constant_color == None:
            self.local_vtk_data['c'].setCurComponentCheaply(
                self.coloring_component)
        axes = ('x','y')
        if self.local_vtk_data['c'].getNumDims() == 3:
            axes = ('x','y','z')
        for axis in axes:
            self.local_vtk_data[axis].setCurComponentCheaply(
                self.orientational_components[{'x':0, 'y':1, 'z':2}[axis]])
        self.accum.StreamExecuteStart()


    def updateAccumMiddle( self, piece, polydata ):
        anag_utils.funcTrace()

        if polydata.GetNumberOfPoints()==0:
            return                                      # Early return

        if self.decimation_factor > 0.0:
            # Decimate the polydata
            triangulate_source = vtkpython.vtkTriangleFilter()
    
            triangulate_source.SetInput( polydata )
                # vtkDecimatePro only works on triangles, and our EB polydata is
                # not all triangles.
            decimated_polydata_source = vtkpython.vtkDecimatePro()
            decimated_polydata_source.SetInput( triangulate_source.GetOutput() )
            decimated_polydata_source.SetTargetReduction( 
                1 - pow(10.0,-self.decimation_factor) )
            decimated_polydata_source.Update()
            decimated_polydata = decimated_polydata_source.GetOutput()
        else:
            decimated_polydata = polydata

        n_points = decimated_polydata.GetNumberOfPoints()
        if n_points == 0:
            return                                      # Early return

        # Set up polydata scalars
        assert( self.coloring_component
             in self.local_vtk_data['c'].getComponentList() )
        decimated_polydata.GetPointData().SetScalars(
            self._getProbedComponent( decimated_polydata,
                                      self.local_vtk_data['c'],
                                      piece))
        assert( decimated_polydata.GetNumberOfPoints() == n_points )

        # Set up polydata vectors
        vectors = vtkpython.vtkDoubleArray()
        vectors.SetNumberOfComponents( 3 )
        vectors.SetNumberOfTuples( n_points )
        c=0
        axis_dict = {0:'x', 1:'y', 2:'z'}
        for comp in self.orientational_components:
            axis = axis_dict[c]
            assert( comp in self.local_vtk_data['c'].getComponentList() )
            vectors.CopyComponent(
                c, 
                self._getProbedComponent( decimated_polydata,
                                          self.local_vtk_data[axis],
                                          piece ),
                0 )
            c += 1
        if self.local_vtk_data['c'].getNumDims() == 2:
            zero_vector = vtkpython.vtkDoubleArray()
            zero_vector.SetNumberOfComponents( 1 )
            zero_vector.SetNumberOfTuples( n_points )
            vectors.CopyComponent( 2, zero_vector, 0 )
        decimated_polydata.GetPointData().SetVectors( vectors )

        self.setScaleFactor( self.scale_factor, update_caller=False )

        self.glyph.SetInput( decimated_polydata )
        self.glyph.Update()
        self.accum.Append()
        

    def updateAccumEnd( self ):
        anag_utils.funcTrace()
        self.accum.StreamExecuteEnd()
        self._cmappedRangeCallback()
        self.vtk_vtk.removeActor( self.actor )
        self.vtk_vtk.addActor( self.actor )


    def hide( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.removeActor( self.actor )
        self.vtk_vtk.render()


#Cut to here
    def setScaleFactor( self, x, update_caller=True ):
        """
        Sets glyphs' linear size (and thus the cube root of their volume...).
        Arg x should be a positive number.  The glyphs' default size corresponds
        to x=1.0.
        
        Users should leave optional argument update_caller alone.
        """

        #Arg update_caller is for preventing infinite recursion.  We set it to
        #False when calling this function from self.updateAccumMiddle().
        anag_utils.funcTrace()
        self.scale_factor = x
        self.glyph.SetScaleFactor( self.base_scale_factor * x )
        # self.base_scale_factor is a constant, fixed in _initForFirstHDF5().
        if update_caller and self.caller_updater:
            apply( self.caller_updater, () )
        self.vtk_vtk.render()


    def setConstantColor( self, rgb ):
        """ Color all the glyphs with the same color. """
        anag_utils.funcTrace()
        self.mapper.ScalarVisibilityOff()
        self.constant_color = rgb
        self.props.SetColor( rgb[0], rgb[1], rgb[2] )
        self.vtk_vtk.render()


    def setColoringComponent( self, comp_name, update_caller=True ):
        """
        Set the data component to color the glyphs by.
        
        Users should leave optional argument update_caller alone.
        """
        anag_utils.funcTrace()
        self.mapper.ScalarVisibilityOn()
        self.constant_color = None
        self.coloring_component = comp_name
        if update_caller and self.caller_updater:
            apply( self.caller_updater, () )


    def setOrientationalComponents( self, comp_list, update_caller=True ):
        """
        Indicate which data components should govern the orientation of the
        glyphs.

        Arg comp_list should be a list of component names, e.g.
        ['xmomentum','ymomentum','zmomentum'] (but with only 2 components if
        the data are 2D).

        Users should leave optional argument update_caller alone.
        """
        anag_utils.funcTrace()
        assert( type(comp_list) == type([]) )
        assert( len(comp_list) == self.local_vtk_data['c'].getNumDims() )
        self.orientational_components = comp_list
        if update_caller and self.caller_updater:
            apply( self.caller_updater, () )

    
    def setDecimationFactor( self, d, update_caller=True ):
        """
        If d==0, then we render as many glyphs as there are points in the input
        polydata (e.g. the number of vertices on the isosurface or embedded
        boundary).  Otherwise, we render only a fraction (1-10^{-d})*100%.
        The trouble is that the glyphs don't get thinned out uniformly over the
        surface.  They disappear more in relatively flat regions and less in
        relatively curved regions.  This reflects a property of the VTK class
        (vtkDecimatePro) we are using.

        This function does not work on stream glyphs, or on the glyphs of
        isocontours or 2-dimensional embedded boundaries.

        Users should leave optional argument update_caller alone.
        """
        anag_utils.funcTrace()
        assert( d >= 0.0 )
        self.decimation_factor = d
        if update_caller and self.caller_updater:
            apply( self.caller_updater, () )


    def setGlyphType( self, glyph_type, do_update=True ):
        """
        Legal arguments are 'cone' or 'arrow'.
        By default, we use 'cone' with 3D datasets and 'arrow' for 2D datasets.
        """
        anag_utils.funcTrace()
        assert( glyph_type==g_glyph_types.cone
            or  glyph_type==g_glyph_types.arrow )
        self.glyph_type = glyph_type

        if   glyph_type == g_glyph_types.cone:
            self.glyph_source = vtkpython.vtkConeSource()
            self.glyph = vtkpython.vtkGlyph3D()
        elif glyph_type == g_glyph_types.arrow:
            self.glyph_source = vtkpython.vtkGlyphSource2D()
            self.glyph_source.SetGlyphTypeToArrow()
            self.glyph = vtkpython.vtkGlyph2D()

        self.glyph.ScalingOn()
        domain = self.local_vtk_data['c'].getDomainExtentsXYZ()
        min_dim = min( domain[3]-domain[0], domain[4]-domain[1] )
        if self.local_vtk_data['c'].getNumDims() == 3:
            min_dim = min( min_dim, domain[5]-domain[2] )
        self.base_scale_factor = min_dim / 20.0
        self.glyph.SetSource( self.glyph_source.GetOutput() )
        self.glyph.SetVectorModeToUseVector()
        self.glyph.SetScaleModeToScaleByScalar()
        self.glyph.SetScaleModeToDataScalingOff()
        self.glyph.SetColorModeToColorByScalar()

        self.accum.SetInput( self.glyph.GetOutput() )

        if do_update and self.caller_updater:
            apply( self.caller_updater, () )


class _GlyphTypes:
    cone = 'cone'
    arrow = 'arrow'
g_glyph_types = _GlyphTypes()
