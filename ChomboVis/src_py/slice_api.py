import anag_utils
from self_control import SelfControl

class SliceApi( SelfControl ):
#Cut to here
    """
    Slicing planes.
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
        """ Pop up the slice control panel. """
        anag_utils.apiTrace()
        self.control_slice.showGUI()


    def withdrawGUI( self ):
        """ Hide the slice control panel. """
        anag_utils.apiTrace()
        self.control_slice.withdrawGUI()


    def getPosition( self, axis, uniqizer=0 ):
        """
        Legal values for arg axis are 'x','y' or (in 3D mode) 'z'.  Returns the
        position of the indicated slice.  The convention is that slices are
        named for the axes to which they are perpendicular.
        """
        anag_utils.apiTrace()
        result = self.vtk_slice.getSlicingPlanes()[axis+str(uniqizer)
                                                  ].getPlanePosition()
        return result


    def setPosition( self, x, axis=None, uniqizer=0 ):
        """
        Sets the position of the slice indicated by arg axis, to arg x.
        Legal values for arg axis are 'x','y' or 'z'.

        In 2D reslice mode, leave out the axis argument.

        The convention is that slices are named for the axes to which they are
        perpendicular.
        Arg uniqizer is for if you've created more than one plane along the
        indicated axis.  These planes are numbered 0, 1, 2, ....
        """
        anag_utils.apiTrace()
        if self.vtk_data.is2DMode():
            self.vtk_data.repositionSlice( self.vtk_data.getResliceDirection(),
                                           x )
        else:
            self.vtk_slice.getSlicingPlanes()[axis+str(uniqizer)].movePlane( x )


    def toggleVisibility( self, on_off, axis=None, uniqizer=0 ):
        """
        Displays, or doesn't, a slice as arg on_off is 1 or 0.
        In 3D mode, arg axis -- 'x','y' or 'z' -- indicates the slice (named
        for the axis to which it is perpendicular).
        In 2D mode, arg axis should be omitted.
        """
        anag_utils.apiTrace()
        if self.vtk_data.is2DMode():
            self.vtk_slice.getSlicingPlanes()['z0'
                                             ].toggleSliceVisibility(on_off)
        else:
            assert( axis != None )
            self.vtk_slice.getSlicingPlanes()[axis + str(uniqizer)
                                             ].toggleSliceVisibility( on_off )


    def setOffset( self, x ):
        """
        Sets the distance between slices representing data at different
        refinement levels.  Legal values for x are [-1.0,1.0].  The units are
        in terms of the size of the domain box.
        """
        anag_utils.apiTrace()
        self.vtk_slice.setOffset( x )


    def getOffset( self ):
        """
        See setOffset()
        """
        anag_utils.apiTrace()
        result = self.vtk_slice.getOffset()
        return result


    def newPlane( self, axis ):
        """
        Creates a new slicing plane, parallel to one of the three default ones.
        Returns this plane's unique id number -- what you need to pass to some
        of the other SliceAPI methods, if you want to manipulate this plane.
        """
        anag_utils.apiTrace()
        return self.vtk_slice.newPlane( axis )


    def setDiffuse( self, x ):
        """
        Sets the Vtk diffuse lighting parameter.  Sets ambient to 1-x.
        Legal range for x is [0.0, 1.0].
        """
        self.vtk_slice.setDiffuse( x )
        self.vtk_slice.setAmbient( 1-x )


    def setSpecular( self, x ):
        """
        Sets the Vtk specular lighting parameter.
        Legal range for x is [0.0, 1.0].
        """
        self.vtk_slice.setSpecular( x )


    def setUseGhostCells( self, on_off ):
        anag_utils.apiTrace()
        self.vtk_slice.setUseGhostCells( on_off )
