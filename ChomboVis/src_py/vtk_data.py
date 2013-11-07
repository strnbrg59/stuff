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

# File: vtk_data.py
# Author: TDSternberg
# Created: 8/13/01

"""
Deals in component and level selection.  Also, other state variables of
relevance that are not specific to a single rendering pipeline.

I've thought of breaking this up into two pieces, where one piece would deal
with pure data -- stuff that has relevance even in the absence of rendering.
The problem with doing that, unfortunately, is that for some variables it's not
100% clear whether they'd belong here or in "pure data", and that would make
coding tricky and annoying.

Corresponds to control_data.py.
"""

import os
import os.path
import sys
import time
import anag_utils
import release_number
from notifier import Notifier
import self_control
import visualizable_dataset
import vtk
import libVTKChomboPython

g_ebBlankCoveredName = '__blank'  # const.  For old-eb blanking trick.  Not
                                  # to be confused with '__covered'.
g_decls = anag_utils.Declarations( 'g_decls', 'g_ebBlankCoveredName' )

class VtkData( self_control.SelfControl ):

    def __init__( self, dep_dict, infile=None ):
        anag_utils.funcTrace()
        metadata = [
           { 'name':'release_number', 'get':1, 'save':1 },
               # See fixup_version_number.sh and release_number.py.
           { 'name':'accum_start', 'set':1, 'notify':1},
           { 'name':'accum_middle', 'set':1, 'notify':1},
           { 'name':'accum_end', 'set':1, 'notify':1},
               # These trigger accumulator update functions in vtk_grid,
               # vtk_iso and vtk_slice (as of this writing; registration is
               # open to anyone).

           { 'name':'infile_name', 'get':1 }, # hdf5
           { 'name':'visdat', 'get':1}, # VisualizableDataset

        # domain_min|max are not permuted even in reslice mode.
           { 'name':'domain_min', 'initval':{}, 'get':1},
           { 'name':'domain_max', 'initval':{}, 'get':1},
               # legal extent along 'x','y','z' axes

        # range_min|max are the known ranges, for each component.
        # Since we don't load all the levels at once, these numbers may
        # gradually expand.  As they do, they trigger actions in control_iso
        # and control_cmap -- widening of the scale widgets there.
        # The elements of range_min|max are Notifiers.  control_iso and
        # control_cmap register with them, to expand their scale ranges if
        # necessary.
           { 'name':'range_min',  'initval':{}, 'get':2},
           { 'name':'range_max',  'initval':{}, 'get':2},

        # cmapped_range_min|max, subscripted by component, indicate the range
        # that is mapped to values in the current colormap.  Initially, these
        # are the same as range_min|max, but in control_cmap we can narrow or
        # widen them.  The elements of cmapped_range_min|max are Notifiers
        # because when they narrow, control_iso needs to narrow the soft limits
        # of its scale widgets.
           { 'name':'cmapped_range_min', 'initval':{},
               'get':2, 'set':2, 'save':1},       # elements are Notifiers
           { 'name':'cmapped_range_max', 'initval':{},
               'get':2, 'set':2, 'save':1 },      # elements are Notifiers

           { 'name':'component_list',   'initval':[], 'get':1},
               # A list of the names of the components.
           { 'name':'env_chombo_vis_home', 'get':1},
               # Home directory of ChomboVis sources, subdirs Vtk, src_py, etc.
               # We don't call it chombo_vis_home because that confuses
               # digraph.py.
           { 'name':'chombo_vis_data_dir', 'get':1},

           { 'name':'cur_component', 'get':1, 'set':2, 'save':1, 'notify':1,
                'initval':None},
               # Name of currently visible component.

           { 'name':'min_available_level', 'get':1, 'set':1, 'initval':0 },
           { 'name':'max_available_level', 'get':1, 'set':1 },
               # min and max in the dataset itself.

           { 'name':'min_visible_level', 'get':1, 'set':2, 'initval':-1,
             'save':1, 'notify':1 }, # initval of -1 forces setMinVisibleLevel()
                                     # to not return early, first time through.
           { 'name':'max_visible_level', 'get':1, 'set':2, 'initval':-1,
             'save':1, 'notify':1 },

           { 'name':'reslice_position', 'initval':{}, 'get':1, 'set':2,
             'save':5 },
           { 'name':'reslice_direction', 'get':1, 'set':2, 'notify':1,
             'save':5 },
           { 'name':'reslice_notifier', 'set':1, 'notify':1 },

           { 'name':'reader', 'get':1},  # vtkChomboReader
           { 'name':'update_is_disabled', 'initval':1, 'get':1},
                # Disables self.vtkUpdate().  Useful in initialization.

            # The next two, selected_box and selected_cell, are not used here.
            # They're set from selector.py, and databrowsers register with them.
            # We only put them here because they are, after all, data, and
            # the databrowser widgets already had a dependency on vtk_data.
            {'name':'selected_box', 'notify':1, 'get':1},
                # (level,box) tuple hilited by selector (not necessarily the
                # same as the box corresponding to the in-focus databrowser).
            {'name':'selected_cell', 'notify':1, 'get':1, 'init':'None'},

            # Particle (ordinal number among all particles!) that has been
            # picked (ctrl-button1):
            {'name':'picked_particle', 'notify':1, 'get':1},

            # control_fab_tables sets this (the value is unimportant) and
            # selector responds by cycling through the whole set of visible
            # levels.
            {'name':'new_databrowser', 'notify':1},

            # These are for proper positioning of slices, isocontours and EBs
            # when we have more than one visible level in 2D mode.
            { 'name':'slice_origin', 'save':5, 'set':1 },
            { 'name':'slice_offset', 'get':1, 'set':2, 'save':5 },
            { 'name':'z_slice_is_visible', 'set':1, 'notify':1, 'save':1 },
                # vtk_iso will register with this.  And it will be set in
                # vtk_slice.

            { 'name':'texture_enabled', 'get':1, 'set':1, 'notify':1},
              # Value of this is initialized from cmd_line.texture.

            { 'name':'slice_grid_visibility', 'set':1, 'save':1, 'notify':1},
              # Set from vtk_grid, acted upon in vtk_slice.
            { 'name':'grid_color', 'set':1, 'get':1, 'save':1,
                'initval':[1,1,1]},
            { 'name':'grid_color_notifier', 'set':1, 'notify':1},
            { 'name':'grid_line_width', 'get':1, 'initval':1, 'notify':1,
                'save':1 },
            { 'name':'isocontour_clip_value', 'get':1, 'set':1, 'initval':0.0,
                'save':1 },

            {'name':'anisotropic_factors_notifier', 'notify':1},
            {'name':'anisotropic_factors', 'initval':(1,1,1),
                'get':2, 'set':2, 'save':1},

            { 'name':'always_use_padded_boxes', 'get':1, 'set':2, 'save':1,
                'notify':1 },
            { 'name':'always_use_real_boxes', 'get':1, 'set':2 },

            { 'name':'gui_range_constraints', 'set':1, 'initval':(-1e70,1e70),
                'save':1 },

            { 'name':'new_component_notifier', 'notify':1 },
            { 'name':'loaded_first_component_notifier', 'notify':1 },

            { 'name':'hide_boxes_notifier', 'notify':1, 'initval':(0,()) },
            { 'name':'show_boxes_notifier', 'notify':1, 'initval':(0,()) },

            # These are used for managing construction and distribution of
            # _LocalVtkData instances.
            {'name':'local_vtk_data_pool', 'initval':[]},
            {'name':'local_vtk_data_pool_index', 'initval':0},

            # These are used only in subclass _LocalVtkData, but we need to
            # "declare" them here because of a current inadequacy in the
            # variable auditing mechanism.
            {'name':'follow_global_levels', 'set':2},
            {'name':'follow_global_curcomponent', 'set':2},
            {'name':'follow_global_padded_boxes', 'set':2},
            {'name':'defactoInit_called'},
            {'name':'vtk_update_notifier', 'notify':1},
            {'name':'never_do_vtk_update', 'set':1},
            {'name':'instance_identifier', 'get':1, 'set':1}, # For debugging
            {'name':'optimization_mode', 'set':2, 'initval':0}
        ]

#       timer = anag_utils.Timer('VtkData.__init__()')
#       timer.on()
        self_control.SelfControl.__init__( self, dep_dict=dep_dict, 
                                           metadata=metadata )
