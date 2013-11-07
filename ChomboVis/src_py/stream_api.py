import anag_utils
from self_control import SelfControl

class StreamApi( SelfControl ):
#Cut to here
    """ Streamlines -- paths of massless particles through vector fields """
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
        """ Pop up the streamlines control panel. """
        anag_utils.apiTrace()
        self.control_stream.showGUI()


    def withdrawGUI( self ):
        """ Hide the streamlines control panel. """
        anag_utils.apiTrace()
        self.control_stream.withdrawGUI()


    def setIntegrateForward( self, on_off ):
        """
        If arg on_off is 1, we integrate forward from the seed points.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setIntegrateForward( on_off )


    def setIntegrateBackward( self, on_off ):
        """
        If arg on_off is 1, we integrate backward from the seed points.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setIntegrateBackward( on_off )


    def setMaxPointsPerLine( self, n ):
        """
        Sets the maximum number of points along which to pursue the numerical
        integration.  Not to be confused with the number of seed points.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setMaxPointsPerLine( n )


    def setRelativeStepsize( self, x ):
        """
        Ratio of the integration step length taken, to the local cell spacing.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setRelativeStepsize( x )


    def setFieldThreshold( self, x ):
        """
        A point in the field gets displayed if at least one of its u, v or w
        components exceeds the field threshold.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setFieldThreshold( x )


    def setComponentMapU( self, component_name ):
        """
        We call the vector field's components U, V and W.  This function
        determines the component to be identified with the U component.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setComponentMap( component_name, 'u' )


    def setComponentMapV( self, component_name ):
        """
        We call the vector field's components U, V and W.  This function
        determines the component to be identified with the V component.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setComponentMap( component_name, 'v' )


    def setComponentMapW( self, component_name ):
        """
        We call the vector field's components U, V and W.  This function
        determines the component to be identified with the W component.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setComponentMap( component_name, 'w' )


    def setMaxLevel( self, n ):
        """
        Arg n is the greatest level of refinement, data from which is to be
        taken into account in calculating streamlines.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setMaxLevel( n )


    def setColor( self, rgb ):
        """
        Set the color of the streamlines.  Arg rgb is a tuple with components
        ranging over [0.0,1.0].
        """
        anag_utils.apiTrace()
        self.vtk_stream.setRgb( rgb )

    
    def setLineWidth( self, w ):
        """
        Set the width of the streamlines.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setLineWidth(w)


    def setDiffuse( self, x ):
        """
        Sets the Vtk diffuse lighting parameter.  Sets ambient to 1-x.
        Legal range for x is [0.0, 1.0].
        """
        self.vtk_stream.setDiffuse( x )
        self.vtk_stream.setAmbient( 1-x )


    def setSpecular( self, x ):
        """
        Sets the Vtk specular lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        self.vtk_stream.setSpecular( x )


    def setSeedSize( self, w ):
        """
        Set the size -- in pixels along the side -- of the squares that
        indicate the seeds' positions.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setSeedSize(w)


    def setAlgorithm( self, algo_name ):
        """
        Legal choices are 'Nothing', 'Fixed Step', 'Clipped Step', and
        'Hedgehog'.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setAlgorithm( algo_name, do_update=0 )


    def setSeeds( self, n_seeds=None, length=None,
                  xy_rotation=None, xz_rotation=None,
                  x_translation=None, y_translation=None, z_translation=None ):
        """
        Seeds are the points from which the numerical integrations begin.
        Arg n_seeds is the number of seeds.
        Arg length is the length of the rake as a fraction of the length of the
        domain box's longest diagonal.
        The other arguments set the seeds' position relative to their initial one,
        which has them laid down along the length of one edge of the bounding box
         -- the edge parallel to the x axis, at which y=z=0.
        """
        anag_utils.apiTrace()
        seeds = self.vtk_stream.lineSeedsFactory()

        if n_seeds:   seeds.populate( n_seeds )
        else:         seeds.populate( seeds.getNSeeds() )

        if length:    seeds.stretch( length )
        else:         seeds.stretch( seeds.getLength() )

        if not xy_rotation: xy_rotation = seeds.getPhi()
        if not xz_rotation: xz_rotation = seeds.getRho()
        seeds.rotate( phi=xy_rotation, rho=xz_rotation )

        if not x_translation: x_translation = seeds.getX()
        if not y_translation: y_translation = seeds.getY()
        if not z_translation: z_translation = seeds.getZ()
        seeds.translate( x=x_translation, y=y_translation, z=z_translation )

        self.vtk_stream.updateVis()


    def showGlyphs( self ):
        """
        Place little pyramids where the stream seed points are.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setShowGlyphs(1)

    def hideGlyphs( self ):
        """
        Undo effect of self.showGlyphs().
        """
        anag_utils.apiTrace()
        self.vtk_stream.setShowGlyphs(0)


    def getGlyphs( self ):
        """
        Return a VtkGlyphs object on which you can set various aspects of the
        glyph state -- setConstantColor(), setColoringComponent(), etc.
        """
        anag_utils.apiTrace()
        return self.vtk_stream.glyphs

    def showTubes( self ):
        """
        Draw tubes around the streamlines.  Use setLineWidth() to control
        their radius.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setTubesActive(1)

    def hideTubes( self ):
        """
        Hide the tubes drawn by showTubes().
        """
        anag_utils.apiTrace()
        self.vtk_stream.setTubesActive(0)


    def setTubeResolution( self, n ):
        """
        Cross-section of tube is a regular n-gon.  This function sets n.
        """
        anag_utils.apiTrace()
        self.vtk_stream.setTubeResolution(n)
