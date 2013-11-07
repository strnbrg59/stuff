import anag_utils
from self_control import SelfControl

class VolumeApi( SelfControl ):
#Cut to here
    """ Volume rendering """
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
        """ Pop up the volume-rendering control panel. """
        anag_utils.apiTrace()
        self.control_vol.showGUI()


    def withdrawGUI( self ):
        """ Hide the volume-rendering control panel. """
        anag_utils.apiTrace()
        self.control_vol.withdrawGUI()


    def setComponent( self, component_name ):
        """
        Component choice is independent of that in vtk_data.
        """
        anag_utils.apiTrace()
        self.vtk_vol.setComponent( component_name )


    def setMaxLevel( self, n ):
        """
        Arg n is the greatest level of refinement, data from which is to be
        taken into account in rendering volume.
        """
        anag_utils.apiTrace()
        self.vtk_vol.setMaxVisibleLevel( n )


    def setMinLevel( self, n ):
        """
        Arg n is the lowest level of refinement, data from which is to be
        taken into account in rendering volume.
        """
        anag_utils.apiTrace()
        self.vtk_vol.setMinVisibleLevel( n )


    def toggleVisibility( self, on_off ):
        """
        Display, or don't, the volume rendering, as arg on_off is 1 or 0.
        """
        anag_utils.apiTrace()
        self.vtk_vol.setIsVisible( on_off )


    def setXferPoints( self, xfer_points ):
        """
        Arg xfer_points is a list of pairs.  Each pair specifies a point on
        domain [0,255], range [0.0,1.0].  The points define a step function
        that defines opacity.  The step function should decrease monotonically.
        """
        anag_utils.apiTrace()
        self.vtk_vol.setXferPoints( xfer_points )


    def setIntegrationStepFactor( self, x ):
        """
        Factor applied to arg for vtkVolume::SetSampleDistance().  Default is
        1.0.  Use smaller numbers for higher resolution but correspondingly
        greater computational cost.
        """
        anag_utils.apiTrace()
        self.vtk_vol.setIntegrationStepFactor( x )