#       timer.stop()

        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'dep_dict' )

        self.release_number = release_number.the_number 

        if os.environ.has_key( 'CHOMBOVIS_HOME' ):
            self.env_chombo_vis_home = os.environ['CHOMBOVIS_HOME']
            self.chombo_vis_data_dir = \
                self.env_chombo_vis_home + '/share/ChomboVis/data'
        else:
            anag_utils.fatal(
                'You must define the CHOMBOVIS_HOME environment component')



        if not isinstance( self, _LocalVtkData ):
            self.setTextureEnabled( self.cmd_line.getTexture() )
            # Create the pool of _LocalVtkData instances.
            # If 30 is not enough, we'll trigger an assertion in
            # makeLocalVtkData().
            for i in range(0,70): 
                self.local_vtk_data_pool.append(
                    _LocalVtkData(
                        dep_dict = { 'vtk_data':self,
                                     'saved_states':self.saved_states,
                                     'cmd_line':self.cmd_line,
                                     'vtk_vtk':self.vtk_vtk } ))

        #decls.memberFunctionAudit(self)

    def _initForFirstHDF5( self ):
        """
        Called after LoadHDF5().  Does initialization that couldn't be done
        in the constructor, i.e. before we'd loaded a file.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self', 'axis_numbers',
            'extents', 'level', 'axis', 'axes', 'i', 'comp_name', 'position',
            'dx0' )

        self.reader.SetAlwaysUseRealBoxes(1)

        # If the hdf5 file contains ghost cell information, then make cum-
        # ghost-cell the default everywhere.  This way we economize on memory
        # (don't necessarily have to hold two sets of FABs) and time (don't
        # have to strip the ghost cells off to create unpadded FABs).
        if self.reader.GetGhostCellsSuppliedByUser() == 0:
            self.setAlwaysUsePaddedBoxes(0)
        else:
            self.setAlwaysUsePaddedBoxes(1)

        self.max_available_level = self.reader.GetNumLevels()-1
        for i in range( 0, self.reader.GetNumComponents() ):
            comp_name = self.reader.GetComponentName( i )
            self.component_list.append( comp_name )
            self.range_min[comp_name] = Notifier(float('inf'))
            self.range_max[comp_name] = Notifier(float('-inf'))
            self.cmapped_range_min[comp_name] = Notifier(1)
            self.cmapped_range_max[comp_name] = Notifier(-1)
            # The 1/-1 thing is our signal that initialization isn't complete.

        # Save domain extents
        extents = self.reader.GetDomainExtentsXYZ()
        i = 0
        for axis in 'x','y','z':
            self.domain_min[axis] = extents[ i ]
            self.domain_max[axis] = extents[ 3 + i ]
            i = i+1

        if self.is2DMode() and (self.cmd_line.getSliceAxis() != None):
            # If we're a _LocalVtkData that's been coerced into reslice mode,
            # then we take care to call setResliceDirection() and
            # setReslicePosition() on it directly.
            self.setResliceDirection( self.cmd_line.getSliceAxis() )
            position = float(self.cmd_line.getAxisPosition())
            for axis in 'x', 'y', 'z':
                position = max( self.domain_min[axis],
                            min( self.domain_max[axis],
                                 float(self.cmd_line.getAxisPosition())))
                self.setReslicePosition( position, [axis,] )

        self.setMinVisibleLevel(0)
        self.setMaxVisibleLevel(0)

        #self.setCurComponent( self.component_list[0], suppress_callbacks=1 )

        # Slice origin and offset.
        dx0 = self.reader.GetLevelDx(0)[0]
        if self.is2DMode():
            self.slice_origin = -0.5 * dx0
            self.slice_offset = 0.01 * dx0
        else:
            self.slice_origin = 0.0
            self.slice_offset = 0.0
        
        self.saved_states.setPrerestoreIsResliceMode( self.isResliceMode() )
        self.saved_states.setIs3DMode( not self.is2DMode() )

        if self.is2DMode():
            self.vtk_vtk.forgetAxesFrame()

        self.setAnisotropicFactors( self.reader.GetAnisotropicFactors()[0],
                                    self.reader.GetAnisotropicFactors()[1],
                                    self.reader.GetAnisotropicFactors()[2] )


        decls.memberFunctionAudit(self)


    def _refresh( self ):
        """
        Overrides self_control.SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        # Sanity-check the restored variables, in case we're restoring a state
        # with aspects that don't apply to the current dataset.  There are some
        # things we won't try to recover from: datasets with different sets of
        # components,...[FIXME: what else?]
        self.setMinVisibleLevel(
             min( max( self.min_available_level, self.getMinVisibleLevel() ),
                  self.max_available_level ), force=1 )
        self.setMaxVisibleLevel(
             max( min( self.max_available_level, self.getMaxVisibleLevel() ),
                  self.min_available_level ), force=1 )

        self.setAlwaysUsePaddedBoxes( self.getAlwaysUsePaddedBoxes() )

        # Make sure all the data you need is loaded.
        if self.getCurComponent():
            self.setCurComponent( self.getCurComponent(), force=1,
                                  suppress_callbacks=1 )

        self.setAnisotropicFactors( self.anisotropic_factors[0],
                                    self.anisotropic_factors[1],
                                    self.anisotropic_factors[2] )


    def cleanup( self ):
        """
        Free up the memory used by the reader.  From this point on, better not
        try doing anything at all with this ChomboVis.
        """
        anag_utils.funcTrace()
        # Here's the correct way to delete the ChomboReader.  This calls the
        # Delete() method.  But you can't call Delete() from here; it's not even
        # wrapped for the Python interface.
        for lvd in self.local_vtk_data_pool:
            lvd.unregisterCallbacks()
            lvd.zeroSelfVariables()
            lvd.reader = None

        self.reader = None


    def getGhostCellsSuppliedByUser( self ):
        """
        Return 1 if the hdf5 file provides ghost cells.  (If not, we generate
        them).
        """
        anag_utils.funcTrace()
        return self.reader.GetGhostCellsSuppliedByUser()


    def getGhostCellsDepth( self ):
        """
        Returns a 2- or 3-vector of the ghost cell depth.  If there are ghost
        cells in the hdf5 file, this is the same as the OutputGhost item in the
        hdf5 file.  Otherwise, it's just (1,1[,1]).
        """
        anag_utils.funcTrace()
        unpadded_dims = self.reader.GetDimensions(0,0,2,0,1)
        padded_dims = self.reader.GetDimensions(0,0,2,1,1)
        result = [ (padded_dims[0]-unpadded_dims[0])/2,
                   (padded_dims[1]-unpadded_dims[1])/2 ]
        if not self.is2DMode():
            result.append( (padded_dims[2]-unpadded_dims[2])/2 )
        return result


    def setAlwaysUsePaddedBoxes( self, yes_no ):
        """
        Triggers a notifier that causes most of the pipelines' local_vtk_data
        instances to switch their own ghost cell mode.
        """
        anag_utils.funcTrace()
        assert( (yes_no == 0)  or  (yes_no == 1) )
        self.reader.SetAlwaysUsePaddedBoxes( yes_no )
        self.always_use_padded_boxes.set( yes_no ) # It's a notifier.


    def printParticleCoordinates( self, particle_num ):
        """
        Print a summary of the indicated particle's values on all
        its components.  Arg particle_num is the particle's ordinal number among
        all particles, i.e. not just those that have passed our filters.
        """
        anag_utils.funcTrace()
        result = ''
        for i in range(0, self.getNumParticles()):
            name = self.getParticleComponentName( i )
            result = result + name + ': ' +\
                str(self.reader.GetParticleCoordinate( particle_num, name )) +\
                '\n'
        print result


    def getParticleCoordinate( self, particle_num, component_name ):
        anag_utils.funcTrace()
        return self.reader.GetParticleCoordinate( particle_num, component_name )


    def getParticleComponentName( self, i ):
        anag_utils.funcTrace()
        return self.reader.GetParticleComponentName(i)


    def makeLocalVtkData( self, 
                          follow_global_levels=1,
                          follow_global_curcomponent=1,
                          follow_global_padded_boxes=1,
                          always_use_real_boxes=0,
                          instance_identifier=None,
                          never_do_vtk_update=None,
                          global_vtk_data=None,
                          share_visualizable_dataset=1 ):
        """
        Returns an instance of a _LocalVtkData (which already exists in the list
        self.local_vtk_data_pool) and does some initialization on it.
        See the comments for class _LocalVtkData for an explanation for why we
        do this pool thing.

        Arg follow_global_levels indicates whether you want this _LocalVtkData
        to update automatically its min and max visible levels whenever those
        change in the global VtkData.

        Arg follow_global_curcomponent is, analogously, for following the global
        VtkData's cur_component.

        Arg follow_global_padded_boxes follows the global VtkData's
        always_use_padded_boxes, which is about using ghost cells (or not using
        them).

        Arg always_use_real_boxes, when 1, causes self.reader (a
        vtkChomboReader) to never use the subdivided boxes that are our solution
        to the problem of hiding coarse-level data in our renderings.  1 is the
        appropriate value for VtkGrid and VtkFabTables.

        Beware about choosing other than 1 for the follow_global_levels and
        follow_global_curcomponent args.  If they're both zero, then when the
        global VtkData changes component, this _LocalVtkData will hold the
        refcount on the previous component; this uses up memory, so don't do it
        unless you have a good reason.  The general rule is, you save memory
        by tracking the global VtkData's current component and visible levels;
        all the FABs are shared (and reference counted) among the
        vtkChomboReaders.

        If you do set follow_global_levels to zero, then somewhere you need to
        call setMinVisibleLevel() and setMaxVisibleLevel() on your local VtkData
        and likewise if you set follow_global_curcomponent to zero, you need to
        manually call setCurComponent().  Failure to do this will mean the
        vtkChomboReader associated with this local VtkData will always have zero
        "pieces".
 
        If you're going to call SetAlwaysUseRealBoxes(1), then you should still
        probably register for levels and curcomponent, because the global
        VtkData (which doesn't use real boxes always) will have to load the
        real boxes anyway, when it loads subdivided boxes.

        Arg global_vtk_data is if you want to create a _LocalVtkData that is
        subordinate to another _LocalVtkData (presumably, so the former can
        track the latter's visible levels and curcomponent).  We do it this way,
        rather than simply calling this function on a _LocalVtkData, because
        it's necessary for all _LocalVtkData's to be in existence since very
        early in ChomboVis' life -- see the comments about the "pool" in the
        comment section of class _LocalVtkData.

        If arg share_visualizable_dataset is None, then this _LocalVtkData
        has its own independent VisualizableDataset.  By default, these are
        shared.  We need independent ones in vtk_slice, when for purposes of
        clipping with the vtkClipPolydata, we need to serve up scalars from
        each of the slices.  Nonetheless, we do load the same hdf5 file; there's
        no provision, yet, in ChomboVis, to have readers tied to different
        files.  (You *can* construct as many VisualizableDataset's, each tied
        to whatever file you like, but you do that in a script and there's no
        provision for using that data in the visualization.)
        """
        anag_utils.funcTrace()

        assert(   len(self.local_vtk_data_pool)
                > self.local_vtk_data_pool_index )
        # If it fails this assertion, just generate a bigger pool; no problem.
        # The number of instances of _LocalVtkData we'll need is, at the
        # moment, fixed.

        result = self.local_vtk_data_pool[ self.local_vtk_data_pool_index ]
        result.setInstanceIdentifier( instance_identifier )
        result.setNeverDoVtkUpdate( never_do_vtk_update )
        if global_vtk_data:
            result.vtk_data = global_vtk_data
        else:
            result.vtk_data = self
        result.defactoInit( share_visualizable_dataset )
        result.setFollowGlobalLevels( follow_global_levels )
        result.setFollowGlobalCurcomponent( follow_global_curcomponent )
        result.setFollowGlobalPaddedBoxes( follow_global_padded_boxes )
        result.setAlwaysUseRealBoxes( always_use_real_boxes )

        # Economize on FABs: make default padded or unpadded as hdf5 file does
        # or doesn't supply ghost cells.
        if result.getGhostCellsSuppliedByUser() == 1:
            result.setAlwaysUsePaddedBoxes( 1 )
        else:
            result.setAlwaysUsePaddedBoxes( 0 )

        self.local_vtk_data_pool_index += 1
        return result


    def setCurComponentCheaply( self, component_name ):
        """
        Like setCurComponent(), but just sets it in the vtkChomboReader and
        doesn't try to adjust the colormap range and trip all kinds of
        notifiers.

        Doesn't do any sanity-testing either.                
        """
        anag_utils.funcTrace()
        self.reader.SetCurrentComponentName( component_name );


    def setCurComponent( self, component_name, **kw ):
        """
        self.cur_component is a Notifier.  We've declared it 'set':2 and
        this here is its custom-coded setter.  What we do here, before
        triggering all the callbacks that have been registered with
        self.cur_var, is initialize (if necessary) that component's entries in
        the self.range_min|max and self.cmapped_range_min|max lists.

        This function also serves as the handler for the component-choice
        widget.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self', 'component_name','kw',
            'visible_range', 'c', 'previous_component' )

        if type(component_name) == type(2):
            component_name = self.component_list[component_name]

        if(   ( component_name == self.getCurComponent() )
          and ( ( not 'force' in kw.keys() ) or ( kw['force'] == 0 ) ) ):
            return

        if component_name == None:
            if self.reader:
                anag_utils.fatal(
                "Tried to set current component to None. This \n"
                "has been known to happen if your user script or .chombovisrc\n"
                "saves the state of the system before any file has been\n"
                "loaded.")
            # If self.reader==None, that means we got here from
            # SelfControl.zeroSelfVariables() and it's ok.
            return

        self.reader.SetCurrentComponentName( component_name );
        visible_range = self.updateRanges( component_name )

        # Check if cmapped_range_min|max has been initialized.  From here on,
        # we won't modify cmapped_range_min|max even as newly-loaded levels
        # reveal a wider data range.  That's because we want to let the user
        # restrict or widen cmapped_range_min|max from control_cmap.
        if( self.cmapped_range_min[component_name].get() >
            self.cmapped_range_max[component_name].get() ):
            self.cmapped_range_min[component_name].set( visible_range[0] )
            self.cmapped_range_max[component_name].set( visible_range[1] )

        # Trigger notifier callbacks:
        previous_component = self.cur_component.get()
        self.cur_component.set( component_name, **kw )
        if previous_component == None:
            self.setLoadedFirstComponentNotifier( 1 )

        if( ( not 'suppress_callbacks' in kw.keys() )
        or ( kw['suppress_callbacks'] == 0 ) ):
            self.vtkUpdate( source = str(self.__class__) )

        decls.memberFunctionAudit(self)    


    def updateRanges( self, component_name ):
        """
        Updates self.range_min|max.
        Returns visible range.
        Upon exit, leaves current component unchanged.
        """
        anag_utils.funcTrace()

        saved_cur_comp = self.reader.GetCurrentComponentName()
        if saved_cur_comp != component_name:
            self.reader.SetCurrentComponentName( component_name );

        # Set range_min|max to the widest range known so far (i.e. in the levels
        # loaded so far).
        visible_range = apply(
                            self.reader.GetConstrainedVisibleRange,
                            self.gui_range_constraints )
        if not component_name in self.range_min.keys():
            self.range_min[component_name] = Notifier( visible_range[0] )
            self.range_max[component_name] = Notifier( visible_range[1] )
        self.range_min[ component_name ].set( visible_range[0] )
        self.range_max[ component_name ].set( visible_range[1] )

        if saved_cur_comp and (saved_cur_comp != component_name):
            self.reader.SetCurrentComponentName( saved_cur_comp )

        return visible_range[:]


    def is2DMode( self ):
        """
        Are we looking at 2D data?  That's when either our dataset is
        intrinsically 2-dimensional, or it's 3D but we're looking at 2D slices
        of it.
        """
        anag_utils.funcTrace()
        if self.saved_states.getNumHDF5sLoaded()==0:
            anag_utils.warning( "No file loaded." )
            return None
        return (self.reader.GetNumDims() == 2) or (self.isResliceMode() == 1)


    def getLevelDx( self, level ):
        """ Returns a triple -- may be anisotropic """
        anag_utils.funcTrace()
        return self.reader.GetLevelDx(level)


    def getNumDims( self ):
        """ Returns 3 in reslice mode. """
        anag_utils.funcTrace()
        return self.reader.GetNumDims()


    def isResliceMode( self ):
        """ Are we viewing 2D slices of a 3D dataset? """
        anag_utils.funcTrace()
        return( (self.reslice_direction.get() != None) 
             or (self.cmd_line.getSliceAxis() != None) )


    def textureOn( self ):
        return self.getTextureEnabled()==1


    def getDataCentering( self, permute_in_reslice_mode=1 ):
        """
        Returns a 3-tuple indicating data centering:
        (0,0,0) => cell
        (1,0,0) => x-face
        (0,1,0) => y-face
        (0,0,1) => z-face
        (1,1,0) => z-edge in 3D, node in 2D
        (1,0,1) => y-edge
        (0,1,1) => x-edge
        (1,1,1) => node
        """
        anag_utils.funcTrace()

        centering = self.reader.GetDataCentering()
        if self.isResliceMode() and permute_in_reslice_mode:
            direction = self.getResliceDirection()
            result = self.permuteCoords( centering, direction )
        else:
            result = centering
        return result


    #
    # Custom getters and setters
    #
    def setAnisotropicFactors( self, xFactor, yFactor, zFactor ):
        anag_utils.funcTrace()
        self.reader.SetAnisotropicFactors( xFactor, yFactor, zFactor )
        self.anisotropic_factors = (xFactor,yFactor,zFactor)
        extents = self.reader.GetDomainExtentsXYZ()
        i = 0
        for axis in 'x','y','z':
            self.domain_min[axis] = extents[ i ]
            self.domain_max[axis] = extents[ 3 + i ]
            i = i+1
        self.setAnisotropicFactorsNotifier(1) # Triggers _LocalVtkData action.

    def getAnisotropicFactors( self ):
        anag_utils.funcTrace()
        return self.reader.GetAnisotropicFactors()


    def getRangeMin(self, component=None):
        if component == None: component = self.getCurComponent()
        return self.range_min[ component ].get()
    def getRangeMax(self, component=None):
        if component == None: component = self.getCurComponent()
        return self.range_max[ component ].get()

    # Cmapped range of each component, i.e. what's mapped to the extremes of the
    # colormap's data.  Elements are Notifiers.
    def getCmappedRangeMin(self, component=None):
        if component == None: component = self.getCurComponent()
        if component == None:  # No component loaded
            return None
        else:
            return self.cmapped_range_min[ component ].get()
    def getCmappedRangeMax(self, component=None):
        if component == None: component = self.getCurComponent()
        if component == None:  # No component loaded
            return None
        else:
            return self.cmapped_range_max[ component ].get()
    def setCmappedRangeMin(self,x):
        anag_utils.funcTrace()
        if x != None:
            self.cmapped_range_min[self.getCurComponent()].set( x )
    def setCmappedRangeMax(self,x):
        anag_utils.funcTrace()
        if x != None:
            self.cmapped_range_max[self.getCurComponent()].set( x )
    def getCmappedRangeMinNotifiers( self ):
        return self.cmapped_range_min.values()
    def getCmappedRangeMaxNotifiers( self ):
        return self.cmapped_range_max.values()
    def getRangeMinNotifiers( self ):
        return self.range_min.values()
    def getRangeMaxNotifiers( self ):
        return self.range_max.values()
        

    # min and max visible levels
    def _updateLoadedLevels( self, min_level, max_level ):
        """
        Make the vtkChomboReader unload unneeded levels, and load needed ones.
        """
        anag_utils.funcTrace()
        # Last chance to prevent assertion in C++ -- which Python can't catch.
        assert( self.min_available_level <=
                min_level <= max_level <=
                self.max_available_level )
        for level in (
          range( 0, min_level ) +
          range( max_level+1, self.getMaxAvailableLevel()+1)):
            if self.reader.LevelIsVisible( level ) == 1:
                self.reader.HideLevel( level )
        for level in range( min_level,
                             max_level+1 ):
            if self.reader.LevelIsVisible( level ) == 0:
                self.reader.ShowLevel( level )

        # Update range_min|max, in case one of the newly-loaded levels contains
        # values beyond the currently-known range.  Elements of range_min|max
        # are Notifiers.
        cur_comp = self.getCurComponent()
        if cur_comp != None:
            visible_range = apply( self.reader.GetConstrainedVisibleRange,
                                   self.gui_range_constraints )
            self.range_min[cur_comp].set( min( self.range_min[ cur_comp ].get(),
                                               visible_range[0] ))
            self.range_max[cur_comp].set( max( self.range_max[ cur_comp ].get(),
                                               visible_range[1] ))


    def setMinVisibleLevel( self, k, **kw ):
        """
        Observes constraints and moves max if necessary.
        Note self.min_visible_level is a Notifier.
        """
        anag_utils.funcTrace()

        if( ( k == self.getMinVisibleLevel() )
        and ( (not 'force' in kw.keys()) or (kw['force']!=1) ) ):
            return
        # self.min_visible_level.callbacks is empty at this point.
        assert( self.min_available_level <= k <= self.max_available_level)
        self.min_visible_level.set( k, suppress_callbacks=1 )
        if k > self.getMaxVisibleLevel():
            self.max_visible_level.set( k, suppress_callbacks=1 )
        self._updateLoadedLevels( k, self.getMaxVisibleLevel() )
        if( ( not 'suppress_callbacks' in kw.keys() )
        or ( kw['suppress_callbacks'] == 0 ) ):
            # Last chance to prevent assertion in C++ -- which Python can't
            # catch.
            assert( self.min_available_level <=
                    self.getMinVisibleLevel() <= self.getMaxVisibleLevel() <=
                    self.max_available_level )
            self.min_visible_level.set( self.getMinVisibleLevel() )
            self.max_visible_level.set( self.getMaxVisibleLevel() )
            self.vtkUpdate( source = str(self.__class__) )            


    def setMaxVisibleLevel( self, k, **kw ):
        """
        Observes constraints and moves min if necessary
        Note self.max_visible_level is a Notifier.
        """
        anag_utils.funcTrace()

        if( ( k == self.getMaxVisibleLevel() )
        and ( (not 'force' in kw.keys()) or (kw['force']!=1) ) ):
            return

        assert( self.min_available_level <= k <= self.max_available_level )
        self.max_visible_level.set( k, suppress_callbacks=1 )
        if k < self.getMinVisibleLevel():
            self.min_visible_level.set( k, suppress_callbacks=1 )
        self._updateLoadedLevels( self.getMinVisibleLevel(), k )
        if( ( not 'suppress_callbacks' in kw.keys() )
        or ( kw['suppress_callbacks'] == 0 ) ):
            assert( self.min_available_level <=
                    self.getMinVisibleLevel() <= self.getMaxVisibleLevel() <=
                    self.max_available_level )
            self.min_visible_level.set( self.getMinVisibleLevel() )
            self.max_visible_level.set( self.getMaxVisibleLevel() )
            self.vtkUpdate( source = str(self.__class__) )            


    def repositionSlice( self, axis, position ):
        """
        Refers to the 2D slice of a 3D dataset.
        """
        anag_utils.funcTrace()
#       timer = anag_utils.Timer()
#       timer.on()
        decls = anag_utils.Declarations( 'decls', 'self', 'axis', 'position',
            'direction_numbers', 'extents', 'min_extent', 'max_extent', 'eps',
            'l', 'timer', 'axis_num', 'dx', 'dx0', 'centering' )

        # Check legality of position.  (This is to catch an error in a
        # user script; you can't make this mistake from the GUI.)
        extents = self.getDomainExtentsXYZ(permute_in_reslice_mode=0)
        direction_numbers = {'x':0, 'y':1, 'z':2 }
        min_extent = extents[ direction_numbers[ self.getResliceDirection() ]]
        max_extent = extents[ direction_numbers[self.getResliceDirection()] + 3]

        if position < min_extent :
            anag_utils.warning(
                'Range Error: position=' + str(position), ', min_extent=',
                min_extent )
        if position > max_extent :
            anag_utils.warning(
                'Range Error: position=' + str(position), ', max_extent=',
                max_extent, 'position-max_extent=',position-max_extent )

        axis_num = {'x':0,'y':1,'z':2}[axis]
        self.setResliceDirection( axis )
        centering = self.getDataCentering()
        l = self.getMaxVisibleLevel()
        dx0 = self.reader.GetLevelDx(0)[axis_num]
        dx  =  self.reader.GetLevelDx(l)[axis_num]
        self.setReslicePosition(
            min( position + centering[axis_num]*0.5*dx0,
                 max_extent - dx*1E-10 ),
            [axis,])


        self.setResliceNotifier(0)

        self.vtkUpdate( source = str(self.__class__) )

        decls.memberFunctionAudit( self ) # Costly, but swamped by cost of the
                                          # vtkUpdate().
#       timer.stop( msg="Time in repositionSlice():" )


    def setPiece( self, box_num, axis=' ', axis_positionXYZ=0 ):
        """
        Calls vtkChomboReader::SetPiece(), which specifies which FAB is to be
        considered the "current" one.  That's the FAB which
        vtkChomboReader::Execute() puts on the VTK pipeline.

        Set arg axis to 'x', 'y' or 'z', to put vtkChomboReader into slicing
        mode.

        Returns -1 if the indicated position on the indicated axis doesn't
        fall within the indicated box.  Returns 0 otherwise.
        """
        anag_utils.funcTrace()
        return self.reader.SetPiece( box_num, axis, axis_positionXYZ )


    def vtkUpdate( self, source ):
        """
        Arg source identifies the class that called vtkUpdate.  We do that
        because only that class needs to update its accumulator.
        """
        anag_utils.funcTrace()

        decls = anag_utils.Declarations( 'self', 'decls', 'i',
            'source', 'level', 'ret_value' )

        if self.update_is_disabled == 1:
            return

        if self.never_do_vtk_update == 1:
            return

        #
        # Update accumulators.
        # What we do is trip Notifiers -- self.accum_start, accum_middle and
        # accum_end.  At this writing, listeners are vtk_grid, vtk_slice and
        # vtk_iso.
        #
        self.setAccumStart( 0, extra_info=source )
        for i in range(0, self.reader.GetNumPieces() ):
            if self.isResliceMode():
                ret_value = self.setPiece(
                    i, self.getResliceDirection(),
                    self.getReslicePosition( [self.getResliceDirection(),] ))
            else:
                ret_value = self.setPiece( i )

            if ret_value == 0:
                self.setAccumMiddle( i, extra_info=source )

        self.setAccumEnd( 0, extra_info=source )

        if not self.cmd_line.getNoVtk():
            self.vtk_vtk.render()

        self.setVtkUpdateNotifier(source)

        decls.memberFunctionAudit( self )


    def loadHDF5( self, infile_name ):
        """
        The real work gets done in _initForFirstHDF5() (which is triggered
        when, in this function, we set the NumHDF5sLoaded Notifier).

        If we're loading our first hdf5 (i.e. into a ChomboVis that was started
        in "null" mode, then we set a Notifier that causes every class to
        execute its _initForFirstHDF5() method.  But if there's already an hdf5
        loaded, the sequence is much simpler: we set a Notifier that causes
        main.py to delete the current ChomboVis, and construct a whole new one.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self', 'infile_name', 'ret',
            'c', 'timer')
