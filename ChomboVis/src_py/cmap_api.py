import anag_utils
from self_control import SelfControl

class CmapApi( SelfControl ):
#Cut to here
    """
    Color mappings corresponding to the data to be displayed.
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
        """ Pop up the colormap control panel. """
        anag_utils.apiTrace()
        self.control_cmap.showGUI()

    def withdrawGUI( self ):
        """ Hide the colormap control panel. """
        anag_utils.apiTrace()
        self.control_cmap.withdrawGUI()


    def showLegend( self ):
        """
        Draw a bar showing what values the various colors correspond to.
        """
        anag_utils.funcTrace()
        self.vtk_cmap.setLegendIsVisible( 1 )


    def hideLegend( self ):
        """
        Draw a bar showing what values the various colors correspond to.
        """
        anag_utils.funcTrace()
        self.vtk_cmap.setLegendIsVisible( 0 )

    def setLegendWidth( self, w ):
        """
        As a fraction of the renderwindow's width.  Doesn't do anything unless
        you've called self.showLegend().
        """
        anag_utils.apiTrace()
        self.vtk_cmap.setBarActorWidth( w )


    def setActiveColormap( self, tag ):
        """
        Legal values for arg tag are 'Default greyscale', 'Default color', 
        'User file' and 'User generated'.  If 'User_file', then
        loadUserColormap() needs to have been called first.
        """
        anag_utils.funcTrace()
        self.vtk_cmap.switchColormap( tag )


    def loadUserColormap( self, filename ):
        """
        Following this, it's legal to call setActiveColormap( 'User file' ).
        See color.cmap for an example of the correct format.
        """
        anag_utils.funcTrace()
        self.vtk_cmap.user_colormap =\
            self.vtk_cmap.loadColormapFromFile( filename )

    
    def getCmappedRangeMin( self ):
        """
        Returns the value, of the current component's range, that corresponds to
        the lowest value in the current colormap.
        """
        anag_utils.funcTrace()
        result = self.vtk_data.getCmappedRangeMin()
        return result


    def setCmappedRangeMin( self, x ):
        """
        Sets the value, of the current component's range, that corresponds to
        the lowest value in the current colormap.
        """
        anag_utils.funcTrace()
        self.vtk_data.setCmappedRangeMin(x)


    def getCmappedRangeMax( self ):
        """
        Returns the value, of the current component's range, that corresponds to
        the highest value in the current colormap.
        """
        anag_utils.funcTrace()
        result = self.vtk_data.getCmappedRangeMax()
        return result


    def setCmappedRangeMax( self, x ):
        """
        Sets the value, of the current component's range, that corresponds to
        the highest value in the current colormap.
        """
        anag_utils.funcTrace()
        self.vtk_data.setCmappedRangeMax(x)


    def getBackgroundColor( self ):
        """ 
        Returns an (r,g,b) tuple.  Component values range over [0.0,1.0].
        """
        anag_utils.funcTrace()
        result = self.vtk_cmap.getBackgroundColor()
        return result


    def setBackgroundColor( self, rgb ):
        """
        Arg rgb needs to be a tuple, with components ranging over [0.0,1.0].
        """
        anag_utils.funcTrace()
        self.vtk_cmap.setBackgroundColor( rgb )


    def setBlackOutliers( self, one_or_zero ):
        """
        Color cell values beyond the colormap range a special color (black,
        by default, or whatever was set from setOutlierColors()).
        """
        anag_utils.funcTrace()
        self.vtk_cmap.setBlackOutliers( one_or_zero )


    def setOutlierColors( self, lo_hi ):
        """
        Set the colors for the low- and high-end outliers (which will show if
        you've made the call "setBlackOutliers(1)").
        Argument lo_hi is a 2-tuple (lo & hi) of 4-tuples (colors in rgba form).
        Here's an example that sets the low end to almost-transparent white and
        the high end to almost-transparent black:
        >>> c.cmap.setOutlierColors( ((1,1,1,0.2), (0,0,0,0.2)) )
        """
        anag_utils.funcTrace()
        self.vtk_cmap.setOutlierColors( lo_hi )        

