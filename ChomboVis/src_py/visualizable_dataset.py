#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
# 
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#

# File: visualizable_dataset.py
# Author: TDSternberg
# Created: 6/18/03

"""
Python interface to the C++ VisualizableDataset class.
"""

import types
import anag_utils
import self_control
import libvisdat
import box_layout_data
import box_layout
import box


g_num_objects_constructed_from_visdat_ptr = {}
g_decls = anag_utils.Declarations(
    'g_decls',
    'g_num_objects_constructed_from_visdat_ptr' )


class VisualizableDataset( self_control.SelfControl ):
#Cut to here
    """
    A VisualizableDataset is the thing that, at any one time, ChomboVis is
    loaded with and displays aspects of.  It is, typically, the result of an
    AMR calculation and consists of BoxLayoutData objects for every data
    component and level of refinement.

    There are two approved ways to obtain a VisualizableDataset:
    1. ChomboVis.reader.getVisualizableDataset()
       which, after "chombovis -i something.hdf5", you can type as follows:
       >>> v = c.reader.getVisualizableDataset()
    2. Call the constructor, passing it the name of an hdf5 file:
       >>> from visualizable_dataset import *
       >>> v = VisualizableDataset( foo.hdf5 )

    Analysis usually begins by calling getBoxLayoutData().  If you are running
    chombovis or chombobrowser (as opposed to chombodata), modify data and want
    to see the results, then switch away from, and then back to, the current
    data component.
    """

#Cut from here

    def __init__( self, hdf5filename=None, visdat_ptr=None ):
        """
        Exactly one of hdf5filename and visdat_ptr must be None (but note that
        visdat_ptr can be 0, as that just indicates it's the first item stuffed
        into the PointerHandleMap).  When
        hdf5filename!=None, we construct a VisualizableDataset from scratch, but
        note that it's a grave mistake to do that more than once for any given
        visdat_ptr, because each visdat_ptr corresponds to one C++ pointer
        and the family of boost::shared_ptr's wrapping it, and __del__ here
        causes that C++ counterpart to be deleted.  So having two (Python)
        VisualizableDataset objects constructed from the same visdat_ptr is like
        constructing two boost::shared_ptr's from the same raw pointer (rather
        than always obtaining a new shared_ptr by copying an existing one).

        When visdat_ptr!=None, we obtain an already-constructed
        VisualizableDataset, typically from vtkChomboReader.

        In ChomboVis, you'll usually want to obtain a pointer to the
        VisualizableDataset that's already held by vtkChomboReader.
        """
        anag_utils.funcTrace()
        self_control.SelfControl.__init__( self, dep_dict={}, metadata = [
           { 'name':'visdat_ptr' }
        ])        
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'hdf5filename', 'visdat_ptr',
            'use_map' )

        assert( ( hdf5filename and (visdat_ptr==None) )
            or  ( (not hdf5filename) and  (visdat_ptr!=None) ) )

        global g_num_objects_constructed_from_visdat_ptr
        use_map = g_num_objects_constructed_from_visdat_ptr
        if hdf5filename:
            self.visdat_ptr = libvisdat.NewVisualizableDataset( hdf5filename )
            use_map[self.visdat_ptr] = 1
        else:
            self.visdat_ptr = visdat_ptr
            if use_map.has_key( visdat_ptr ):
                anag_utils.fatal( "use_map=", use_map, "visdat_ptr=",visdat_ptr)
            assert( not use_map.has_key( visdat_ptr ) )
            use_map[self.visdat_ptr] = 1