#       timer = anag_utils.Timer( 'vtk_data.loadHDF5()' )
#       timer.on()

        if not os.path.isfile( infile_name ):
            anag_utils.error( "File not found:", infile_name )
            return

        self.reader = libVTKChomboPython.vtkChomboReader()

        self.infile_name = infile_name
        self.cmd_line.setInfile( infile_name )

        ret = self.reader.LoadFile( infile_name )
#       timer.pause( 'after reader.LoadFile()' )
        self.reader.SetOptimizationMode( self.optimization_mode )
#       timer.on()

        if ret != 0 :
            anag_utils.fatal( "Fatal error loading file", infile_name )

        self.reader.SetDebugLevel( anag_utils.getDebugLevel() )

        # Trigger _initForFirstHDF5() and _refresh() in all the other classes.
        # This will happen at initialization too, but there's no harm in that
        # since uninitialized classes will not have registered yet anyway.
        # We briefly disable self.vtkUpdate() for efficiency.
        # self.disableUpdate() # Unnecessary, I think.
        #for c in self.saved_states.num_HDF5s_loaded.callbacks:
        #    sys.stderr.write( "num_hdf5s_loaded callback:" + str(c) + '\n' )
        self.saved_states.setNumHDF5sLoaded(
            1 + self.saved_states.getNumHDF5sLoaded() )
