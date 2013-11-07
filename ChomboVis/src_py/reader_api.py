import anag_utils
import self_control
from self_control import SelfControl
import os

class ReaderApi( SelfControl ):
#Cut to here
    """
    Domains and ranges of the data to be displayed.  Control over the level of
    detail.
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
        """ Pop up the data control panel. """
        anag_utils.apiTrace()
        self.control_data.showGUI()

    def withdrawGUI( self ):
        """ Hide the data control panel. """
        anag_utils.apiTrace()
        self.control_data.withdrawGUI()


    def loadHDF5( self, infilename, preserve_state=None ):
        """
        Load a new hdf5 file into ChomboVis.

        If arg preserve_state!=None, then try to load the new file in the same
        state as the current file.  Will fail if old state mentions a level or
        box or component etc that doesn't exist in the new file.
        """
        anag_utils.apiTrace()

        if preserve_state:
            (temp_statefile_name, dummy) =\
                anag_utils.g_temp_file_mgr.makeTempFile(
                    'chombovis', 'state', create=0 )
            self.saved_states.saveState( temp_statefile_name )

        self_control.resetMultiInstanceQualifier()

        infilename = os.path.expanduser(infilename) # In case there's a '~'.
        if preserve_state:
            self.vtk_vtk.setNewHDF5Notifier(
                infilename, extra_info=temp_statefile_name )
        else:
            self.vtk_vtk.setNewHDF5Notifier( infilename )


    def saveToHDF5( self, outfilename, axis=None, axis_position=None ):
        """
        Dump the current dataset to an hdf5 file.
        Use optional args axis and axis_position if you want to save a 2D
        slice.
        """
        anag_utils.apiTrace()
        if axis == None:
            self.vtk_data.saveToHDF5( outfilename )
        else:
            # Don't try this unless the dataset is 3D (or 3D resliced):
            assert( self.vtk_data.isResliceMode() or
                    (not self.vtk_data.is2DMode()) )
            sliced_visdat = self.getVisualizableDataset().slice(axis,
                                                                axis_position)
            sliced_visdat.saveToHDF5( outfilename )


    def onNewHDF5( self, func ):
        """
        Arranges for function "func" to be invoked every time we load a new
        hdf5.  Useful in .chombovisrc.
        """
        anag_utils.apiTrace()
        self.vtk_vtk.getNotifierNewHDF5Notifier().addCallback(
            lambda d1, d2, func=func: func() )


    def aFileIsLoaded( self ):
        """ Returns 1 if a file is loaded, None otherwise """
        anag_utils.apiTrace()
        if self.saved_states.getNumHDF5sLoaded() == 0:
            return None
        else:
            return 1


    def getHDF5FileName( self ):
        """ Name of HDF5 file from which we loaded the current dataset. """
        anag_utils.apiTrace()
        result = self.vtk_data.getInfileName()
        return result


    def getVisualizableDataset( self ):
        """
        Returns a reference to the VisualizableDataset object currently loaded
        in the vis system.
        """
        anag_utils.apiTrace()
        return self.vtk_data.getVisualizableDataset()


    def getDataSummary( self ):
        """
        Range info covers all *visible* levels.
        The cheapest way to load all the levels is to turn off all displays
        (including slices).
        """
        anag_utils.apiTrace()
        result = self.vtk_data.getDataSummary()
        return result


    def getComponentNames( self ):
        """ Returns a list of the components in the loaded hdf5 file. """
        anag_utils.apiTrace()
        result = self.vtk_data.getComponentList()
        return result

    def getNumComponents( self ):
        anag_utils.apiTrace()
        result = len(self.vtk_data.getComponentList())
        return result
    # For backward compatibility:
    getNumberOfComponents = getNumComponents

    def getCurrentComponent( self ):
        """
        Returns the name of the component currently displayed, i.e. the one
        whose values would be reflected in the isosurfaces or slices, were we to
        turn those on now.
        """
        anag_utils.apiTrace()
        result = self.vtk_data.getCurComponent()
        return result

    def setCurrentComponent( self, name_or_number ):
        """
        Sets the current component to arg name_or_number (i.e. it can be either
        a component's name or its serial number).  See also getCurrentComponent.
        """
        anag_utils.apiTrace()
        if type(name_or_number)==type(2):
            name = self.vtk_data.getComponentList()[name_or_number]
        else:
            name = name_or_number
        self.vtk_data.setCurComponent( name )


    def getVisibleLevelMin( self ):
        """
        Returns the serial number of the coarsest displayed refinement level. 
        """
        anag_utils.apiTrace()
        result = self.vtk_data.getMinVisibleLevel()
        return result
    def setVisibleLevelMin( self, i ):
        """
        Sets the lowest (coarsest) displayed refinement level to arg i.
        """
        anag_utils.apiTrace()
        self.vtk_data.setMinVisibleLevel( i )
    def getVisibleLevelMax( self ):
        """
        Returns the serial number of the finest displayed refinement level. 
        """
        anag_utils.apiTrace()
        result = self.vtk_data.getMaxVisibleLevel()
        return result
    def setVisibleLevelMax( self, i ):
        """
        Sets the highest (finest) displayed refinement level to arg i.
        """
        anag_utils.apiTrace()
        self.vtk_data.setMaxVisibleLevel( i )

    def setAnisotropicFactors( self, xFactor, yFactor, zFactor ):
        anag_utils.apiTrace()
        self.vtk_data.setAnisotropicFactors( xFactor, yFactor, zFactor )
    def getAnisotropicFactors( self ):
        anag_utils.apiTrace()
        result = self.vtk_data.getAnisotropicFactors()
        return result

    def getDomainBounds( self ):
        """
        Returns the low and high in the x, y and z dimensions, as a dictionary
        whose keys are 'x','y' and 'z', and whose values are [lo,hi] pair.  Thus
        the ['x'][0] element is the low x value, ['x'][1] the high x value,
        ['y'][0] the low y value, etc.
        """
        anag_utils.apiTrace()
        result = {}
        for axis in 'x','y','z':
            result[axis] = ( self.vtk_data.getDomainMin()[axis],
                             self.vtk_data.getDomainMax()[axis] )
        return result


    def getNumEmbeddedBoundaries( self ):
        """
        Only knows about EBs defined the old way, i.e. fraction-0, xnormal-0,
        etc.
        """
        anag_utils.apiTrace()
        result = self.vtk_data.getReader().GetNumEBs()
        return result
    # For backwards compatibility
    getNumberOfEmbeddedBoundaries = getNumEmbeddedBoundaries
    

    def getNumLevels( self ):
        anag_utils.apiTrace()
        result = self.vtk_data.getMaxAvailableLevel() + 1
        return result
    # For backward compatibility:
    getNumberOfLevels = getNumLevels


    def getNumBoxes( self, level ):
        """ Returns number of boxes at specified level. """
        anag_utils.apiTrace()
        result = self.vtk_data.getReader().GetLevelNumBoxes( level )
        return result
    # For backward compatibility:
    getNumberOfBoxes = getNumBoxes

    def getBoxExtents( self, level, box_num ):
        """
        Returns a dictionary whose keys are 'i', 'j' and 'k'.  For each key, the
        value is a pair indicating the low and high cell coordinate in that
        dimension.
        """
        anag_utils.apiTrace()
        result = self.vtk_data.getBoxExtents( level, box_num )
        return result        

    def getDatum( self, level, box_num, component, i, j, k=0 ):
        """
        Returns value of the indicated component at the indicated level and box,
        at box coordinates (i,j,k).  Note (i,j,k) are relative to the box, not
        to the entire problem domain (and thus they run from 0 to the box's
        size, in cells, along the respective dimensions).  To convert from
        problem-domain coordinates to box-relative coordinates, use the
        getBoxExtents() method.

        Arg component is the string name of the variable.  Use
        getComponentNames() (a list -- call its index() method) to obtain the
        correspondence between components' names and their ordinal numbers.

        For 2D data, leave arg k at its default.
        """
        reader = self.vtk_data.getReader()
        comp_list = self.vtk_data.getComponentList()
        result = reader.GetDatum( level, box_num,
                                  comp_list.index(component), i, j, k )
        return result

    def getCurrentRange( self ):
        """
        Returns the low and high values of the current component, over the
        currently visible levels.  If you want the range over all levels,
        use the getNumLevels(), setVisibleLevelMin(), and
        setVisibleLevelMax() methods of this class.  To make a particular
        component the current one, use setCurrentComponent().
        """
        anag_utils.apiTrace()
        result = self.vtk_data.getReader().GetVisibleRange()
        return result


    def getRange( self, level, component ):
        """
        Returns the low and high values of the indicated component (identified
        by name or number -- your choice) at the indicated level.
        """
        anag_utils.apiTrace()
        if type(component) == type("string"):
            compnames = self.getComponentNames()
            comp_num = compnames.index( component )
        else:
            comp_num = component
        return self.vtk_data.getReader().GetLevelComponentRange(level,comp_num)


    def is2DMode( self ):
        """
        Returns 1 if the data is intrinsically two-dimensional, or if we're
        working with a slice taken from a 3D data set.
        """
        anag_utils.apiTrace()
        result = self.vtk_data.is2DMode()
        return result


    def isResliceMode( self ):
        """
        Returns 1 if we're in reslice mode, i.e. we're looking at a 2D slice of
        a 3D dataset.
        """
        anag_utils.apiTrace()
        return self.vtk_data.isResliceMode()


    def printBoxInfo( self ):
        """
        Prints (on stdout) a summary of the number of boxes and cells, by level.
        """
        anag_utils.apiTrace()
        print self.vtk_data.getBoxInfo()
        

    def printRangeInfo( self ):
        """
        Returns a summary of the range of every component.
        Note this takes in only the *visible* ranges, i.e. those included by
        the currently visible levels.
        Beware, this function may cause much data to be loaded.
        """
        anag_utils.apiTrace()
        print self.vtk_data.getRangeInfo()

    def useGhostCells( self, on_off ):
        """
        Turn on ghost cells globally.  Generate them if they are not provided
        in the hdf5 file.
        """
        anag_utils.apiTrace()
        assert( (on_off==0) or (on_off==1) )
        self.vtk_data.setAlwaysUsePaddedBoxes( on_off )


    def defineNewComponent( self, name, func, argnames_or_nums ):
        """
        Creates a new component named "name", that is generated by applying
        the pointwise (Python) function "func" to every tuple made of
        corresponding elements of the (existing) components named in "argnames"
        (which should be a tuple of strings or ints -- your choice).
        Warning to Python novices: when typing to the interpreter,
        ('foo','bar') is a tuple, but ('foo') is not; to create a one-element
        tuple, add a comma thusly: ('foo',).

        None of this will work if func is not sufficiently "static".  In
        particular, don't pass a lambda function or a function defined
        inside another function.  If you must use a lambda, do something like
        this:
          add_5 = lambda x : foo(x,5)  # add_5 is at module scope
          c.reader.defineNewComponent( 'add5', add_5, (1,) )

        Example: see examples/newcomp.py.
        """
        anag_utils.apiTrace()
        self.vtk_data.defineNewComponent( name, func, argnames_or_nums )

    def registerNewComponent( self, name ):
        """
        Call this after creating a new component on VisualizableDataset directly
        i.e. not through c.reader.defineNewComponent().
        """
        anag_utils.apiTrace()
        self.vtk_data.registerNewComponent( name )


    def hideBox( self, level, box_num ):
        """
        General comments for hideBox(), showBox(), hideBoxes(), showBoxes(),
        hideAllBoxes() and showAllBoxes():
        1. By default, ChomboVis comes up with all boxes at level 0, component 0
           displayed.  If you're interested in hideBox(), you probably have a
           really big dataset that doesn't comfortably fit into memory.
           To avoid getting a whole component loaded like that, right from the
           get-go, put "-i slices_off=1" on your ChomboVis command line (the
           "-i" is to let you type Python commands).
           Then: "c.reader.hideAllBoxes(0)".
           Then, before selecting a component, you need to designate at least
                 one box as "visible", e.g.
                 "c.reader.showBox(0,1)"
                 This is box 1 on level 0.  If you want to "show" a box on a
                 higher level, m, (and no boxes at level 0), then first you need
                 to say "c.reader.setVisibleLevelMax(m), or ChomboVis won't know
                 what colormap range to set.
           Finally: select a component (either from the
                 Visualization-->DataSelection menu, or the
                 "c.reader.setCurrentComponent" command.

        2. If ChomboVis doesn't seem to be responding to your hideBox() or
           showBox() commands, try typing "c.misc.vtkUpdate()".

        3. ChomboVis will go to your hdf5 file only for the boxes you
           specifically ask for.  Set the debug level (on the "Options" pulldown
           menu or "c.misc.setDebugLevel(4)" to see a console message each time
           a box gets loaded or freed.

        Specific comment for hideBox(): this turns off the specified box.
        """
        anag_utils.apiTrace()
        assert( 0 <= level <= self.vtk_data.getMaxAvailableLevel() )
        assert( 0 <= box_num < self.vtk_data.getLevelNumBoxes(level) )
        self.vtk_data.hideBox( level, box_num )


    def showBox( self, level, box_num ):
        """
        Do show the indicated box.
        """
        anag_utils.apiTrace()
        assert( 0 <= level <= self.vtk_data.getMaxAvailableLevel() )
        assert( 0 <= box_num < self.vtk_data.getLevelNumBoxes(level) )
        self.vtk_data.showBox( level, box_num )


    def hideBoxes( self, level, box_num_tuple ):
        """
        Like hideBox(), but operates on a collection of boxes, e.g. (1,3,5).
        """
        anag_utils.apiTrace()

        assert( 0 <= level <= self.vtk_data.getMaxAvailableLevel() )
        for box_num in box_num_tuple:
            assert( 0 <= box_num < self.vtk_data.getLevelNumBoxes(level) )

        self.vtk_data.hideBoxes( level, box_num_tuple )


    def showBoxes( self, level, box_num_tuple ):
        """
        Like showBox(), but operates on a collection of boxes, e.g. (1,3,5).
        """
        anag_utils.apiTrace()

        assert( 0 <= level <= self.vtk_data.getMaxAvailableLevel() )
        for box_num in box_num_tuple:
            assert( 0 <= box_num < self.vtk_data.getLevelNumBoxes(level) )

        self.vtk_data.showBoxes( level, box_num_tuple )


    def hideAllBoxes( self, level=None ):
        """
        Don't show any boxes on the indicated level.
        If arg level==None (the default), then hide all boxes on all levels.
        """
        anag_utils.apiTrace()
        if level != None:
            self.vtk_data.hideAllBoxes( level )
        else:
            for lev in range(0,self.getNumLevels()):
                self.vtk_data.hideAllBoxes( lev )


    def showAllBoxes( self, level ):
        """
        Don't show any boxes on the indicated level.
        """
        anag_utils.apiTrace()
        self.vtk_data.showAllBoxes( level )


    def getLevelTime( self, level ):
        """
        The simulation time associated with the completion of computation of
        the indicated level.
        """
        anag_utils.apiTrace()
        return self.vtk_data.getReader().GetLevelTime( level )
