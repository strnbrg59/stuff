import anag_utils
from self_control import SelfControl

class GridApi( SelfControl ):
#Cut to here
    """
    Boxes at various levels of detail.
    """
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
        """ Pop up the grid control panel. """
        anag_utils.apiTrace()
        self.control_grid.showGUI()


    def withdrawGUI( self ):
        """ Hide the grid control panel. """
        anag_utils.apiTrace()
        self.control_grid.withdrawGUI()


    def setDetail( self, tag ):
        """
        Legal values for arg tag are 'Nothing', 'Bounding boxes',
        'Slice cells', 'Face cells', and 'All cells' when in 3D mode.
        In 2D mode, 'Slice cells' and 'Face cells' are not legal.
        """
        anag_utils.apiTrace()
        self.vtk_grid.chooseCellDetail( tag )


    def toggleSolidBoxes( self, on_off ):
        """
        Show (on_off=1) or hide (0) boxes as solid bodies.
        """
        anag_utils.apiTrace()
        self.vtk_grid.setShowSolidBoxes(1)


    def setSolidBoxOpacity( self, x ):
        """
        Arg x should fall within [0,1].  0 is totally transparent, 1 is totally
        opaque.
        """
        anag_utils.apiTrace()
        assert( 0.0 <= x <= 1.0 )
        self.vtk_grid.setOpacity(x)


    def setSolidBoxShrinkageFactor( self, x ):
        """
        Shrink the solid boxes by to a linear size equal to a multiple
        (1 - 10^x) of themselves.
        """
        anag_utils.apiTrace()
        assert( x < 0.0)
        self.vtk_grid.setSolidBoxShrinkageFactor( x )


    def toggleDomainBox( self, on_off ):
        """
        Displays, or doesn't display, the domain box as arg on_off is 1 or 0.
        """
        anag_utils.apiTrace()
        self.vtk_grid.showDomainBox( on_off )


    def toggleTickMarks( self, on_off ):
        """
        Displays, or doesn't display, the tick marks as arg on_off is 1 or 0.
        """
        anag_utils.funcTrace()
        self.vtk_grid.showTickMarks( on_off )
    

    def toggleClipping( self, on_off ):
        """ Turns clipping on and off.  See ClipAPI. """
        self.vtk_grid.setIsClipped(on_off)


    def setOutlineColor( self, rgb ):
        """
        Sets the color of the grids, domain box and tickmarks.
        Arg rgb needs to be a tuple with elements ranging on [0.0,1.0].
        """
        anag_utils.apiTrace()
        self.vtk_grid.setOutlineColor( rgb )

    # For backward compatibility:
    setColor = setOutlineColor

    def setSolidColor( self, rgb ):
        """
        Sets the color of the solid boxes.  Arg rgb needs to be a tuple
        with elements ranging on [0.0,1.0].
        """
        anag_utils.apiTrace()
        self.vtk_grid.setSolidColor( rgb )


    def setColorOutlinesByLevel( self, yes_no, prescribed_colors=None ):
        """
        If yes_no==0, then color all grids by the constant color.
        If yes_no==1, then:
            Color the grids different colors, by level.  If
            prescribed_colors==None (the default), the colors are evenly spaced
            across the default colormap.  Otherwise, the color for level i is
            set to prescribed_colors[i] (so if you set prescribed_colors, you'll
            need to make it a tuple with at least as many elements as there are
            levels in your dataset).

        Example:
            colors = ( (1,1,1), (0,0,0), (1,0,0) )
            c.grid.setColorOutlinesByLevel(1, colors)
        """
        anag_utils.apiTrace()
        self.vtk_grid.setColorOutlinesByLevel( yes_no )
        if prescribed_colors != None:
            self.vtk_grid.setPrescribedOutlineLevelColors( prescribed_colors )


    def setColorSolidsByLevel( self, yes_no, prescribed_colors=None ):
        """
        See setColorOutlinesByLevel(); this is like that, but for solid boxes.
        """
        anag_utils.apiTrace()
        self.vtk_grid.setColorSolidsByLevel( yes_no )
        if prescribed_colors != None:
            self.vtk_grid.setPrescribedSolidLevelColors( prescribed_colors )


    def setLineWidth( self, w ):
        """ Set grid lines to w pixels wide. """
        anag_utils.funcTrace()
        self.vtk_grid.setLineWidth( w )


    def setUseGhostCells( self, on_off ):
        anag_utils.funcTrace()
        self.vtk_grid.setUseGhostCells( on_off )
        self.vtk_vtk.render()


    def setDiffuse( self, x ):
        """
        Sets the Vtk diffuse lighting parameter.  Sets ambient to 1-x.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.apiTrace()
        self.vtk_grid.setDiffuse( x )
        self.vtk_grid.setAmbient( 1-x )


    def setSpecular( self, x ):
        """
        Sets the Vtk specular lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.apiTrace()
        self.vtk_grid.setSpecular( x )
