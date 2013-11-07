import anag_utils
from self_control import SelfControl

class ClipApi( SelfControl ):
#Cut to here
    """
    Clipping plane
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
        """ Pop up the clipping control panel. """
        anag_utils.apiTrace()
        self.control_clip.showGUI()

    def withdrawGUI( self ):
        """ Hide the clipping control panel. """
        anag_utils.apiTrace()
        self.control_clip.withdrawGUI()


    def defineClipPlane( self, direction, position, altitude, azimuth ):
        """
        Define a position and orientation for the plane that determines the
        clipping; on one side of the plane we render, on the other side we do
        not render.
        """
        anag_utils.funcTrace()
        self.vtk_clip.defineClipPlane( direction=direction, position=position,
            angle0=altitude, angle1=azimuth )


    def setInsideOut( self, on_off ):
        """ Show complement of clipped data. """
        anag_utils.apiTrace()
        assert( on_off==0  or  on_off==1 )
        self.vtk_clip.setInsideOutIndicator( on_off )


    def togglePlaneVisibility( self, on_off ):
        """
        1 = show the clipping plane.  0 = don't show it.
        """
        anag_utils.apiTrace()
        assert( on_off==0  or  on_off==1 )
        self.vtk_clip.setVisiblePlaneIsVisible( on_off )
