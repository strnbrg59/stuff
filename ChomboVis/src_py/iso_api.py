import vtk
import anag_utils
import vtk_iso  # For some constants
from self_control import SelfControl

class IsoApi( SelfControl ):
#Cut to here
    """
    Isosurfaces and isocontours.
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
        """ Pop up the isosurfaces/contours control panel. """
        anag_utils.apiTrace()
        self.control_iso.showGUI()


    def withdrawGUI( self ):
        """ Hide the isosurfaces/contours control panel. """
        anag_utils.apiTrace()
        self.control_iso.showGUI()


    def toggleVisibility( self, on_off ):
        """
        Displays, or doesn't display, isosurfaces as arg on_off is 1 or 0.
        """
        anag_utils.apiTrace()
        self.vtk_iso.setDoShowIsosurfaces( on_off )

    def toggleClipping( self, on_off ):
        """ Turns clipping on and off.  See ClipAPI. """
        self.vtk_iso.setIsClipped(on_off)


    def setMin( self, x ):
        """
        Sets the value that defines the isosurface corresponding to the lowest
        value.  (See isoSetNum().)
    
        A separate value of this min value is stored for each component.  So
        if you change components after calling this method, you'll see the min
        value getting reset.
        """
        anag_utils.apiTrace()
        self.vtk_iso.setMinIso( x )
        self.vtk_iso.updateContours()


    def setMax( self, x ):
        """
        Sets the value that defines the isosurface corresponding to the highest
        value.  (See isoSetNum().)

        A separate value of this max value is stored for each component.  So
        if you change components after calling this method, you'll see the max
        value getting reset.
        """
        anag_utils.apiTrace()
        self.vtk_iso.setMaxIso( x )
        self.vtk_iso.updateContours()


    def setNum( self, n ):
        """
        Sets the number of isosurfaces (in 2D mode isocontours) displayed.

        A separate value of this num value is stored for each component.  So
        if you change components after calling this method, you'll see the num
        value getting reset.
        """
        anag_utils.apiTrace()
        self.vtk_iso.setNumIsos( n )
        self.vtk_iso.updateContours()


    def toggleShadeByValue( self, on_off):
        """
        When arg on_off is 1, the coloration of the isosurfaces reflects the
        value of the current component.  When arg on_off is 0, the color is
        whatever the user set with isoSetColor().
        """
        anag_utils.apiTrace()
        if on_off == 0:
            self.vtk_iso.changeColor( vtk_iso.g_constant_color )
        else:
            self.vtk_iso.changeColor( vtk_iso.g_self_color )


    def setConstantColor( self, rgb ):
        """
        Set the color of the isosurfaces to arg rgb, a triple whose elements
        range over [0.0,1.0].
        """
        anag_utils.apiTrace()
        self.vtk_iso.changeColor( vtk_iso.g_constant_color, rgb )


    def setCrossColoration( self, other_component_name ):
        """
        Color the isosurface with values of the indicated other component.
        """
        anag_utils.apiTrace()
        self.vtk_iso.changeColor( other_component_name )


    def setUseGhostCells( self, on_off ):
        """
        Without ghost cells, there are gaps at the box boundaries.
        """
        anag_utils.apiTrace()
        self.vtk_iso.setUseGhostCells( on_off )


    def setDiffuse( self, x ):
        """
        Sets the Vtk diffuse lighting parameter.  Sets ambient to 1-x.
        Legal range for x is [0.0, 1.0].
        """
        self.vtk_iso.setDiffuse( x )
        self.vtk_iso.setAmbient( 1-x )


    def setSpecular( self, x ):
        """
        Sets the Vtk specular lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        self.vtk_iso.setSpecular( x )


    def setLineWidth( self, w ):
        """
        Set the width of the isocontours (but not of the isosurfaces).
        """
        anag_utils.apiTrace()
        self.vtk_iso.setLineWidth(w)


    def toggleLOD( self, on_off ):
        """
        Use, or don't use, a level-of-detail actor (vtkLODActor).  By default,
        we do use it.  It has the effect of turning your isosurface into dots
        when moving it would occur with a refresh rate slower than about
        2/second.
        Arg on_off should be 0 or 1.
        """
        anag_utils.apiTrace()
        assert( on_off==0  or  on_off==1 )
        self.vtk_vtk.removeActor( self.vtk_iso.getIsoActor() )
        if on_off == 0:
            self.vtk_iso.setIsoActor( self.vtk_vtk.newActor() )
            self.vtk_iso.getIsoActor().SetProperty( self.vtk_iso.getIsoProp() )
            self.vtk_iso.getIsoActor().SetMapper( self.vtk_iso.getIsoMap() )
        else:
            self.vtk_iso.setIsoActor( vtk.vtkLODActor() )
            self.vtk_iso.getIsoActor().SetNumberOfCloudPoints(10000)
            self.vtk_iso.getIsoActor().SetProperty( self.vtk_iso.getIsoProp() )
            self.vtk_iso.getIsoActor().SetMapper( self.vtk_iso.getIsoMap() )
        self.vtk_vtk.addActor( self.vtk_iso.getIsoActor() )


    def setOpacity( self, opacity ):
        anag_utils.apiTrace()
        assert( opacity >= 0.0  and  opacity <= 1.0 )
        self.vtk_iso.setOpacity( opacity )


    def showGlyphs( self ):
        """
        Place little pyramids along the isosurface or isocontour.
        """
        anag_utils.apiTrace()
        self.vtk_iso.setShowGlyphs(1)

    def hideGlyphs( self ):
        """
        Undo effect of self.showGlyphs().
        """
        anag_utils.apiTrace()
        self.vtk_iso.setShowGlyphs(0)


    def getGlyphs( self ):
        """
        Return a VtkGlyphs object on which you can set various aspects of the
        glyph state -- setConstantColor(), setColoringComponent(), etc.
        """
        anag_utils.apiTrace()
        return self.vtk_iso.glyphs
