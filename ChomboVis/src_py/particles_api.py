import anag_utils
from self_control import SelfControl
import vtk_particles # for g_filters

class ParticlesApi( SelfControl ):
#Cut to here
    """ Particles rendering """
#Cut from here
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        instance_vars = []
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

    def showGUI( self ):
        """ Pop up the particles control panel. """
        anag_utils.apiTrace()
        self.control_particles.showGUI()


    def withdrawGUI( self ):
        """ Hide the particles control panel. """
        anag_utils.apiTrace()
        self.control_particles.withdrawGUI()


    def getNumParticles( self ):
        anag_utils.apiTrace()
        result = self.vtk_data.getNumParticles()
        return result


    def getNumParticleComponents( self ):
        anag_utils.apiTrace()
        result = self.vtk_data.getNumParticleComponents()
        return result


    def getComponentNames( self ):
        """ Particle components, that is. """
        anag_utils.apiTrace()
        result = self.vtk_particles.getComponentNames()
        return result


    def setMarkerType( self, marker_type ):
        """
        Legal choices for arg marker_type are "nothing", "points", "spheres" and
        "cones".
        """
        anag_utils.apiTrace()
        self.vtk_particles.setMarkerType( marker_type )


    def setMarkerSize( self, x ):
        """
        If marker type is 'points', x is interpreted as the number of pixels
        along the side of the (square) marker image.
        If marker type is a glyph ('sphere' or 'cone') x is passed to the
        glyph's SetScaleFactor() function.
        """
        anag_utils.apiTrace()
        self.vtk_particles.setMarkerSize( x )


    def setGlyphScaleAndColorMode( self, mode ):
        """
        Legal values are:
            'constant color, no scale'
            'color by component value'
            'color and scale by component value'
        Unfortunately, I don't yet know how to scale but not color; or how to
        scale by one component and color by another.
        """
        anag_utils.apiTrace()
        self.vtk_particles.setGlyphScaleMode( mode )


    def setDoScaleGlyphs( self, yes_no ):
        anag_utils.apiTrace()
        assert( yes_no == 0  or  yes_no == 1 )
        self.vtk_particles.setDoScaleGlyphs( yes_no )


    def setGlyphScalingComponent( self, name ):
        """
        Arg name is the name of a component.  The glyphs get scaled by that
        component's values.
        If name=='', then turn off glyph scaling.
        """
        anag_utils.apiTrace()
        self.vtk_particles.setGlyphScalingComponent( name )


    def setDoOrientGlyphs( self, yes_no ):
        anag_utils.apiTrace()        
        assert( yes_no == 0  or  yes_no == 1 )
        self.vtk_particles.setDoOrientGlyphs( yes_no )


    def setGlyphOrientationComponents( self, xyz ):
        """
        Arg xyz is a tuple of the names of those components to use for orienting
        the glyphs.  In 2D mode, no need to supply z component (and is ignored
        if you do).
        """
        anag_utils.apiTrace()
        self.vtk_particles.setGlyphOrientationComponents( xyz )


    def setMarkerConstantColor( self, rgb ):
        """
        Color all the markers with the tuple rgb.
        """
        anag_utils.apiTrace()
        self.vtk_particles.setMarkerConstantColor( rgb )


    def setFilteringComponent( self, name ):
        """
        Particles are rendered if the value of the "filtering component"
        lies within the min and max of the "filter range".
        """
        anag_utils.apiTrace()
        self.filtering_component = name
        self.vtk_particles.setFilteringComponent(
            vtk_particles.g_filters.ordinary, name )


    def setFilteringComponentMinAndMax( self, lo, hi ):
        """
        Particles are rendered if the value of the "filtering component"
        lies within the closed interval [lo,hi].
        Warning: don't call this function until after you've called
        setFilteringComponent().
        """
        anag_utils.apiTrace()
        self.vtk_particles.setFilter(vtk_particles.g_filters.ordinary,
                                     lo=lo, hi=hi )


    def setOffsetFilteringComponent( self, name ):
        """
        Particles are rendered if the value of the "filtering component"
        lies within the min and max of the "filter range".
        """
        anag_utils.apiTrace()
        self.filtering_component = name
        self.vtk_particles.setFilteringComponent(
            vtk_particles.g_filters.offset, name )


    def setOffsetFilteringComponentMinAndMax( self, lo, hi ):
        """
        Particles are rendered if the value of the "filtering component"
        lies within the closed interval [lo,hi].
        Warning: don't call this function until after you've called
        setOffsetFilteringComponent().
        """
        anag_utils.apiTrace()
        self.vtk_particles.setFilter( vtk_particles.g_filters.offset,
                                      lo=lo, hi=hi )


    def setPositionalComponents( self, xyz ):
        """
        Arg xyz is a tuple of the names of those components to use for plotting
        particles' positions.  In 2d mode, no need for z.
        """
        anag_utils.apiTrace()
        self.vtk_particles.setPositionalComponents( xyz )        


    def setOpacity( self, x ):
        """
        Set the opacity (between 0 and 1 inclusive) of the particles displayed.
        """
        anag_utils.apiTrace()
        self.vtk_particles.setOpacity( x )


    def setDiffuse( self, x ):
        """
        Sets the Vtk diffuse lighting parameter.  Sets ambient to 1-x.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.apiTrace()
        self.vtk_particles.setDiffuse( x )
        self.vtk_particles.setAmbient( 1-x )


    def setSpecular( self, x ):
        """
        Sets the Vtk specular lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.apiTrace()
        self.vtk_particles.setSpecular( x )


    def setGlyph3dResolution( self, res ):
        """
        Theta and phiResolution for sphere, resolution for cone -- affects
        smoothness but at a cost in memory and computation.
        Sorry, but there doesn't appear to be any easy way to change the
        resolution of the circle glyphs.
        """
        anag_utils.apiTrace()
        self.vtk_particles.setGlyph3dResolution( res )