#       timer.pause( 'after setNumHDF5sLoaded()' )
#       timer.on()

        self.enableUpdate()
#       timer.pause( 'after enableUpdate()' )
#       timer.stop()

        decls.memberFunctionAudit( self )


    def defineNewComponent( self, name, func, argnames_or_nums ):
        """
        Creates a new component named "name", that is generated by applying
        the pointwise (Python) function "func" to every tuple made of
        corresponding elements of the (existing) components named in
        "argnames_or_nums" (which should be a tuple of strings or ints -- your
        choice).
        """
        anag_utils.funcTrace()
        assert( not isinstance( self, _LocalVtkData ) )

        visdat = self.getVisualizableDataset()
        visdat.defineNewComponent( name, func, argnames_or_nums )
        self.registerNewComponent( name )


    def redefineNewComponent( self, name, func, argnames_or_nums ):
        """
        Redefines a component previously created by self.defineNewComponent.
        """
        anag_utils.funcTrace()
        assert( not isinstance( self, _LocalVtkData ) )

        visdat = self.getVisualizableDataset()
        visdat.redefineNewComponent( name, func, argnames_or_nums )


    def getVisualizableDataset( self ):
        """
        First time this is called, we construct a (Python) VisualizableDataset
        that corresponds to the (C++) VisualizableDataset that is currently
        loaded in the vis system.  Stores that object as self.visdat, which is
        what we return on subsequent calls to this function.  It would be a
        grave mistake to construct a Python VisualizableDataset more than once
        from the same C++ VisualizableDataset object.  See comments under
        visualizable_dataset.__init__ for more on this.
        """
        if not self.visdat:
            self.visdat = visualizable_dataset.VisualizableDataset(
                visdat_ptr = self.reader.GetVisualizableDatasetPtr() )
        return self.visdat


    def registerNewComponent( self, name ):
        """
        Call this after creating a new component on VisualizableDataset directly
        i.e. not through ChomboVis.ReaderAPI.defineNewComponent().
        """
        anag_utils.funcTrace()

        # Every class but this one registers with self.new_component_notifier.
        # In _LocalVtkData, this function is the callback we register:
        self._newComponentNotifierHandler( name, None )
            
        # Trigger all kinds of bookkeeping adjustments made necessary by the
        # appearance of this new component.
        self.setNewComponentNotifier( name )


    def _newComponentNotifierHandler( self, compname, unused ):
        """
        Do bookkeeping to accomodate the new component.
        """
        self.component_list.append( compname )
        self.range_min[compname] = Notifier(float('inf'))
        self.range_max[compname] = Notifier(float('-inf'))
        self.cmapped_range_min[compname] = Notifier(1)
        self.cmapped_range_max[compname] = Notifier(-1)

        # Pass notification on to objects that use this _LocalVtkData:
        if isinstance( self, _LocalVtkData ):
            map( lambda notifier, self=self :
                self.registerCallback( notifier, self._cmappedRangeCallback ),
                self.vtk_data.getCmappedRangeMinNotifiers()
               +self.vtk_data.getCmappedRangeMaxNotifiers() )         
            self.setNewComponentNotifier( compname )


    def setDebugLevel( self, dl ):
        """
        Affects both Python-layer and C++-layer debug level.  However, the
        C++ debug level can't be set until we've loaded up our hdf5 data.
        """
        anag_utils.funcTrace()
        anag_utils.setDebugLevel( dl )
        if self.saved_states.getNumHDF5sLoaded() > 0:
            self.reader.SetDebugLevel( dl )
        else:
            anag_utils.warning( "Can't set C++ debug level until hdf5 data has"
                " been loaded." )

    def getDebugLevel( self ):
        anag_utils.funcTrace()
        return anag_utils.getDebugLevel()

    
    def setOptimizationMode( self, mode ):
        """
        Choices are 0 for memory, and 1 for speed.
        If arg is 1, then no FABs are ever deleted.  (We'll still only load
        them as needed, but once they're loaded, they're never deleted as,
        otherwise, they would be as they appear to not be needed.)

        Changing the optimization mode from speed to memory will not,
        unfortunately, delete any already-loaded FABs; the effect will only be
        to delete (as they appear to be unneeded) FABs not loaded to this point.
        
        By default, ChomboVis comes up in speed-optimization mode.
        """
        anag_utils.funcTrace()
        assert( (mode==0) or (mode==1) )
        self.optimization_mode = mode
        if self.reader:
            self.reader.SetOptimizationMode( mode )
        # If no reader (yet), then we take care to call SetOptimizationMode()
        # on the reader when it's constructed.


    def disableUpdate(self): self.update_is_disabled = 1
    def enableUpdate(self): self.update_is_disabled = 0


    def getVisibleBoxExtentsXYZ( self, ndx ):
        """
        Arg ndx is an index into the flattened list of visible boxes.
        Returns 6-tuple in format (xmin,ymin,zmin,xmax,ymax,zmax).
        """
        anag_utils.funcTrace()
        level = self.reader.GetVisibleBoxLevel( ndx )
        box_num = self.reader.GetVisibleBoxNum( ndx )
        return self.getBoxExtentsXYZ( level, box_num )


    def getBoxExtentsXYZ( self, level, box, visible_part_only=0 ):
        """
        Returns 6-tuple in format (xmin,ymin,zmin,xmax,ymax,zmax).

        This function usually just calls vtkChomboReader.GetBoxExtentsXYZ(),
        but in reslice mode, the result is permuted.

        In some places in ChomboVis, we still call directly
        vtkChomboReader::GetBoxExtentsXYZ().  Why?  It depends on why we want
        to know the extents.  If it's for rendering (e.g. to determine the
        outline of a box we're going to render) then we call
        vtk_data.getBoxExtents(), so as to benefit from coordinate
        permutation in case we're in reslice mode.  But if our purpose is
        actually to know what the domain of x or y or z is -- say, to set the
        limits of a Scale widget, or to sanity-test input -- then we don't want
        any coordinate permutation and we call directly to vtkChomboReader.

        Arg visible_part_only is of relevance for non-cell-centered data.  When
        1, the extents are of the box that the user should see, i.e. with margin
        cells cut down the middle in the directions the data is not
        cell-centered.  When 0, the extents are of the whole box.
        """
        anag_utils.funcTrace()

        extents = self.reader.GetBoxExtentsXYZ( level, box, visible_part_only )
        if self.isResliceMode():
            direction = self.getResliceDirection()
            return ( self.permuteCoords( extents[0:3], direction )
                   + self.permuteCoords( extents[3:6], direction ))
        else:
            return extents


    def getVisibleBoxLevel( self, box_num ):
        """
        Return the level of the indicated box, where box_num is an index into
        flattened hierarchy -- all the "pieces".
         """
        return self.reader.GetVisibleBoxLevel( box_num )


    def getVisibleBoxNum( self, box_num ):
        """
        Return the box's index number within its level.  Arg box_num is the
        index into the flattened hierarchy -- all the "pieces".
        """
        return self.reader.GetVisibleBoxNum( box_num )


    def getBoxExtents( self, level, box, permuted_in_reslice_mode=1 ):
        """
        Returns a dictionary whose keys are 'i', 'j' and 'k'.  For each key, the
        value is a pair indicating the low and high cell coordinate in that
        dimension.
        In reslice mode, the coordinates are permuted.
        """
        anag_utils.funcTrace()
        origin = self.reader.GetBoxOrigin( level, box )
        size = self.reader.GetBoxDims( level, box )

        result = {}
        result['i'] = ( origin[0], size[0]+origin[0]-1 )
        result['j'] = ( origin[1], size[1]+origin[1]-1 )
        result['k'] = ( origin[2], size[2]+origin[2]-1 )

        if self.isResliceMode() and (permuted_in_reslice_mode==1):
            temp = [result['i'], result['j'], result['k']]
            temp = self.permuteCoords( temp, self.getResliceDirection() )
            result = {} # Gives temp ownership of tuples.
            result['i'] = temp[0]
            result['j'] = temp[1]
            result['k'] = temp[2]

        return result


    def getDomainExtentsXYZ( self, permute_in_reslice_mode=1,
                                   with_ghosts=0 ):
        """
        Delegates to ChomboReader::GetDomainExtentsXYZ(), but in reslice case
        also permutes the coordinates.
        In reslice mode, the coordinates are permuted.
        When optional arg with_ghosts==1, count ghost cells.
        """
        anag_utils.funcTrace()
        extents = list(self.reader.GetDomainExtentsXYZ())
        if with_ghosts==1:
            g_depth = self.getGhostCellsDepth()
            min_level = self.getMinVisibleLevel()
            dx = self.getLevelDx( min_level )
            for i in range(0,3):
                extents[i]   = extents[i]   - g_depth[i]*dx[i]
                extents[i+3] = extents[i+3] + g_depth[i]*dx[i]

        if self.isResliceMode() and permute_in_reslice_mode==1:
            direction = self.getResliceDirection()
            return ( self.permuteCoords( extents[0:3], direction )
                   + self.permuteCoords( extents[3:6], direction ))
        else:
            return extents


    def getComponentSerialNum( self, component_name ):
        anag_utils.funcTrace()
        return self.component_list.index( component_name )


    def getLevelNumBoxes( self, level ):
        """ Return number of boxes at indicated level """
        anag_utils.funcTrace()
        return self.reader.GetLevelNumBoxes( level )

    def getCoordinateOrigin( self ):
        """ HDF5 origin attribute """
        anag_utils.funcTrace()
        return self.reader.GetCoordinateOrigin()

    def getNumComponents( self ):
        anag_utils.funcTrace()
        return self.reader.GetNumComponents()

    def getNumEBs( self ):
        """
        Only knows about EBs defined the old way, with fraction-0, etc.
        """
        anag_utils.funcTrace()
        return self.reader.GetNumEBs()        

    def getNumParticles( self ):
        anag_utils.funcTrace()
        return self.reader.GetNumParticles()
    def getNumParticleComponents( self ):
        anag_utils.funcTrace()
        return self.reader.GetNumParticleComponents()
    def getParticleComponentRange( self, component_name ):
        anag_utils.funcTrace()
        return (self.reader.GetParticleComponentRangeMin(component_name),
                self.reader.GetParticleComponentRangeMax(component_name))

    #
    # There follow several functions that have to do with the layering we need
    # to do to make slices, isocontours and embedded boundaries show up properly
    # in 2D mode.
    #
    def computeLevelOffset( self, box_num, level=None ):
        """
        Returns the z-direction offset at which we render the slice that
        corresponds to a given level.

        If arg level==None, then we interpret box_num as that element of the
        flattened list of currently visible boxes (the list we loop over in the
        accumulator).  If arg level!=None, then we interpret box_num as that
        box of the indicated level (in which case arg box_num isn't even used).
        """

        # Obtain a scaling factor.  The purpose of this is very different,
        # between 2D and 3D modes.  In 2D mode the purpose is to put the layers
        # far enough apart so the zbuffer resolution thing doesn't bite us
        # (and then our fine- and coarse-level contours will appear to
        # intersect).  In 3D mode, we just want an amount of spacing that, when
        # control_slice sets vtk_data.slice_offset to its maximum of 1.0, the
        # offset slices are still somewhere near the boundaries of the problem
        # domain.
        if self.is2DMode():
            scaling_factor = 30 # FIXME: If there are more levels than that,
                                # it won't look good.
        else:
            scaling_factor = self.domain_max['x'] -\
                             self.domain_min['x'] # Choice of x is arbitrary

        if level == None:
            level = self.getReader().GetVisibleBoxLevel( box_num )
        dx0 = self.reader.GetLevelDx(0)[0]
        delta = (level - self.getMinVisibleLevel()) \
              * scaling_factor/(self.max_available_level+1)
        if self.is2DMode():
            multi = self.slice_offset * 1 * delta + self.slice_origin
        else:
            multi = self.slice_offset * dx0 * delta + self.slice_origin

        return multi


    # Custom setter.
    def setSliceOffset( self, x, force_update=None):
        anag_utils.funcTrace()
        if (x != self.slice_offset) or (force_update==1):
            self.slice_offset = x
            if x == 0:
                self.setAlwaysUseRealBoxes(0)
            else:
                self.setAlwaysUseRealBoxes(1)
            self.vtkUpdate( source = 'slice' )


    def setResliceDirection( self, direction ):
        if self.reader:
            self.reader.SetSlicingDirection( direction )
        self.reslice_direction.set( direction )


    def setReslicePosition( self, position, axis_list ):
        anag_utils.funcTrace()
        if self.reader:
            self.reader.SetSlicingPosition( position )
        self.reslice_position[axis_list[0]] = position
        # The axis_list business is an artifact of how such dictionary-setters
        # are defined in SelfControl (and we want to emulate that style if we're
        # defining our own setReslicePosition()).


    def unitTest(self):
        pass


    def reslicePositionIsInBox( self, level, box_num, visible_part_only=0 ):
        """
        Returns 1 if the reslice plane goes through the box indicated by
        arg the box-th box at the level-th level.
        Returns 0 otherwise.

        Arg visible_part_only is of relevance for non-cell-centered data.  When
        1, the extents are of the box that the user should see, i.e. with margin
        cells cut down the middle in the directions the data is not
        cell-centered.  When 0, the extents are of the whole box.
        """
        anag_utils.funcTrace()

        # Get the extents, but straight from the ChomboReader; we don't want
        # the coordinates permuted (as self.getVisibleBoxExtentsXYZ() would do).
        extents = self.reader.GetBoxExtentsXYZ( level, box_num,
                                                visible_part_only )

        axis = self.getResliceDirection()
        axis_num = 0 * (axis=='x') + 1 * (axis=='y') + 2 * (axis=='z')
        position = self.getReslicePosition([axis,])
        if extents[axis_num] <= position <= extents[3+axis_num]:
            return 1
        else:
            return 0


    def _getNumVisibleReslicePieces( self ):
        """
        Returns the number of pieces (i.e. boxes) that intersect the current
        slice, if we're in reslice mode.
        We want to know when this is zero, so we can bail out early from
        vtkUpdate().
        """
        anag_utils.funcTrace()
        assert( self.isResliceMode() )

        result = 0
        for i in range(0, self.reader.GetNumPieces() ):
            level   = self.getVisibleBoxLevel( i )
            box_num = self.getVisibleBoxNum( i )
            if self.reslicePositionIsInBox( level, box_num ):
                result = result + 1
        return result


    def permuteCoords( self, point, axis ):
        """
        Arg point is a triple.  We permute those three coordinates to what they
        would be if arg axis were the axis pointing out of the screen, rather
        than (by default) the z axis.

        We can't delegate to vtkChomboReader::PermuteCoords(), because in some
        cases the elements of arg point are themselves lists (as in
        self.getBoxExtents()).
        """
        anag_utils.funcTrace()

        permutation = {'x':(1,2,0), 'y':(2,0,1), 'z':(0,1,2)}[axis]
        result = ( point[permutation[0]],
                   point[permutation[1]],
                   point[permutation[2]] )
        return result

    def inversePermuteCoords( self, point, axis ):
        """
        Inverse to self.permuteCoords().
        """
        anag_utils.funcTrace()

        permutation = {'y':(1,2,0), 'x':(2,0,1), 'z':(0,1,2)}[axis]
        result = ( point[permutation[0]],
                   point[permutation[1]],
                   point[permutation[2]] )
        return result


    def planeIntersectsBox( self, level, box_num, plane_direction,
                                                  plane_position ):
        """
        Return 1 if the box (indicated by its level and number within that
        level) intersects the indicated plane.  Arg plane_position is in
        distance (i.e. not cell) units.

        Return None otherwise.
        """
        anag_utils.funcTrace()
        result = self.reader.PlaneIntersectsBox(
                    level, box_num, plane_direction, plane_position )
        return result


    def xyz2ijk( self, xyz, level ):
        """
        Given distance (xyz) coordinates within the domain box, returns cell
        index (ijk) coordinates.

        Ignores reslice-mode coordinate permutation.

        Arg xyz should be a 3-tuple.
        Return value is a 3-tuple.
        Calling function has to check that some element of result isn't exactly
        one plus the highest index in some direction.
        """
        anag_utils.funcTrace()

        centering = self.getDataCentering()
        dx = self.reader.GetLevelDx(level)
        dx0 = self.reader.GetLevelDx(0)
        centering_correction = [0,0,0]
        for i in (0,1,2):
            centering_correction[i] = 0.5*centering[i]*(dx[i]-dx0[i])

        result = []
        for i in (0,1,2):
            result.append( int((xyz[i] - self.getCoordinateOrigin()[i]
                                +centering_correction[i])\
            / self.reader.GetLevelDx(level)[i]))

        return tuple(result)


    def getBoxInfo( self ):
        """
        Returns a summary of the number of boxes and cells, by level.
        """
        anag_utils.funcTrace()
        result = ''

        n_cells = 0
        n_boxes = 0
        for level in range(0, self.getMaxAvailableLevel() + 1):
            n_level_boxes = self.reader.GetLevelNumBoxes( level )
            n_level_cells = 0
            for box in range(0, n_level_boxes ):
                extents_ijk = self.getBoxExtents( level, box )
                n_box_cells = (1 + extents_ijk['i'][1] - extents_ijk['i'][0]) \
                            * (1 + extents_ijk['j'][1] - extents_ijk['j'][0]) \
                            * (1 + extents_ijk['k'][1] - extents_ijk['k'][0])
                n_level_cells = n_level_cells + n_box_cells
        
            n_cells = n_cells + n_level_cells
            n_boxes = n_boxes + n_level_boxes
        
            result = result +\
                "level " + str(level) + " : " + str(n_level_boxes) + " boxes, "\
                + str(n_level_cells) + " data points.\n"
        
        result = result + "All levels: " + str(n_boxes) + " boxes, " +\
                 str(n_cells) + " data points."
        return result


    def getRangeInfo( self ):
        """
        Returns a summary of the range of every component.  Note this takes in
        only the *visible* ranges, i.e. those included by the currently visible
        levels.
        """
        anag_utils.funcTrace()

        result = ''
        gamuts = {}
        for comp in range(0, len(self.getComponentList())):
            comp_name = self.getComponentList()[comp]
            gamuts[comp] = (9E309,-9E309)
            for level in range(self.getMinVisibleLevel(),
                               self.getMaxVisibleLevel()+1):
                level_range = self.reader.GetLevelComponentRange(level, comp)
                gamuts[comp] = (min(gamuts[comp][0], level_range[0]),
                                max(gamuts[comp][1], level_range[1]))

            result = result + comp_name + ' '
            result = result + str(gamuts[comp])
            result = result + '\n'

        return result


    def getDataSummary( self ):
        """
        Combine getBoxInfo() and getRangeInfo().
        """
        anag_utils.funcTrace()

        # Internally, we work in cell-centered mode.  For display purposes,
        # however, we'll need to make a centering correction to the reported
        # domain max (but not the domain min).
        centering = self.getDataCentering()
        cell_centered_domain_max = self.getDomainMax()
        cell_centered_domain_min = self.getDomainMin()
        dx0 = self.reader.GetLevelDx(0)
        corrected_domain_max = [
            cell_centered_domain_max['x'] - centering[0]*dx0[0],
            cell_centered_domain_max['y'] - centering[1]*dx0[1],
            cell_centered_domain_max['z'] - centering[2]*dx0[2] ]
        if self.is2DMode() and not self.isResliceMode():
            corrected_domain_max[2] = 0
        corrected_domain_max = tuple(corrected_domain_max)
        domain_min = (
            cell_centered_domain_min['x'],
            cell_centered_domain_min['y'],
            cell_centered_domain_min['z'] )

        result = '**Box info**\n' + self.getBoxInfo() + '\n\n'  +\
                 '**Domain** : ' + str(domain_min) + ', '   +\
                                   str(corrected_domain_max) + '\n\n' +\
                 '**Component ranges** (at levels ' +\
                    str(self.getMinVisibleLevel()) + ' through ' +\
                    str(self.getMaxVisibleLevel()) + ')\n' + self.getRangeInfo()

        dx = []
        dt = []
        time = []
        for l in range( self.getMinAvailableLevel(),
                        self.getMaxAvailableLevel()+1 ):
            dx.insert(l, self.reader.GetLevelDx(l)[0])
            dt.insert(l, self.reader.GetLevelDt(l))
            time.insert(l, self.reader.GetLevelTime(l))
        result = result + '\n' + '**dx =' + str(tuple(dx))
        result = result + '\n' + '**dt =' + str(dt)
        result = result + '\n' + '**time =' + str(time)

        if self.getNumParticles() > 0:
            result = result + '\n\n' + "**Particles**"
            result = result + '\n    Number of particles: '\
                   + str(self.getNumParticles())
            for i in range(0, self.getNumParticleComponents()):
                name = self.reader.GetParticleComponentName(i)
                ranj = [self.getParticleComponentRange(name)[0],
                        self.getParticleComponentRange(name)[1]]
                result = result + '\n' + '    ' + name  + ': ' + str(ranj)

        return result


    def saveToHDF5( self, outfilename ):
        """
        Dump the current dataset to an hdf5 file.
        FIXME: in reslice mode, should dump the slice.
        """
        anag_utils.funcTrace()
        self.reader.DumpHDF5( outfilename )


    def setAlwaysUseRealBoxes( self, yes_no ):
        assert( (yes_no == 0)  or  (yes_no == 1) )
        self.always_use_real_boxes = yes_no
        self.reader.SetAlwaysUseRealBoxes( yes_no )


    def currentPieceIsPadded( self ):
        """
        Returns 1|0 as the current box (whose vtkStructuredData is in the
        ChomboReader's GetOutput()) is|isn't padded out with ghost cells.
        """
        anag_utils.funcTrace()
        return self.reader.CurrentPieceIsPadded()

    def currentPieceIsReal( self ):
        """
        Returns 1|0 as the current box (whose vtkStructuredData is in the
        ChomboReader's GetOutput()) is|isn't real (as opposed to subdivided).
        """
        anag_utils.funcTrace()
        return self.reader.CurrentPieceIsReal()

    
    def hideBox( self, level, box_num ):
        """
        Prevents the indicated _real_ box from being loaded and rendered.
        Works incrementally; if other boxes have previously been designated as
        hidden, they'll remain hidden after this.
        """
        anag_utils.funcTrace()
        self.reader.HideBox( level, box_num )
        self.setHideBoxesNotifier( (level, (box_num,) ) )

    def showBox( self, level, box_num ):
        """
        Undoes hideBox().
        """
        anag_utils.funcTrace()
        self.reader.ShowBox( level, box_num )
        self.setShowBoxesNotifier( (level, (box_num,) ) )

    def hideBoxes( self, level, box_tuple ):
        """
        Like hideBox(), but works on a whole collection of boxes.
        """
        anag_utils.funcTrace()
        vtk_array = vtk.vtkIntArray()
        vtk_array.SetNumberOfTuples( len(box_tuple) )
        i = 0
        for b in box_tuple:
            vtk_array.InsertValue( i, b )
            i += 1
        self.reader.HideBoxes( level, vtk_array )
        self.setHideBoxesNotifier( (level, box_tuple) )

    def showBoxes( self, level, box_tuple ):
        """
        Like showBox(), but works on a whole collection of boxes.
        """
        anag_utils.funcTrace()
        vtk_array = vtk.vtkIntArray()
        vtk_array.SetNumberOfTuples( len(box_tuple) )
        i = 0
        for b in box_tuple:
            vtk_array.InsertValue( i, b )
            i += 1
        self.reader.ShowBoxes( level, vtk_array )
        self.setShowBoxesNotifier( (level, box_tuple) )


    def hideAllBoxes( self, level ):
        """ See hideBox() """
        anag_utils.funcTrace()
        self.reader.HideAllBoxes( level )
        self.setHideBoxesNotifier(
            (level, range(0,self.reader.GetLevelNumRealBoxes(level))) )
        

    def showAllBoxes( self, level ):
        """ See showBox() """
        anag_utils.funcTrace()
        self.reader.ShowAllBoxes( level )
        self.setShowBoxesNotifier(
            (level, range(0,self.reader.GetLevelNumRealBoxes(level))) )