#Cut to here

    def defineNewComponent( self, name, func, argnames_or_nums ):
        """
        Arg name is the new component's name.
        Arg func is the generating function.  It takes as many float arguments
            as there are elements in the tuple argnames.
        Arg argnames_or_nums is a tuple with the names, or alternatively the
            serial numbers, of those existing components that the new component
            is to be a function of.

        Warning: the new component will not be known to the ChomboVis class.
        If you want the new component to be known to the ChomboVis class (and,
        through it, to the ChomboVis GUI) use the defineNewComponent() function
        of ChomboVis.readerAPI (">>> c.reader.defineNewComponent(...)").
        Thus:
        >>> c.reader.getNumComponents()
        2
        >>> visdat = c.reader.getVisualizableDataset()
        >>> def mydiff(x,y): return x-y
        ...
        >>> visdat.defineNewComponent( 'diff01', mydiff, (0,1) )
        >>> visdat.getNumComponents()
        3
        >>> c.reader.getNumComponents()
        2
        """
        anag_utils.funcTrace()
        libvisdat.DefineNewComponent( self.visdat_ptr,
                                      name, func, argnames_or_nums )


    def redefineNewComponent( self, name, func, argnames_or_nums ):
        """
        See self.defineNewComponent().
        """
        anag_utils.funcTrace()
        libvisdat.RedefineNewComponent( self.visdat_ptr,
                                      name, func, argnames_or_nums )
        

    def saveToHDF5( self, outfilename, ascii=0 ):
        """ Set arg ascii=1 if you want ascii (not hdf5) output. """
        anag_utils.funcTrace()
        libvisdat.SaveToHDF5( self.visdat_ptr, outfilename, ascii )


    def slice( self, axis, axis_position ):
        """
        Return a VisualizableDataset that is a slice of this one, through the
        indicated axis at the indicated position.
        The slice will have no particle data.

        For an example, see $CHOMBOVIS_HOME/share/ChomboVis/examples/slice.py.
        """
        anag_utils.funcTrace()
        visdat_ptr=libvisdat.Slice( self.visdat_ptr, axis, axis_position )
        if visdat_ptr == 0:
            return None
        else:
            return VisualizableDataset( visdat_ptr = visdat_ptr )
            # OK to construct from visdat_ptr; there are no other Python
            # VisualizableDataset's associated with this pointer.


    def cropLevels( self, crop_levels ):
        """
        Return a version of this VisualizableDataset that includes only the
        levels mentioned in the tuple arg crop_levels.
        Be sure the numbers in crop_levels are consecutive and mention only
        levels that actually exist.  E.g. crop_levels=(0,1), crop_levels=(2,3).
        """
        anag_utils.funcTrace()
        return self.crop( crop_box = box.Box(self.getProblemDomain(0)),
                          crop_levels = crop_levels )


    def cropComponents( self, crop_components ):
        """
        Return a version of this VisualizableDataset that includes only the
        components mentioned in the tuple arg crop_components.  Arg
        crop_components can be a tuple of strings -- component names -- or a
        tuple of numbers -- component numbers.
        """
        anag_utils.funcTrace()
        return self.crop( crop_box = box.Box(self.getProblemDomain(0)),
                          crop_levels = (),
                          crop_components = crop_components )


    def crop( self, crop_box, crop_levels=(), crop_components=() ):
        """
        Return a cropped version of this VisualizableDataset.  Throw away cells
        outside arg crop_box, which is an instance of class Box.  The remaining
        boxes are the intersections of the original boxes and crop_box.  If
        there were ghost cells, we keep them and so in the result there may well
        be some (ghost) cells outside crop_box.

        Arg crop_box is interpreted in terms of cells, not (necessarily) values;
        this is of relevance when the data are not cell-centered.  So for
        example, if crop_box=((0,0),(1,1)) that's a 2x2 box but when the data
        are node-centered the correponsing FAB contains 9 data values.

        Use optional argument crop_levels if you want to save only a subset of
        the levels.  E.g. crop_levels=(0,1).  BUT, be sure that the numbers you
        put into crop_levels are consecutive and mention only existent levels.
        If you only want to crop levels (i.e. retain the whole problem domain)
        use the cropLevels() function.

        Optional argument crop_components works like crop_levels -- if it's ()
        we don't throw away any components.  But if there's something in there
        -- it can be a tuple of component names or component numbers -- then we
        keep only those components.

        The particle data is kept as is; no particles are "cropped" out.

        Note this function returns a VisualizableDataset, while the crop()
        function in box_layout_data.py works in-place.
        """
        anag_utils.funcTrace()

        # Input testing: check that crop_levels are consecutive and refer only
        # to existing levels.
        lc0 = list(crop_levels)
        lc0.sort()
        if list(crop_levels) != lc0:
            anag_utils.error( "Arg crop_levels must be a list of consecutive "
                "and increasing integers." )
            return None
        if len(crop_levels) > 0:
            if( (crop_levels[0] < 0)
            or  (crop_levels[len(crop_levels)-1] > self.getNumLevels()-1) ):
                anag_utils.error( "Arg crop_levels:", crop_levels, "refers to "
                    "nonexistent levels." )
                return None


        # Input testing: check that crop_components refers only to existing
        # components.  If it's numbers, convert them to the corresponding
        # component names, as that's how the C++ layer expects them.
        if len(crop_components) > 0:
            component_names = self.getComponentNames()
            if type(crop_components[0]) == types.IntType: # convert to strings
                tlist = []
                for i in crop_components:
                    assert( (i>=0) and (i<len(component_names)) )
                    tlist.append( component_names[i] )
                crop_components = tuple(tlist)
            else:
                for name in crop_components:
                    assert( name in component_names )

        if not isinstance( crop_box, box.Box ):
            anag_utils.error( "Arg crop_box must be of type box.Box." )
            return None


        if( len(crop_box[0]) == len(crop_box[1]) == 2 ):  # 2D mode
              crop_box = box.Box(( crop_box[0] + (0,), crop_box[1] + (0,) ))
        else:
              crop_box = box.Box((crop_box[0], crop_box[1]))
              # Working with a copy, cuz we're going to call grow() on it if
              # the data are not cell-centered.
        crop_box.grow( incr = self.getDataCentering(), at_both_ends=0 )

        visdat_ptr = libvisdat.CropToGeneralBox(
            self.visdat_ptr, crop_box.getCorners(),
            crop_levels,
            crop_components )
        if visdat_ptr == 0: # Happens, if crop_box intersects nothing.
            anag_utils.warning( "crop() is returning None" )
            return None
        else:
            return VisualizableDataset( visdat_ptr = visdat_ptr )
            # OK to construct from visdat_ptr; there are no other Python
            # VisualizableDataset's associated with this pointer.


    def setDebugLevel( self, d ):
        libvisdat.SetDebugLevel( self.visdat_ptr, d )

    def getBoxLayoutData( self, level, component, contrapad=0 ):
        """
        Returns a (Python) BoxLayoutData object that shadows one of this
        VisualizableDataset's BoxLayoutData's; modifications to the returned
        BoxLayoutData thus affect this VisualizableDataset.

        Arg contrapad, when true, means we want FABs that have ghost
        cells when the hdf5 file doesn't provide them, or FABs that do
        not have ghost cells when the hdf5 file does provide them.
        Default value is false.
        """
        anag_utils.funcTrace()
        bld_int_ptr = libvisdat.GetBoxLayoutData(
            self.visdat_ptr, level, component, 1, contrapad )
        bld_ptr = box_layout_data.BoxLayoutData( bld_int_ptr,
                                                 self.getDataCentering() )
        return bld_ptr


    def getBoxLayout( self, level, real=1 ):
        """
        Returns a box_layout.BoxLayout object.  Its dimensions are the cells,
        not (necessarily) the number of data values -- this matters when the
        data are not cell-centered.
        """
        anag_utils.funcTrace()
        boxes = libvisdat.GetBoxLayout( self.visdat_ptr, level, real )
        dx = libvisdat.GetDx( self.visdat_ptr, level )
        origin =  libvisdat.GetOrigin( self.visdat_ptr )
        result = box_layout.BoxLayout( boxes = boxes, dx = dx, origin = origin )
        result.shrink( self.getDataCentering(), at_both_ends=0 )
        return result

    
    def getDx( self, level ):
        anag_utils.funcTrace()
        return libvisdat.GetDx( self.visdat_ptr, level )


    def getDt( self, level ):
        anag_utils.funcTrace()
        return libvisdat.GetDt( self.visdat_ptr, level )

    def getTime( self, level ):
        anag_utils.funcTrace()
        return libvisdat.GetTime( self.visdat_ptr, level )


    def getDimensionality( self ):
        """
        2 or 3
        """
        anag_utils.funcTrace()
        return libvisdat.GetDimensionality( self.visdat_ptr )


    def getPrecision( self ):
        """
        1=float, 2=double.
        """
        anag_utils.funcTrace()
        return libvisdat.GetPrecision( self.visdat_ptr )


    def getOutputGhost( self ):
        """
        Returns a tuple that shows the depth of user-supplied ghost cells,
        e.g. (1,1,1), (0,0), (2,1).
        """
        anag_utils.funcTrace()
        result = libvisdat.GetOutputGhost( self.visdat_ptr )
        if self.getDimensionality() == 2:
            result = result[0:2]
        return result


    def getDataCentering( self ):
        """
        Returns a tuple that shows the deviation from cell-centeredness in each
        direction.  E.g. (0,0)=cell-centered, (1,1,1)=3D node-centered,
        (1,0,0)=3D x-face-centered, (0,1,1)=3D yz-edge-centered.
        """
        anag_utils.funcTrace()
        result = libvisdat.GetDataCentering( self.visdat_ptr );
        if self.getDimensionality() == 2:
            result = result[0:2]
        return result


    def getNumComponents( self ):
        """ Includes components in the hdf5 file, as well as generated ones. """
        anag_utils.funcTrace()
        return len(self.getComponentNames())

    def getNumLevels( self ):
        anag_utils.funcTrace()
        return libvisdat.GetNumLevels( self.visdat_ptr )

    def getOrigin( self ):
        """ Position of lower-left-hand corner of domain, in distance units. """
        anag_utils.funcTrace()
        result = libvisdat.GetOrigin( self.visdat_ptr )
        if self.getDimensionality() == 2:
            result = result[0:2]
        return result


    def getComponentNames( self ):
        """ Includes components in the hdf5 file, as well as generated ones. """
        anag_utils.funcTrace()
        names = libvisdat.GetComponentNames( self.visdat_ptr )
        return names

    
    def getProblemDomain( self, level ):
        """ Indices of lo and hi corner cells. """
        anag_utils.funcTrace()
        result = libvisdat.GetProblemDomain( self.visdat_ptr, level )
        if self.getDimensionality() == 2:
            result = (result[0][0:2], result[1][0:2])
        return result

    def fabIsEmpty( self, level, box_num, component, padded, real ):
        """
        Returns True if FAB hasn't been allocated.  Useful for studying memory
        parsimony.
        """
        anag_utils.funcTrace()
        if real:
            return libvisdat.RealFabIsEmpty( self.visdat_ptr,
                                             level, box_num, component, padded )
        else:
            return libvisdat.FabIsEmpty( self.visdat_ptr,
                                         level, box_num, component, padded )

    def getLinePlot( self, p0, p1, n, component, finest_level ):
        """
        Returns a tuple of pairs (2-tuples) which represents the line plot
        defined by n equally spaced points between (and inclusive of) the points
        p0 and p1 (2- or 3-tuples depending on the dimensionality of this
        VisualizableDataset).

        Arg n must be greater than 1.

        Arg finest_level is the finest level at which to look for cells; for
        each point on the line, we look first at finest_level, then (if there's
        no box there at that level) we move down through the levels, all the
        way to the zero-th level.

        If any of the coordinates fall out of bounds, we just skip that point
        on the lineplot (hence you will see a gap).

        Note the return value is not a LinePlot object.  LinePlot objects know
        how to draw themselves on the VTK display.  VisualizableDataset is a
        data-only (i.e. no graphics) class.  To obtain an actual LinePlot object
        look at misc_api.makeLinePlot().
        """
        anag_utils.funcTrace()

        assert( n > 1 )
        assert( len(p0) == len(p1) == self.getDimensionality() )
        if self.getDimensionality() == 2:
            p0 = (p0[0],p0[1],0)
            p1 = (p1[0],p1[1],0)

        assert( finest_level < self.getNumLevels() )
        return libvisdat.GetLinePlot( self.visdat_ptr, p0, p1, n, component,
                                      finest_level );


#Cut from here
    def unitTest( self ):
        anag_utils.funcTrace()

        def mydiff(x,y): return x - y
        self.defineNewComponent( 'mydiff', mydiff, (1,0) )

        db_lev = anag_utils.getDebugLevel()
        self.setDebugLevel( 4 )
        anag_utils.setDebugLevel( 4 )

        bld1 = self.getBoxLayoutData( 1, 2 )
        bld1.unitTest()

        bld2 = self.getBoxLayoutData( 1, 2, 0, 1)
        bld2.unitTest()

        self.setDebugLevel( db_lev )
        anag_utils.setDebugLevel( db_lev )
        (temp_file_name,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
            'visdattest', 'hdf5', create=0 )
        self.saveToHDF5( temp_file_name )

#
# Unit test
#
if __name__ == '__main__':
    visdat = VisualizableDataset( hdf5filename='../data/test.2d.hdf5' )
    visdat.unitTest()
