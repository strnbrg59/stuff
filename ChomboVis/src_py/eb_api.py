import vtk
import anag_utils
from self_control import SelfControl
from vtk_eb import g_eb_discriminator_name

g_decls = anag_utils.Declarations( 'g_decls', 'g_eb_discriminator_name' )

class EbApi( SelfControl ):
#Cut to here
    """
    Embedded boundaries.
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
        if not self.isOldStyle():
            self.vtk_eb = self.vtk_iso_eb

    def _refresh( self ):
        anag_utils.funcTrace()
    def cleanup( self ):
        anag_utils.funcTrace()

#Cut to here

    def isOldStyle( self ):
        """
        Return True iff hdf5 file contains component named 'fraction-0' and
        no iso_eb=1 on the command line..
        """
        anag_utils.apiTrace()
        return (    ('fraction-0' in self.vtk_data.getComponentList())
               and not self.cmd_line.getIsoEb() )


    def showGUI( self ):
        """ Pop up the embedded boundaries control panel. """
        anag_utils.apiTrace()
        self.control_eb.showGUI()


    def withdrawGUI( self ):
        """ Hide the embedded boundaries control panel. """
        anag_utils.apiTrace()
        self.control_eb.withdrawGUI()


    def setCapping( self, yes_no ):
        """
        0 => no capping in EB representation.
        1 => capping
        """
        anag_utils.apiTrace()
        assert( (yes_no == 0) or (yes_no == 1) )
        self.vtk_eb.setCapping( yes_no )


    def setNormalOutward( self, yes_no ):
        """
        0 => normal is inward (toward fluid -- EB Chombo convention)
        1 => outward (out of fluid)
        """
        anag_utils.apiTrace()
        assert( self.isOldStyle() )
        assert( (yes_no == 0) or (yes_no == 1) )
        self.vtk_eb.setNormalOutward( yes_no )


    def toggleVisibility( self, on_off ):
        """
        Display, or don't, an embedded boundary, as arg on_off is 1 or 0.
        """
        anag_utils.apiTrace()
        if self.isOldStyle() and (self.vtk_data.getReader().GetNumEBs() == 0):
            anag_utils.error( "Called EB method, but there are no"
            " embedded boundaries in data.")
        else:
            self.vtk_eb.setIsVisible( on_off )


    def toggleClipping( self, on_off ):
        """ Turns clipping on and off.  See ClipAPI. """
        anag_utils.apiTrace()
        if not self.isOldStyle():
            anag_utils.warning( "Clipping against orientable plane not yet "
                "available." )
        self.vtk_eb.setIsClipped(on_off)


    def toggleShadeByValue( self, on_off):
        """
        When arg on_off is 1, the coloration of the embedded boundary reflects
        the value of the current component.  When arg on_off is 0, the color is
        whatever the user set with isoSetColor().
        """
        anag_utils.apiTrace()
        self.vtk_eb.changeColor( not on_off )


    def setColor( self, rgb ):
        """
        Arg rgb is a tuple with components ranging over [0.0,1.0].
        """
        anag_utils.apiTrace()
        self.vtk_eb.changeColor( 1, rgb )


    def setLineWidth( self, w ):
        """
        Set the width of the EB when in 2D mode.
        """
        anag_utils.apiTrace()
        if self.vtk_data.is2DMode():
            self.vtk_eb.setLineWidth(w)

    def setUseGhostCells( self, yes_no ):
        """
        If arg is 1, then use (if necessary generated) ghost cells to render the
        embedded boundary.
        """
        anag_utils.apiTrace()
        assert( yes_no==0   or  yes_no==1 )
        assert( not self.isOldStyle() )
        self.vtk_eb.setUseGhostCells( yes_no )
        if self.vtk_eb.getIsVisible():
            self.vtk_eb.updateVis()


    def setAlgo( self, algo ):
        """ Legal values for arg algo are 'Distance' and 'Volume fraction'. """
        anag_utils.apiTrace()
        self.vtk_eb.setAlgo( algo )


    def selectBoundary( self, boundary_num, on_off ):
        """
        Select whether the indicated embedded boundary is to be displayed or
        not.  Legal values for arg boundary_num are
        integers from 0 to one less than the number of boundaries in the data.
        See ReaderAPI.getNumEmbeddedBoundaries().
        """
        anag_utils.apiTrace()
        self.vtk_eb.selectBoundary( boundary_num, on_off )


    def toggleLOD( self, on_off ):
        """
        Use, or don't use, a level-of-detail actor (vtkLODActor).  By default,
        we do use it.  It has the effect of turning your EB into dots when
        moving it would occur with a refresh rate slower than about 2/second.
        Ignored, if using new-style EB.
        Arg on_off should be 0 or 1.
        """
        anag_utils.apiTrace()
        if not self.isOldStyle():
            return
        assert( on_off==0  or  on_off==1 )
        self.vtk_vtk.removeActor( self.vtk_eb.getActor() )
        if on_off == 0:
            self.vtk_eb.setActor( self.vtk_vtk.newActor() )
            self.vtk_eb.getActor().SetProperty( self.vtk_eb.getProp() )
            self.vtk_eb.getActor().SetMapper( self.vtk_eb.getMapper() )
        else:
            self.vtk_eb.setActor( vtk.vtkLODActor() )
            self.vtk_eb.getActor().SetProperty( self.vtk_eb.getProp() )
            self.vtk_eb.getActor().SetMapper( self.vtk_eb.getMapper() )
        self.vtk_vtk.addActor( self.vtk_eb.getActor() )        


    def setDiffuse( self, x ):
        """
        Sets the Vtk diffuse lighting parameter.  Sets ambient to 1-x.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.apiTrace()
        self.vtk_eb.setDiffuse( x )
        self.vtk_eb.setAmbient( 1-x )


    def setSpecular( self, x ):
        """
        Sets the Vtk specular lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        anag_utils.apiTrace()
        self.vtk_eb.setSpecular( x )


    def setLevelsetComponent( self, c ):
        """
        Sets the component whose 0 isocontour defines the embedded boundary.
        """
        anag_utils.apiTrace()
        assert( not self.isOldStyle() )
        self.vtk_eb.setLevelsetComponent(c)
        if self.vtk_eb.getIsVisible():
            self.vtk_eb.updateVis()


    def setClippingIsovalue( self, x ):
        """
        Should be zero if the levelset component is defined appropriately.
        """
        anag_utils.apiTrace()
        assert( not self.isOldStyle() )
        self.vtk_data.setIsocontourClipValue( x )
        if self.vtk_eb.getIsVisible():
            self.vtk_eb.updateVis()


    def setFluid2( self, c ):
        """
        Sets the component whose values are to be shown where the first fluid's
        covered cells are.  Won't be visible unless you call
        setBlankCoveredCells().
        """
        anag_utils.apiTrace()
        assert( not self.isOldStyle() )
        self.vtk_eb.setFluid2( c )
        if self.vtk_eb.getIsVisible():
            self.vtk_eb.updateVis()


    def setBlankCoveredCells( self, yes_no ):
        """
        If arg yes_no==1, then don't show covered cells.
        This means different things, depending on whether we're doing EB the old
        way (fraction-0, etc) or the new way (vtk_iso_eb).
        """
        anag_utils.apiTrace()
        self.vtk_eb.setBlankCoveredCells(yes_no)
        if self.vtk_eb.getIsVisible():
            self.vtk_eb.updateVis()


    def setOpacity( self, opacity ):
        anag_utils.apiTrace()
        assert( opacity >= 0.0  and  opacity <= 1.0 )
        self.vtk_eb.setOpacity( opacity )
        if self.vtk_eb.getIsVisible():
            self.vtk_eb.updateVis()


    def showGlyphs( self ):
        """
        Place little pyramids along the EB.
        """
        anag_utils.apiTrace()
        self.vtk_eb.setShowGlyphs(1)


    def hideGlyphs( self ):
        """
        Undo effect of self.showGlyphs().
        """
        anag_utils.apiTrace()
        self.vtk_eb.setShowGlyphs(0)


    def getGlyphs( self ):
        """
        Return a VtkGlyphs object on which you can set various aspects of the
        glyph state -- setConstantColor(), setColoringComponent(), etc.
        """
        anag_utils.apiTrace()
        return self.vtk_eb.glyphs


    def registerCoveredCellsDiscriminator( self, 
            component_name, fmin, fmax, eb_id=0 ):
        """
        Tells ChomboVis about a component whose values identify covered, regular
        and irregular cells.
        Covered cells are where the values are < fmin.
        Regular cells are where the values are > fmax.
        All other cells are irregular.
        Arg eb_id is for identifying which embedded boundary this is for, in the
        event there is more than one embedded boundary.

        Do not use __covered-n (n=0,1,...) for the component name.
        """
        anag_utils.apiTrace()
        assert( component_name.find( g_eb_discriminator_name ) == -1 )
        def xform(x, fmin, fmax):
            """
            For use in registerCoveredCellsDiscriminator -- pointers to local
            functions get lost.
            """
            if   x < fmin: return 0
            elif x > fmax: return 2
            else:          return 1
        xxform = lambda x : xform( x, fmin, fmax )
        self.__dict__[ g_eb_discriminator_name + str(eb_id) ] = xxform

        self.vtk_data.defineNewComponent( g_eb_discriminator_name + str(eb_id),
            xxform, (component_name,) )