class _LocalVtkData( VtkData ):
    """
    This subclass of VtkData is for pipeline-specific control over current
    component and visible levels.
    
    LocalVtkData both isa VtkData and holds a VtkData -- the global one (passed
    in through dep_dict).  This class registers with the global one, so as to
    keep in sync with current component, visible levels etc, whenever the local
    one doesn't want to over-ride those.

    Classes like VtkIso, VtkFabTables, VtkGrid etc use an instance of this
    class, rather than the global VtkData, and thus gain independent control
    over their current component, visible levels, and policy regarding the use
    of only real boxes.  To obtain an instance of _LocalVtkData, these classes
    call VtkData.makeLocalVtkData() (i.e. they call this method on the instance
    of the global VtkData that they hold -- and that should also be the only
    call they make on the global VtkData).  The global VtkData, in its __init__,
    has already constructed a goodly number of _LocalVtkData instances;
    makeLocalVtkData() returns one of these instances from the pool.

    The reason  we need to create this pool is that it's important for all
    _LocalVtkData instances to be constructed before anything above VtkData in
    the class construction sequence (see ChomboVis.__init__).  This is because
    construction of any subclass of SelfControl (and _LocalVtkData as well as
    all the Vtk* and Control* classes are subclasses of SelfControl) enqueues
    that class on a list of Notifier callbacks -- the list for the Notifier
    SavedStates.state_dict.  When we restore a state, these callbacks are called
    in order (i.e. one for each instance of a subclass of SelfControl) and one
    of the things that happens is that each instance wipes out all its Notifier
    callbacks.  The pool approach is how we make sure that _LocalVtkData wipes
    out its Notifier callbacks *before* classes like VtkIso, VtkGrid etc, in
    their _refresh() methods, reregister with _LocalVtkData Notifiers.
    Otherwise we lose those reregistrations.  (An alternative would have been to
    do some re-ordering of the callbacks of SavedStates.state_dict.)
    """
    def __init__( self, dep_dict ):

        VtkData.__init__( self, dep_dict )
        # Doesn't do much, except the all-important call to SelfControl.__init__
        # which puts us nice and early on the list of callbacks registered with
        # SavedStates.state_dict.

        decls = anag_utils.Declarations('decls', 'dep_dict' )
        self.decls = anag_utils.Declarations( 'decls', instance=self )

        self.multi_instance_qualifier =\
            self_control.generateMultiInstanceQualifier()


    def defactoInit( self, share_visualizable_dataset ):
        """
        We put as little as possible in __init__ because that's called for
        every _LocalVtkData in VtkData's pool; some of them won't even be used.
        But if we get here, it means this _LocalVtkData is being returned from
        VtkData.makeLocalVtkData(), so now we need to make this instance fully
        operational.

        Arg share_visualizable_dataset is usually 1, which means we set up the
        ChomboReader to use the same VisualizableDataset already in use.  But
        when this arg is None, we actually construct a fully independent
        VisualizableDataset (albeit tied to the same hdf5 file).
        """
        anag_utils.funcTrace()

        self.reader = libVTKChomboPython.vtkChomboReader()
        if share_visualizable_dataset:
            self.reader.ShareFile( self.vtk_data.getReader() )
        else:
            self.reader.LoadFile( self.vtk_data.getInfileName() )
        assert( self.reader.SetOptimizationMode )

        self.reader.SetOptimizationMode( self.optimization_mode )

        self.setTextureEnabled( self.cmd_line.getTexture() )
        self.infile_name = self.cmd_line.getInfile()

        if self.getInstanceIdentifier() == 'RESLICED':
            anag_utils.warning( 'RESLICED' )

        # We go right onto _initForFirstHDF5() and _refresh();
        # we can't wait for SelfControl to call them for us, because before
        # that happens, classes relying on this LocalVtkData will be
        # trying to query its reader for data.
        VtkData._initForFirstHDF5( self )
        self.defactoInit_called = 1
        self._refresh()

        self.enableUpdate()


    def _initForFirstHDF5( self ):
        """
        This is only here because it's required.  We took care of everything
        by calling the base class' _initForFirstHDF5() in self.defactoInit().
        anag_utils.funcTrace()
        """
        pass


    def _refresh( self ):
        """
        The main activity of this _refresh() function is to pass through certain
        notifications.  If module A wants to notify module B, and they don't
        use the same local_vtk_data, then module A will have to set a Notifier
        on the global vtk_data.  Module B will register with its local_vtk_data
        (in line with our practice of using the local_vtk_data as much as
        possible, in favor of the global one).  So B's local_vtk_data will need
        to be notified by the global vtk_data.
        """
        anag_utils.funcTrace()

        if self.getInstanceIdentifier() == 'RESLICED':
            anag_utils.warning( 'RESLICED' )

        if not self.defactoInit_called:
            return

        self.registerCallback(
            self.vtk_data.getNotifierCurComponent(),
            self._setCurComponentIfFollowing )

        self.registerCallback(
            self.vtk_data.getNotifierLoadedFirstComponentNotifier(),
            lambda n, d2, self=self: self.setLoadedFirstComponentNotifier(n) )

        self.registerCallback(
            self.vtk_data.getNotifierMinVisibleLevel(),
            self._setMinVisibleLevelIfFollowing )
        self.registerCallback(
            self.vtk_data.getNotifierMaxVisibleLevel(),
            self._setMaxVisibleLevelIfFollowing )

        self.registerCallback(
            self.vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._setAlwaysUsePaddedBoxesIfFollowing )

        self.registerCallback(
            self.vtk_data.getNotifierResliceNotifier(),
            self._syncToReslicePosAndDirection )

        map( lambda notifier, self=self :
            self.registerCallback( notifier, self._cmappedRangeCallback ),
            self.vtk_data.getCmappedRangeMinNotifiers()
           +self.vtk_data.getCmappedRangeMaxNotifiers() )

        self.registerCallback(
            self.vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicFactorsNotifierCallback )

        self.registerCallback(
            self.vtk_data.getNotifierPickedParticle(),
            self._pickedParticleNotifierCallback )

        self.registerCallback(
            self.vtk_data.getNotifierNewComponentNotifier(),
            self._newComponentNotifierHandler )

        self.registerCallback(
            self.vtk_data.getNotifierShowBoxesNotifier(),
            self._showBoxesNotifierHandler )
        self.registerCallback(
            self.vtk_data.getNotifierHideBoxesNotifier(),
            self._hideBoxesNotifierHandler )

        if self.cur_component.get() == g_ebBlankCoveredName:
            self.cur_component.set( self.vtk_data.getCurComponent() )

        VtkData._refresh( self )


    def _showBoxesNotifierHandler( self, data, dummy ):
        """ Arg data has the structure (level, (boxes)) """
        anag_utils.funcTrace()
        self.showBoxes( data[0], data[1] )
    def _hideBoxesNotifierHandler( self, data, dummy ):
        """ Arg data has the structure (level, (boxes)) """
        anag_utils.funcTrace()
        self.hideBoxes( data[0], data[1] )


    def _pickedParticleNotifierCallback( self, particle_num, dummy ):
        anag_utils.funcTrace()
        self.setPickedParticle( particle_num )


    def _anisotropicFactorsNotifierCallback( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        factors = self.vtk_data.getAnisotropicFactors()
        self.setAnisotropicFactors( factors[0], factors[1], factors[2] )
        self.vtkUpdate( source = str(self.__class__) )


    def _cmappedRangeCallback( self, dummy1, dummy2 ):
        if self.getCurComponent():
            # Gotta check this, in case follow_global_curcomponent==0.
            self.setCmappedRangeMin( self.vtk_data.getCmappedRangeMin() )
            self.setCmappedRangeMax( self.vtk_data.getCmappedRangeMax() )


    def _syncToReslicePosAndDirection( self, dummy1, dummy2 ):
        """
        Callback registered with the global VtkData's self.reslice_notifier.
        Sets reslice direction and position in this class.
        We can't conveniently make VtkData.reslice_position a notifier, because
        it's already a dictionary, and we don't support auto-generation of
        accessors for dictionaries of notifiers.  That's why we have
        VtkData.reslice_notifier; listeners can register with that and then
        query reslice_position and reslice_direction to find out what happened.
        """

        self.setResliceDirection( self.vtk_data.getResliceDirection() )
        for a in 'x', 'y', 'z':
            self.setReslicePosition( 
                self.vtk_data.getReslicePosition([a,]), [a,] )
        self.setResliceNotifier(0)
        self.vtkUpdate( source = str(self.__class__) )


    #
    # Callbacks that cause LocalVtkData's visible levels and cur_component
    # to follow their global counterparts -- but only if that feature is turned
    # on through the self.follow_global_* state variables.
    #
    def _setCurComponentIfFollowing( self, comp_name, extra_info ):
        if self.follow_global_curcomponent == 1:
            self.setCurComponent( component_name=comp_name )
    def _setMinVisibleLevelIfFollowing( self, lev, extra_info ):
        if self.follow_global_levels == 1:
            self.setMinVisibleLevel( k=lev )
    def _setMaxVisibleLevelIfFollowing( self, lev, extra_info ):
        if self.follow_global_levels == 1:
            self.setMaxVisibleLevel( k=lev )
    def _setAlwaysUsePaddedBoxesIfFollowing( self, yes_no, extra_info ):
        anag_utils.funcTrace()
        if self.follow_global_padded_boxes == 1:
            self.setAlwaysUsePaddedBoxes( yes_no )
            self.vtkUpdate( source = str(self.__class__) )


    def setFollowGlobalLevels( self, on_off ):
        anag_utils.funcTrace()
        self.follow_global_levels = on_off
        if on_off == 1:
            update_is_disabled = self.update_is_disabled
            if not update_is_disabled:
                self.disableUpdate() # Just for a moment...
            if self.getMaxVisibleLevel() != self.vtk_data.getMaxVisibleLevel():
                self.setMaxVisibleLevel(self.vtk_data.getMaxVisibleLevel())
            if not update_is_disabled:
                self.enableUpdate()  # ...there!
            if self.getMinVisibleLevel() != self.vtk_data.getMinVisibleLevel():
                self.setMinVisibleLevel(self.vtk_data.getMinVisibleLevel())

    def setFollowGlobalCurcomponent( self, on_off ):
        anag_utils.funcTrace()
        self.follow_global_curcomponent = on_off
        if on_off == 1:
            if self.getCurComponent() != self.vtk_data.getCurComponent():
                self.setCurComponent( self.vtk_data.getCurComponent() )

    def setFollowGlobalPaddedBoxes( self, on_off ):
        anag_utils.funcTrace()
        self.follow_global_padded_boxes = on_off
        if on_off == 1:
            if( self.getAlwaysUsePaddedBoxes()
            !=  self.vtk_data.getAlwaysUsePaddedBoxes() ):
                self.setAlwaysUsePaddedBoxes(
                    self.vtk_data.getAlwaysUsePaddedBoxes() )
