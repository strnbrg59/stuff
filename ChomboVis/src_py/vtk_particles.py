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

# File: vtk_particles.py
# Author: TDSternberg
# Created: 3/21/03

import anag_utils
from self_control import SelfControl

import random
import vtkpython
import libVTKChomboPython

g_light_blue = (0.5, 0.5, 1.0 )
g_decls = anag_utils.Declarations('g_decls', 'g_distinguished_component_names',
    'g_marker_types', 'g_light_blue', 'g_filters', 'g_filter_types',
    'g_giant_val' )

class VtkParticles( SelfControl ):

    def __init__(self, dep_dict):
        anag_utils.funcTrace()

        instance_data = [
            { 'name':'marker_type', 'initval':'nothing', 'get':1, 'set':2,
                'save':1 },
            { 'name':'local_vtk_data', 'get':1 },
            { 'name':'particles_source'},
            { 'name':'actor', 'get':1 },
            { 'name':'mapper' },
            { 'name':'prop' },
            { 'name':'depthsorter'},
            { 'name':'circle_source' },
            { 'name':'sphere_source' },
            { 'name':'cone_source' },
            { 'name':'glyph3d_resolution', 'set':2, 'initval':12,
                'save':1},
            { 'name':'glyph' },
            { 'name':'decimation_factor', 'get':1, 'set':2, 'save':1,
                'initval':1},
            { 'name':'marker_size', 'get':1, 'set':2, 'save':1 },
            { 'name':'marker_constant_color', 'get':1, 'set':2, 'save':1 },
            { 'name':'glyph_scale_mode', 'get':1, 'set':2, 'save':1 },
            { 'name':'glyph_scaling_component', 'get':1, 'set':2, 'save':1 },
            { 'name':'reasonable_marker_sizes', 'get':1, 'initval':{}},
            { 'name':'component_names', 'get':1, 'initval':[] },
            { 'name':'positional_components', 'get':1, 'initval':{},'save':1},
            { 'name':'glyph_orientation_components', 'get':1, 'initval':{},
                'save':1 },
            { 'name':'do_orient_glyphs', 'get':1, 'set':2, 'initval':0,
                'save':1},
            { 'name':'needs_update', 'get':1, 'set':1, 'initval':1 },
            { 'name':'filtering_component', 'get':1, 'set':2, 'save':1,
                'initval':{g_filters.ordinary:None, g_filters.offset:None} },
            { 'name':'filter_range', 'get':1, 'save':1,
                'initval':{g_filters.ordinary:{}, g_filters.offset:{}} },
            { 'name':'offset_filter_offset', 'get':1, 'set':2, 'initval':0,
                'save':1 },
            { 'name':'component_ranges', 'get':1, 'initval':{} },
            { 'name':'opacity', 'get':1, 'set':2, 'save':5, 'initval':1.0},
            { 'name':'hilite_actor'}
        ]

        SelfControl.__init__( self, dep_dict, instance_data)
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( 'decls', 'dep_dict', 'instance_data' )

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self', 'name', 'default_xyz',
            'default_mxmymz', 'default_vxvyvz', 'comps', 'num_components',
            'is_3d_mode', 'i', 'filter_type', 'comp' )

        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=1,
            follow_global_curcomponent=1,
            follow_global_padded_boxes=1,
            always_use_real_boxes=0,
            instance_identifier='particles' )

        if self.local_vtk_data.getNumParticles() == 0:
            return

        self._assembleComponentNames()
        for comp in self.component_names:
            self.component_ranges[comp] = self.getComponentRange( comp )

        # Initialize the C++ object that is our interface to the particles data.
        self.particles_source = libVTKChomboPython.vtkChomboParticles()
        self.particles_source.SetChomboReader( self.local_vtk_data.getReader() )

        #
        # Other initialization.
        #
        self.glyph_scale_mode = g_glyph_scale_modes.none

        for filter_type in g_filter_types:
            for name in self.component_names:
                self.filter_range[g_filters.ordinary][name] =\
                     [-g_giant_val, g_giant_val]
                self.filter_range[g_filters.offset][name] = [None,None]
            if g_distinguished_component_names.id in self.component_names:
                self.setFilter( filter_type = filter_type,
                                name = g_distinguished_component_names.id )
            elif g_distinguished_component_names.x in self.component_names:
                self.setFilter( filter_type = filter_type,
                                name = g_distinguished_component_names.x )
            else:
                anag_utils.fatal(
                    "Particles data must provide a component named",
                    g_distinguished_component_names.x )


        self.computeReasonableMarkerSizes()
        if   'mass' in self.component_names:
            self.setGlyphScalingComponent( 'mass' )
        elif 'charge' in self.component_names:
            self.setGlyphScalingComponent( 'charge' )
        else:
            self.setGlyphScalingComponent( self.component_names[0] )

        self.hilite_actor = self.vtk_vtk.newActor()

        #
        # Create pipelines
        #
        self.circle_source = vtkpython.vtkGlyphSource2D()
        self.circle_source.SetGlyphTypeToCircle()
        self.circle_source.SetScale(1.0)
        self.circle_source.SetFilled(0)

        self.sphere_source = vtkpython.vtkSphereSource()
        self.cone_source   = vtkpython.vtkConeSource()
        
        self.glyph = vtkpython.vtkGlyph3D()
        self.glyph.ScalingOn()
        self.glyph.SetScaleFactor( 0.2 )
        
        self.mapper = self.vtk_vtk.newMapper()
        self.mapper.SetLookupTable( self.vtk_cmap.buildDefaultColormap() )
        
        self.prop = self.vtk_vtk.newProperty()
        self.setMarkerConstantColor((1,1,1))
        self.prop.SetPointSize(12)

        # Without this vtkDepthSortPolyData, the apparent opacity of the glyphs
        # is wrong.
        self.depthsorter = vtkpython.vtkDepthSortPolyData()
        self.depthsorter.SetDirectionToBackToFront()
        self.depthsorter.SetVector(1,1,1)
        self.depthsorter.SetCamera( self.vtk_cameras.getCamera() )
        self.depthsorter.SortScalarsOn()
        
        self.actor = self.vtk_vtk.newActor()
        self.actor.SetMapper( self.mapper )
        self.actor.SetProperty( self.prop )


        default_xyz = [ g_distinguished_component_names.x,
                        g_distinguished_component_names.y ]
        default_vxvyvz = [ g_distinguished_component_names.vx,
                           g_distinguished_component_names.vy ]
        default_mxmymz = [ g_distinguished_component_names.mx,
                           g_distinguished_component_names.my ]
        is_3d_mode = (not self.local_vtk_data.is2DMode()) \
                     or  self.local_vtk_data.isResliceMode()
        if( is_3d_mode ):
            default_xyz = default_xyz + [g_distinguished_component_names.z,]
            default_vxvyvz = default_vxvyvz +\
                [g_distinguished_component_names.vz,]
            default_mxmymz = default_mxmymz +\
                [g_distinguished_component_names.mz,]

        if self.positional_components == {}:  # I.e. no state_file
            # Positional components are, by default, position_[xyz].
            self.setPositionalComponents( default_xyz )

            # Glyph orientation components are, by default, velocity_[xyz] if
            # present; else momentum_[xyz] if present, else constant=[0,0,1].
            # FIXME: we don't support "constant" components like that.
            if( (g_distinguished_component_names.vx
                in self.glyph_orientation_components.keys())
            and 
                (g_distinguished_component_names.vy
                in self.glyph_orientation_components.keys())
            and
                ((g_distinguished_component_names.vz
                  in self.glyph_orientation_components.keys())
                or (not is_3d_mode))):
                    self.setGlyphOrientationComponents( default_vxvyvz )

            elif( (g_distinguished_component_names.mx
                in self.glyph_orientation_components.keys())
            and 
                (g_distinguished_component_names.my
                in self.glyph_orientation_components.keys())
            and
                ((g_distinguished_component_names.mz
                 in self.glyph_orientation_components.keys())
                or (not is_3d_mode))):
                    self.setGlyphOrientationComponents( default_mxmymz )
    
            else:
                comps = []
                num_components = self.local_vtk_data.getNumParticles()
                for i in range(0, self.local_vtk_data.getNumDims()):
                    comps.append(
                        self.component_names[i%num_components])
                self.setGlyphOrientationComponents( comps )
                    
        else:
            self.setPositionalComponents( self.positional_components['x'],
                                          self.positional_components['y'],
                                          self.positional_components['z'] )


        decls.memberFunctionAudit( self )


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.local_vtk_data.getNumParticles() == 0:
            return

        self.setMarkerSize( self.marker_size )
        self.setOpacity( self.opacity )
        self.setMarkerConstantColor( self.marker_constant_color )
        if self.local_vtk_data.getNumDims() == 2:
            self.setPositionalComponents( (self.positional_components['x'],
                                           self.positional_components['y']) )
        else:
            self.setPositionalComponents( (self.positional_components['x'],
                                           self.positional_components['y'],
                                           self.positional_components['z']) )

        for filter_type in g_filter_types:
            saved_comp = self.getFilteringComponent([filter_type,])
            for comp in self.component_names:
                if self.filter_range[filter_type][comp][0] != None:
                    self.setFilter( filter_type, comp,
                        self.filter_range[filter_type][comp][0],
                        self.filter_range[filter_type][comp][1] )
            self.setFilteringComponent( filter_type, saved_comp) 
        self.setOffsetFilterOffset( self.offset_filter_offset )

        self.setMarkerType( self.marker_type )
        self.setGlyphScaleMode( self.getGlyphScaleMode(), force=1 )
        if( self.glyph_scaling_component != None ):
            self.setGlyphScalingComponent( self.glyph_scaling_component,
                                           force=1 )
        if self.glyph_orientation_components != {}:
            if self.local_vtk_data.getNumDims() == 2:
                self.setGlyphOrientationComponents((
                    self.glyph_orientation_components['x'],
                    self.glyph_orientation_components['y']))
            else:
                self.setGlyphOrientationComponents((
                    self.glyph_orientation_components['x'],
                    self.glyph_orientation_components['y'],
                    self.glyph_orientation_components['z'] ))
        self.setDecimationFactor( self.decimation_factor, force=1 )

        self.setGlyph3dResolution( self.glyph3d_resolution )

        self.registerCallback(
            self.local_vtk_data.getNotifierResliceDirection(),
            self.forceUpdate )

        self.updateVis()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unhighlightParticle()
        self.makeVisible( 0 )


    def highlightParticle( self, particle_num ):
        anag_utils.funcTrace()
        self.vtk_vtk.removeActor( self.hilite_actor )
        hilite_sphere = vtkpython.vtkSphereSource()
        if self.glyph_scale_mode == g_glyph_scale_modes.colorAndScale:
            radius = 0.75 * self.glyph.GetScaleFactor() *\
                self.particles_source.GetScalingScalarValue( particle_num )
        else:
            radius = 0.75 * self.glyph.GetScaleFactor()

        hilite_sphere.SetRadius( radius )
        pos = self.particles_source.GetParticleXYZCoordinates( particle_num )
        anag_utils.info( "particle position=", pos )
        data_centering = self.local_vtk_data.getDataCentering()
        dx0 = self.local_vtk_data.getLevelDx(0)
        origin = self.local_vtk_data.getCoordinateOrigin()
        hilite_sphere.SetCenter( pos[0] + dx0[0]*data_centering[0]/2,
                                 pos[1] + dx0[1]*data_centering[1]/2,
                                 pos[2] + dx0[2]*data_centering[2]/2 )
 

        hilite_prop = self.vtk_vtk.newProperty()
        apply( hilite_prop.SetColor, g_light_blue )
        hilite_prop.SetOpacity(0.3)
        hilite_mapper = self.vtk_vtk.newMapper()
        hilite_mapper.SetInput( hilite_sphere.GetOutput() )
        self.hilite_actor.SetMapper( hilite_mapper )
        self.hilite_actor.SetProperty( hilite_prop )
        self.vtk_vtk.addActor( self.hilite_actor )
        self.vtk_vtk.render()


    def unhighlightParticle( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.removeActor( self.hilite_actor )


    def getSelectedParticleNumber( self, n ):
        """
        Arg n is a particle's ordinal number among those particles that made it
        past our filters, i.e. were passed on to VTK for rendering.  The return
        value is that particle's ordinal number among the entire set of
        particles.
        """
        anag_utils.funcTrace()
        return self.particles_source.GetSelectedParticleNumber( n )


    def setMarkerConstantColor( self, rgb ):
        anag_utils.funcTrace()
        self.marker_constant_color = rgb
        self.prop.SetColor( rgb[0], rgb[1], rgb[2] )
        self.vtk_vtk.render()


    def setOpacity( self, x ):
        """
        Set the opacity (between 0 and 1 inclusive) of the particles displayed.
        """
        anag_utils.funcTrace()
        self.prop.SetOpacity( x )
        self.opacity = x
        if(  (self.marker_type != g_marker_types.nothing)
        and  (self.marker_type != g_marker_types.points) ):
            self.needs_update=1
        self.updateVis()


    def setGlyphScaleMode( self, mode, force=0 ):
        anag_utils.funcTrace()
        assert( mode==g_glyph_scale_modes.none
        or      mode==g_glyph_scale_modes.colorOnly
        or      mode==g_glyph_scale_modes.colorAndScale )
        if mode != self.glyph_scale_mode  or  force==1:
            self.glyph_scale_mode = mode
            if mode == g_glyph_scale_modes.none:
                # Give color control back to self.prop:
                self.mapper.ScalarVisibilityOff()
                self.particles_source.SetDoScaleGlyphs(0)
            else:
                self.mapper.ScalarVisibilityOn()
                self.particles_source.SetDoScaleGlyphs( 1 )
                if mode == g_glyph_scale_modes.colorOnly:
                    self.glyph.SetScaleModeToDataScalingOff()
                if mode == g_glyph_scale_modes.colorAndScale:
                    self.glyph.SetScaleModeToScaleByScalar()
            self.forceUpdate(None,None)


    def setGlyphScalingComponent( self, name, force=0 ):
        """
        Arg name is the name of a component.  The glyphs get scaled and/or
        colorcoded (see self.setGlyphScaleMode()) by that component's values.
        """
        anag_utils.funcTrace()
        assert( name != '' )
        if( ( name == self.glyph_scaling_component )
        and ( force != 1 ) ):
            return
        self.glyph_scaling_component = name;
        self.particles_source.SetGlyphScalingComponent( name )
        self.forceUpdate(None,None)
        self.forceUpdate(None,None)


    def setDecimationFactor( self, n, force=0 ):
        """
        Display only every n-th particle that would otherwise be displayed.
        (Thus, if we also filter by a component's values, we display only
        every n-th particle that makes it past that filter.)
        Arg n should be an int.
        """
        anag_utils.funcTrace()
        if n!=self.decimation_factor  or  force==1:
            self.decimation_factor = n
            self.particles_source.SetDecimationFactor( n )
            self.forceUpdate(None,None)


    def getPointsPerGlyph( self ):
        anag_utils.funcTrace()

        # Number of points per glyph times number of visible glyphs:
        num_glyph_points = self.glyph.GetOutput().GetNumberOfPoints()
        num_points = self.particles_source.GetNumberOfSelectedParticles()
        anag_utils.info( "num_glyph_points=",num_glyph_points,
                         "num_points=", num_points )
        return num_glyph_points/num_points

    def getCellsPerGlyph( self ):
        anag_utils.funcTrace()

        # Number of cells per glyph times number of visible glyphs:
        num_glyph_cells = self.glyph.GetOutput().GetNumberOfCells()
        num_glyphs = self.particles_source.GetNumberOfSelectedParticles()
        anag_utils.info( "num_glyph_cells=",num_glyph_cells,
                         "num_glyphs=", num_glyphs )
        return num_glyph_cells/num_glyphs
        

    def forceUpdate( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self.needs_update = 1
        self.updateVis()        


    def computeReasonableMarkerSizes( self ):
        """
        Come up with a reasonable range for the glyph-size widget in
        control_particles, and a reasonable initial value of the glyph size.
        Sets self.reasonable_marker_sizes, a dictionary whose keys are 'min',
        'init' and 'max'.
        """
        anag_utils.funcTrace()

        domain_size_l_inf = max(
            ( self.local_vtk_data.getDomainMax()['x']
               - self.local_vtk_data.getDomainMin()['x'] ),
            ( self.local_vtk_data.getDomainMax()['y']
               - self.local_vtk_data.getDomainMin()['y'] ),
            ( self.local_vtk_data.getDomainMax()['z']
               - self.local_vtk_data.getDomainMin()['z'] ) )
        self.reasonable_marker_sizes['min'] = domain_size_l_inf/100.0        
        self.reasonable_marker_sizes['init'] = domain_size_l_inf/20.0
        self.reasonable_marker_sizes['max'] = domain_size_l_inf/2.0
        if not self.marker_size:
            self.marker_size = self.reasonable_marker_sizes['init']


    def getComponentRange( self, componentName ):
        anag_utils.funcTrace()
        lo = self.local_vtk_data.getParticleComponentRange( componentName )[0]
        hi = self.local_vtk_data.getParticleComponentRange( componentName )[1]
        return (lo,hi)


    def setFilter( self, filter_type, name=None, lo=None, hi=None ):
        """
        For arg filter_type==g_filters.ordinary, limit rendering to those
        particles whose values, on the arg name component, lie on the closed
        interval [lo,hi].

        For arg filter_type==g_filters.offset, offset the apparent position of
        particles whose values, on the arg name component, lie on the closed
        interval [lo,hi].
    
        If caller does not provide name, lo or hi, we use what's cached.

        We set name, lo and hi all in the same function because we want to call
        self.updateVis() but we want to economize on the number of calls to it.

        If lo==None or hi==None and has never been set, we set them to the
        component's range.
        """
        anag_utils.funcTrace()

        assert(  (self.getFilteringComponent([filter_type,])!="")
              or (name!=None) )

        # Return immediately if args imply no change.
        if( (name == self.getFilteringComponent([filter_type,]))
        and (lo == self.filter_range[filter_type][name][0])
        and (hi == self.filter_range[filter_type][name][1]) ):
            return                                      # Early return

        # Initialize (lo,hi) first time we visit a component.
        # self.component_ranges[name] gets set once and for all, but
        # self.filter_range[filter_type][name] is subject to user control
        # (and might be set because it's 'save':1.
        if [lo,hi] == [None,None]:
            if self.filter_range[filter_type][name] == [None,None]:
                span = [self.component_ranges[name][0],
                       self.component_ranges[name][1]]
                self.filter_range[filter_type][name] = span
            lo,hi = self.filter_range[filter_type][name]

        # Use arg or its cached counterpart as arg==None.
        if name!=None:
            self.setFilteringComponent( filter_type, name )
        else:
            name = self.getFilteringComponent([filter_type,])
        if lo!=None:
            self.filter_range[filter_type][name][0] = lo
        else:
            lo = self.filter_range[filter_type][name][0]
        if hi!=None:
            self.filter_range[filter_type][name][1] = hi
        else:
            hi = self.filter_range[filter_type][name][1]
        assert( lo != None )
        assert( hi != None )
        if filter_type == g_filters.ordinary:
            self.particles_source.SetFilterMinMax( lo, hi )
        else:
            self.particles_source.SetOffsetFilterMinMax( lo, hi )

        self.needs_update = 1
        self.updateVis()        


    def setFilteringComponent( self, filter_type, name ):
        """
        See setFilter().
        """
        anag_utils.funcTrace()
        self.filtering_component[filter_type] = name
        cpp_name = name
        if name == None:
            cpp_name = ""
        if filter_type == g_filters.ordinary:
            self.particles_source.SetFilteringComponent( cpp_name )
        else:
            self.particles_source.SetOffsetFilteringComponent( cpp_name )
        self.forceUpdate(None,None)


    def setOffsetFilterOffset( self, x ):
        anag_utils.funcTrace()
        self.offset_filter_offset = x
        self.particles_source.SetOffsetFilterOffset( x )
        self.forceUpdate(None,None)

    def setPositionalComponents( self, xyz ):
        """
        Arg xyz is a tuple of the names of those components to use for plotting
        particles' positions.  In 2d mode, no need for z.
        """
        anag_utils.funcTrace()

        if( xyz[0] != self.particles_source.GetXComponent()
        or  xyz[1] != self.particles_source.GetYComponent()
        or  ( (self.local_vtk_data.getNumDims()==2)
            or xyz[2] != self.particles_source.GetZComponent()) ):
            self.particles_source.SetXComponent( xyz[0] )
            self.particles_source.SetYComponent( xyz[1] )
            self.positional_components['x'] = xyz[0]
            self.positional_components['y'] = xyz[1]

            if self.local_vtk_data.getNumDims() == 3:
                self.particles_source.SetZComponent( xyz[2] )
                self.positional_components['z'] = xyz[2]

            self.needs_update = 1
            self.updateVis()

    def setPositionalComponent( self, value, axis ):
        """
        Set the positional component for the named axis.
        """
        anag_utils.funcTrace()
        self.positional_components[axis] = value
        if self.local_vtk_data.getNumDims() == 2:
            self.setPositionalComponents( (self.positional_components['x'],
                                           self.positional_components['y']) )
        else:
            self.setPositionalComponents( (self.positional_components['x'],
                                           self.positional_components['y'],
                                           self.positional_components['z']) )


    def getPositionalComponent( self, axis ):
        anag_utils.funcTrace()
        return self.positional_components[axis]


    def setGlyphOrientationComponents( self, xyz ):
        """
        Arg xyz is a tuple of the names of those components to use for orienting
        the glyphs.  In 2D mode, no need to supply z component (and is ignored
        if you do).
        """
        anag_utils.funcTrace()
        num_dims = self.local_vtk_data.getNumDims()
        if xyz == {}: # That's its initial value.
            return
        if( xyz[0] != self.particles_source.GetXGlyphOrientationComponent()
        or  xyz[1] != self.particles_source.GetYGlyphOrientationComponent()
        or  (  (num_dims==2)
            or (xyz[2]!=self.particles_source.GetZGlyphOrientationComponent()))
        ):
            self.particles_source.SetXGlyphOrientationComponent( xyz[0] )
            self.particles_source.SetYGlyphOrientationComponent( xyz[1] )
            self.glyph_orientation_components['x'] = xyz[0]
            self.glyph_orientation_components['y'] = xyz[1]
            if num_dims==3:
                self.particles_source.SetZGlyphOrientationComponent( xyz[2] )
                self.glyph_orientation_components['z'] = xyz[2]

            self.glyph.SetVectorModeToUseVector()
            self.setDoOrientGlyphs( 1 )
    
    def setGlyphOrientationComponent( self, name, direction ):
        """
        Like self.setGlyphOrientationComponents() but sets a single component.
        """
        anag_utils.funcTrace()
        assert( direction=='x' or direction=='y' or direction=='z' )
        vals = { 'x':self.particles_source.GetXGlyphOrientationComponent(),
                 'y':self.particles_source.GetYGlyphOrientationComponent(),
                 'z':self.particles_source.GetZGlyphOrientationComponent() }
        vals[direction] = name
        self.setGlyphOrientationComponents( (vals['x'],vals['y'],vals['z']) )


    def setDoOrientGlyphs( self, yes_no ):
        anag_utils.funcTrace()
        assert( yes_no == 0  or  yes_no == 1 )
        if( (yes_no == 1)
        and (self.particles_source.GetXGlyphOrientationComponent() == '') ):
            anag_utils.Error( "Call setGlyphOrientationComponents() first." )
            return
        self.do_orient_glyphs = yes_no
        self.particles_source.SetDoOrientGlyphs( yes_no )
        self.needs_update = 1
        self.updateVis()


    def getComponentSerialNum( self, name ):
        """
        Components are in alphabetical order.
        """
        anag_utils.funcTrace()
        return self.component_names.index( name )


    def setMarkerType( self, radiobutton_tag ):
        """
        Choices are the members of class _MarkerType ('nothing', 'point',
        'sphere' or 'cone' at this writing).
        """
        anag_utils.funcTrace()
        self.marker_type = radiobutton_tag
        if   radiobutton_tag == g_marker_types.nothing:
            self.makeVisible( 0 )
        elif radiobutton_tag == g_marker_types.points:
            self.particles_source.SetMarkersArePoints(1)
            self.setMarkerSize( self.marker_size )
            self.needs_update = 1
            self.makeVisible( 1 )
        else:
            self.particles_source.SetMarkersArePoints(0)
            self.setMarkerSize( self.marker_size )
            self.needs_update = 1
            if   radiobutton_tag == g_marker_types.spheres:
                self.glyph.SetSource( self.sphere_source.GetOutput() )
            elif radiobutton_tag == g_marker_types.cones:
                self.glyph.SetSource( self.cone_source.GetOutput() )
            elif radiobutton_tag == g_marker_types.circles:
                self.circle_source.Update()
                self.glyph.SetSource( self.circle_source.GetOutput() )
            else:
                assert(0)
            self.makeVisible( 1 )
    

    def makeVisible( self, on_off ):
        assert( on_off == 0  or  on_off == 1 )
        anag_utils.funcTrace()
        if self.local_vtk_data.getNumParticles() == 0:
            return

        if on_off==0:
            self.vtk_vtk.removeActor( self.actor )
            self.vtk_vtk.render()
        else:
            self.vtk_vtk.addActor( self.actor )
            self.updateVis()
            self.vtk_vtk.render()


    def updateVis( self ):
        anag_utils.funcTrace()
        if self.needs_update == 1:
            if   self.marker_type == g_marker_types.nothing:
                return
            elif self.marker_type == g_marker_types.points:
                self.mapper.SetInput( self.particles_source.GetParticleOutput())
            elif( self.marker_type == g_marker_types.cones
              or  self.marker_type == g_marker_types.spheres
              or  self.marker_type == g_marker_types.circles):
                self.glyph.SetInput( self.particles_source.GetParticleOutput() )
                if self.opacity < 0.99:
                    self.depthsorter.SetInput( self.glyph.GetOutput() )
                    self.mapper.SetInput( self.depthsorter.GetOutput() )
                else:
                    self.mapper.SetInput( self.glyph.GetOutput() )

            self.mapper.SetScalarRange(
                self.particles_source.GetGlyphScalingComponentMin(),
                self.particles_source.GetGlyphScalingComponentMax() )
            self.setMarkerSize( self.marker_size )
            self.vtk_vtk.render()
            self.needs_update = 0


    def _assembleComponentNames( self ):
        """
        Sets self.component_names.  (It would be inconvenient to pass a
        collection of strings from the C++ layer.)
        """
        anag_utils.funcTrace()
        reader = self.local_vtk_data.getReader()
        for i in range( 0, reader.GetNumParticleComponents() ):
            name = reader.GetParticleComponentName( i )
            self.component_names.insert( i, name )


    def setMarkerSize( self, x ):
        anag_utils.funcTrace()
        self.marker_size = float(x)
        if   self.marker_type == g_marker_types.points:
            self.prop.SetPointSize( self.marker_size/
                                    self.reasonable_marker_sizes['min'] )
        elif( self.marker_type == g_marker_types.cones
          or  self.marker_type == g_marker_types.spheres
          or  self.marker_type == g_marker_types.circles):
            self.glyph.SetScaleFactor( self.marker_size )
        elif self.marker_type == g_marker_types.nothing:
            pass
        else:
            assert(0)
        self.vtk_vtk.render()


    def setDiffuse( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.prop:  # None if no file loaded
            self.prop.SetDiffuse( x )
            self.vtk_vtk.render()
    def setAmbient( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.prop:  # None if no file loaded
            self.prop.SetAmbient( x )
            self.vtk_vtk.render()
    def setSpecular( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.prop:  # None if no file loaded
            self.prop.SetSpecular( x )
            self.vtk_vtk.render()


    def setGlyph3dResolution( self, res ):
        """
        Theta and phiResolution for sphere, resolution for cone -- affects
        smoothness but at a cost in memory and computation.
        """
        anag_utils.funcTrace()
        self.glyph3d_resolution = res
        self.sphere_source.SetThetaResolution( res )
        self.sphere_source.SetPhiResolution( res )
        self.cone_source.SetResolution( res )
        self.vtk_vtk.render()


    def generateTestPoints( self ):
        """ Just for testing. Points will come from the ChomboReader. """
        anag_utils.funcTrace()
        points = vtkpython.vtkPoints()
        points.SetDataTypeToFloat()
        num_points = 100
        scale = 10
        for i in range(0,num_points):
            points.InsertPoint( i, [ random.random()*scale,
                                     random.random()*scale,
                                     random.random()*scale ] )

        dataset = vtkpython.vtkPolyData()
        dataset.SetPoints( points )
        return dataset


#
# Constants
#
class _DistinguishedComponentNames:
    """
    Make sure these stay in sync with their counterparts in utils/Consts.h!
    """
    x = 'position_x'
    y = 'position_y'
    z = 'position_z'
    id = 'particle_id'
    vx = 'velocity_x',
    vy = 'velocity_y',
    vz = 'velocity_z',
    mx = 'momentum_x',
    my = 'momentum_y',
    mz = 'momentum_z'

g_distinguished_component_names = _DistinguishedComponentNames()

class _MarkerTypes:
    cones = 'cones'
    spheres = 'spheres'
    circles = 'circles'
    points = 'points'
    nothing = 'nothing'
g_marker_types = _MarkerTypes()

class _GlyphScaleModes:
    none = 'constant color, no scale'
    colorOnly = 'color by component value'
    colorAndScale = 'color and scale by component value'
g_glyph_scale_modes = _GlyphScaleModes()


class _Filters:
    ordinary = 'ordinary'
    offset   = 'offset'
g_filters = _Filters()
g_filter_types = (g_filters.ordinary, g_filters.offset)

g_giant_val = 9E60
