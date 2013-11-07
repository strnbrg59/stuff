#
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

# File: selector.py
# Author: TDSternberg
# Created: 6/12/01

import algorithms
import anag_utils
from self_control import SelfControl
import vtk_line_segment

import libVTKChomboPython
import vtkpython

#g_timer = anag_utils.Timer()
g_light_green = (0.53, 0.83, 0.53 )
g_light_blue = (0.5, 0.5, 1.0 )
g_decls = anag_utils.Declarations( 'g_decls', 'g_light_green', 'g_light_blue' )


class Selector( SelfControl ):
    """
    A skewer and highlight boxes wherever the skewer intersects them.
    """

    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        SelfControl.__init__( self,dep_dict,
            metadata = [
                { 'name':'box' },  # A box.
                { 'name':'cell' }, # A cell within the box (2D only).
                { 'name':'ray' },  # The skewer.
                { 'name':'selected_boxes', 'initval':None }, # _BoxInfo's.
                { 'name':'selected_box_index' }, # For cycling.  An integer.
                { 'name':'local_vtk_data'}
            ])
        decls = anag_utils.Declarations( 'self', 'decls', 'dep_dict' )
        self.decls = anag_utils.Declarations( "decls", instance=self )

        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        self.local_vtk_data = self.vtk_fab_tables.getLocalVtkData()

        self.box = _HiliteBox( dep_dict =
                        { 'saved_states':self.saved_states,
                          'vtk_vtk':self.vtk_vtk,
                          'local_vtk_data':self.local_vtk_data } )
        self.box.setColor( g_light_green )

        self.cell = _HiliteBox( dep_dict =
                        { 'saved_states':self.saved_states,
                          'vtk_vtk':self.vtk_vtk,
                          'local_vtk_data':self.local_vtk_data } )
        self.cell.setColor( g_light_blue )

        self.ray = _Skewer( dep_dict = 
                        { 'saved_states':self.saved_states,
                          'vtk_vtk':self.vtk_vtk })


        # self.selected_boxes is initialized in self._setSelectedBoxes().
        # self.selected_box_index is initialized in self.turnOn().


    def _refresh( self ):
        anag_utils.funcTrace()

        # When vtk_data.reslice_direction changes, move the position of the
        # green box to reflect the new orientation of the axes.
        self.registerCallback(
            self.local_vtk_data.getNotifierResliceNotifier(),
            self._handleResliceDirection )

        # When a new databrowser pops up, cycle through the entire set of
        # visible levels.  This is the best way I know to arrange for the
        # selected cell to get highlighted.  It's a workaround for a Pmw bug
        # that causes the scrollbar components of Pmw.ScrolledText widgets to
        # not come up in a consistent state.
        self.registerCallback(
            self.local_vtk_data.getNotifierNewDatabrowser(),
            lambda dummy1, dummy2, self=self: self.cycle( increment=0 ) )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.box.cleanup()
        self.cell.cleanup()
        self.ray.cleanup()


    def _handleResliceDirection( self, direction, dummy2 ):
        """
        Callback registered with vtk_data.reslice_direction.
        Reorients position of highlight box, to put it in line with the axes'
        orientations in the new reslice direction.
        """
        anag_utils.funcTrace()
        if self.ray.getIsOn():
            box_info = self.selected_boxes[ self.selected_box_index ]

            direction = self.local_vtk_data.getResliceDirection()
            position  = self.local_vtk_data.getReslicePosition([direction])
            if( self.local_vtk_data.planeIntersectsBox(
                    box_info.getLevel(), box_info.getBoxNum(),
                    direction, position )):
                #anag_utils.info( "plane intersects at",
                #    (box_info.getLevel(), box_info.getBoxNum()))
                self.box.show( box_info.getExtents() )
            else:
                self.box.turnOff()

            self.cell.turnOff()


    def turnOn( self, x, y ):
        """
        Args (x,y) are the coordinates, in the plane of the display, where the
        mouse was when the user fired the selector ray.

        This function is called when the user does a ctrl-left-button in the
        Vtk display window.
        """
        anag_utils.funcTrace()

        self.ray.findEndpoints( origin={'x':x, 'y':y} ) # Sets ray.endpoints.
        self._setSelectedBoxes() # Finds skewered boxes.  Uses ray.endpoints.

        if len( self.selected_boxes ) > 0:
            self.ray.turnOn( self.ray.getEndpoints() )
            self.selected_box_index = -1 # cycle() will up it to 0 now...
            (selected_cell, selected_box_contains_selected_cell) = self.cycle()
            if selected_cell:
                i_box = 0
                while ( (i_box < len(self.selected_boxes))
                    and selected_cell
                    and (not selected_box_contains_selected_cell) ):
                        (selected_cell,
                         selected_box_contains_selected_cell) = self.cycle()
                        i_box+=1
        else:
            self.ray.turnOff()


    def _setSelectedBoxes( self ):
        """
        Figure out which boxes, on the currently visible levels, the selector
        has skewered.

        Side effect: sets m_selected_boxes to a list of _BoxInfo objects.

        This replaces vtkChomboReader::IntersectActiveBoxes() and all its
        support paraphernalia.  Unfortunately, it's also slower in Python, and
        that makes a difference when the number of boxes is large, like in the
        tens of thousands.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'level', 'box_num',
            'box_extents', 'face_intersections',
            'comparisonFunc', 'ray_endpoints', 'distance_to_box',
            'ray_near_endpoint', 'ray_far_endpoint', 'box_lo_corner',
            'box_hi_corner','b', 'box'
            )

        self.selected_boxes = []

        ray_endpoints = self.ray.getEndpoints()
        ray_near_endpoint = ray_endpoints[0:3]
        ray_far_endpoint  = ray_endpoints[3:6]

        for level in range( self.local_vtk_data.getMinVisibleLevel(),
                            self.local_vtk_data.getMaxVisibleLevel() + 1 ):
            for box in range( 0, self.local_vtk_data.getLevelNumBoxes(level) ):
                box_extents = self.local_vtk_data.getBoxExtentsXYZ(level,box,0)
                box_lo_corner = list(box_extents[0:3])
                box_hi_corner = list(box_extents[3:6])
    
                # We spend almost all our time in algorithms.distanceToBox().
                distance_to_box = algorithms.distanceToBox(
                    ray_near_endpoint, ray_far_endpoint, # two points on the ray
                    box_lo_corner, box_hi_corner )       # box corners
    
                if( ( distance_to_box != None)  # None means, no intersection.
                and (   ( not self.local_vtk_data.isResliceMode() )
                     or ( anag_utils.getDebugLevel() >= 4 )
                     or ( self.local_vtk_data.reslicePositionIsInBox(level,
                                                                     box )))):
                    self.selected_boxes.append(
                        _BoxInfo(level=level, box_num=box,
                                 distance=distance_to_box,
                                 local_vtk_data=self.local_vtk_data))

        # Sort the boxes, first by decreasing level, second by increasing
        # distance.
        def comparisonFunc( a,b ):
            if   a.level > b.level:       return -1
            elif a.level < b.level:       return  1
            elif a.distance > b.distance: return  1
            elif a.distance < b.distance: return -1
            else:                         return 0
        self.selected_boxes.sort( comparisonFunc )

        decls.memberFunctionAudit( self )


    def _getSelectedCell( self, level, box, ray_endpoints ):
        """
        Return the (i,j,k) coordinates (within the box) and the extents of the
        precise cell under the selector.

        Thus the return value is a dictionary with keys 'cell_ijk' and
        'extents'.

        Returns None if skewer misses the domain, or (in 3D) no planes are lit
        up.

        In 3D, if the selector skewers more than one slicing plane, we select a
        cell on the nearest (visible) plane skewered.

        Reslice mode is treated as though it's ordinary 2D; the third index of
        cell_ijk is always set to zero, and the first two indices correspond
        to the directions that are "virtual x" and "virtual y" in the reslice
        orientation.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self', 'level', 'box',
            'ray_endpoints', 'dx', 'offset_corrected_frac',
            'offset_corrected_relative',
            'axis2ndx', 'box_extentsXYZ', 'box_extentsIJK',
            'frac', 'relative', 'absolute', 'ndx', 'ndx_num', 'axis',
            'axis_num', 'axis_nums', 'ndx2axis', 'dist_to_plane',
            'dist_to_nearest_plane', 'num_dims',
            'cell_extents', 'direction', 'level_offset', 'plane', 'overshot',
            'intersection', 'plane_position', 'nearest_visible_plane',
            'domain_extents' )
        #anag_utils.info( "ray_endpoints=",
        #                 map( lambda x, f=algorithms.prettyRound: f(x,2),
        #                      ray_endpoints ))

        if level == -1: # Selector is outside domain box.  Invalid.
            return None

        axis2ndx = {'x':'i', 'y':'j', 'z':'k'}
        ndx2axis = {'i':'x', 'j':'y', 'k':'z'}
        ndx_num = {'i':0, 'j':1, 'k':2}
        axis_num = {'x':0, 'y':1, 'z':2}

        box_extentsXYZ = list(self.local_vtk_data.getBoxExtentsXYZ(
                                level, box, 0))
        box_extentsIJK = self.local_vtk_data.getBoxExtents( level, box )

        level_offset = self.local_vtk_data.computeLevelOffset( box_num=box,
                                                               level=level )

        intersection = {}
        if self.local_vtk_data.is2DMode():
            plane_position = 0
            nearest_visible_plane = 'z'
            intersection['z'] = {}
            for axis in 'x','y','z':
                intersection['z'][axis] = algorithms.linePlaneIntersection(
                        ray_endpoints[0:3], ray_endpoints[3:6],
                        'z', plane_position, axis )
        else:
            # Find nearest visible plane that's skewered.  We need to loop over
            # the axes in an outer and an inner loop.  In the outer loop we call
            # the index "plane", in the inner one, "axis".
            nearest_visible_plane = None
            dist_to_nearest_plane = None
            overshot = {'x':0, 'y':0, 'z':0}
            for plane in 'x', 'y','z':
                if( not self.vtk_slice.getSlicingPlanes()[
                                plane+'0'].getSliceIsVisible() ):
                    continue
                plane_position =\
                    self.vtk_slice.getSlicingPlanes()[plane+'0'
                                                     ].getPlanePosition()\
                  + level_offset
                
                intersection[plane] = {}
                dist_to_plane = 0.0
                axis_nums = {'x':0, 'y':1, 'z':2}
                num_dims = self.local_vtk_data.getNumDims()
                domain_extents =\
                    self.local_vtk_data.getDomainExtentsXYZ(with_ghosts=1)
                for axis in 'x','y','z':
                    intersection[plane][axis] =\
                         algorithms.linePlaneIntersection(
                            ray_endpoints[0:3], ray_endpoints[3:6],
                            plane, plane_position, axis )
                    if((intersection[plane][axis] <
                        domain_extents[axis_nums[axis]])
                    or( intersection[plane][axis] >
                        domain_extents[axis_nums[axis]+num_dims])):
                        overshot[plane] = 1

                    dist_to_plane = dist_to_plane \
                                  + pow( ray_endpoints[axis_num[axis]] -
                                         intersection[plane][axis], 2.0 )
                if( (  ( dist_to_nearest_plane == None )
                    or ( dist_to_plane < dist_to_nearest_plane ) )
                and ( overshot[plane] == 0 )):
                    dist_to_nearest_plane = dist_to_plane
                    nearest_visible_plane = plane

            if nearest_visible_plane == None:  # ==> No planes visible
                return None

        frac = {}
        offset_corrected_frac = 0
        # The offset_corrected_* variables are for determining the cell's
        # actual position in its box -- for use by control_fab_table.
        for axis in 'x', 'y', 'z':
            frac[axis] = (
                (  intersection[nearest_visible_plane][axis] 
                 - box_extentsXYZ[axis_num[axis]] )
              / (  box_extentsXYZ[3+axis_num[axis]] 
                 - box_extentsXYZ[axis_num[axis]] ))
            if axis == nearest_visible_plane:
                offset_corrected_frac = (
                    (  intersection[nearest_visible_plane][axis] 
                     - level_offset
                     - box_extentsXYZ[axis_num[axis]] )
                  / (  box_extentsXYZ[3+axis_num[axis]] 
                     - box_extentsXYZ[axis_num[axis]] ))

        relative = {}
        absolute = {}
        offset_corrected_relative = 0
        for ndx in 'i','j','k':
            relative[ndx] = int(frac[ndx2axis[ndx]] *
                        (1+box_extentsIJK[ndx][1]-box_extentsIJK[ndx][0]))
            absolute[ndx] = box_extentsIJK[ndx][0] + relative[ndx]
            if ndx == axis2ndx[nearest_visible_plane]:
                offset_corrected_relative = \
                    int(offset_corrected_frac *
                        (1+box_extentsIJK[ndx][1]-box_extentsIJK[ndx][0]))
                absolute[ndx] = box_extentsIJK[ndx][0] +\
                                offset_corrected_relative

        if( self.local_vtk_data.is2DMode()
        and not self.local_vtk_data.isResliceMode() ):
            absolute['k'] = 0

        # Cell extents are for hiliting in the VTK window.
        cell_extents = [0,0,0,0,0,0]
        dx = self.local_vtk_data.getReader().GetLevelDx(level)
        if self.local_vtk_data.isResliceMode():
            direction = self.local_vtk_data.getResliceDirection()
            dx = self.local_vtk_data.permuteCoords( dx[:], direction )[:]
        
        for ndx in 'i', 'j', 'k':
            cell_extents[ ndx_num[ndx]] =\
                box_extentsXYZ[ndx_num[ndx]] + relative[ndx] * dx[ndx_num[ndx]]
            cell_extents[3+ndx_num[ndx]] = cell_extents[ndx_num[ndx]] +\
                dx[ndx_num[ndx]]

        decls.memberFunctionAudit( self )
        return {'cell_ijk':(absolute['i'],absolute['j'],absolute['k']),
                'extents':cell_extents}


    def turnOff(self):
        """
        Turn off the ray and the highlighted boxes.  This is called when the
        user does a ctrl-middle-button, or fires a selector into a region where
        there are no boxes.
        """
        anag_utils.funcTrace()
        self.ray.turnOff()
        self.box.turnOff()
        self.cell.turnOff()

    
    def cycle( self, increment=1 ):
        """
        Move the highlight to the next box skewered by the selector ray.
        When reach end, go back to first box.  Boxes are highlighted in order:
        first, by decreasing level; second, by increasing distance from the
        user's eyes.  But in 3d, we go straight to the first box that
        intersects a plane, so we highlight the picked cell right away.
        This is triggered by a ctrl-right-button.

        Returns a tuple whose first element indicates whether the ray has
        intersected any cell on a visible plane, and whose second element
        indicates whether the box that contains that cell is now highlighted.
        """
        anag_utils.funcTrace()

        if (self.selected_boxes==None) or (len(self.selected_boxes) == 0):
            return (None,None)

        self.selected_box_index = ( (self.selected_box_index + increment)
                                   % len(self.selected_boxes) )

        box_info = self.selected_boxes[ self.selected_box_index ]
        endpoints = self.ray.getEndpoints()
        selected_cell = self._getSelectedCell( box_info.getLevel(),
                                               box_info.getBoxNum(),
                                               endpoints )
        selected_box_contains_selected_cell = None

        if self.ray.getIsOn():
            self.box.show( box_info.getExtents() )

            box_extents = self.local_vtk_data.getBoxExtentsXYZ(
                box_info.getLevel(), box_info.getBoxNum(), 0)
            if( self.local_vtk_data.is2DMode() 
            or (  ( selected_cell != None )
              and ( algorithms.box1ContainsBox2( box_extents,
                                                 selected_cell['extents'] )))):
                self.cell.show( selected_cell['extents'] )
                selected_box_contains_selected_cell = 1
            else:
                self.cell.turnOff()

        # This triggers a callback in the databrowser launcher:
        self.local_vtk_data.setSelectedBox( (box_info.getLevel(),
                                             box_info.getBoxNum()) )

        # This triggers a callback in the data browser (causing it to scroll
        # so the selected cell is visible):
        if selected_cell:
            cell_ijk = list(selected_cell['cell_ijk'])
            self.local_vtk_data.setSelectedCell( cell_ijk )

        return (selected_cell, selected_box_contains_selected_cell)
                

class _HiliteBox( SelfControl ):
    """
    A highlight box we put around the selected box or cell.
    """
    def __init__( self, dep_dict ):
        SelfControl.__init__( self, dep_dict, metadata =
          [
            { 'name':'source' },
            { 'name':'actor' },
            { 'name':'property' }
          ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )

        self.source = libVTKChomboPython.vtkBBoxSource()
        mapper = self.vtk_vtk.newMapper()
        self.actor = self.vtk_vtk.newActor()
        self.property = self.vtk_vtk.newProperty()
        
        self.property.SetLineWidth( 3.0 )
        self.actor.SetProperty( self.property )
        mapper.SetInput( self.source.GetOutput() )
        mapper.ImmediateModeRenderingOn()
        self.actor.SetMapper( mapper )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.turnOff()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def setColor( self, rgb ):
        apply( self.property.SetColor, rgb )


    def turnOff( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.removeActor( self.actor )


    def show( self, extents ):
        """
        Highlight the indicated box.  When in reslice mode, it's up to the
        caller to make sure the extents coordinates are appropriately permuted.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 
            'decls', 'extents', 'extents_lo', 'extents_hi' )

        extents_lo = list(extents[0:3])
        extents_hi = list(extents[3:6])

        self.source.SetBounds( extents_lo[0], extents_hi[0],
                               extents_lo[1], extents_hi[1],
                               extents_lo[2], extents_hi[2] )
        
        self.vtk_vtk.addActor( self.actor )

        decls.memberFunctionAudit(self)


class _Skewer( SelfControl, vtk_line_segment.VtkLineSegment ):
    """ The skewer """
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()

        SelfControl.__init__( self, dep_dict, metadata =
          [
            {'name':'builder'},
          ] )

        vtk_line_segment.VtkLineSegment.__init__( self, color=(1,1,1),
                                                        dep_dict=dep_dict)

        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'dep_dict',
            'mapper' )

        self.builder = libVTKChomboPython.vtkRayBuilder()
        self.builder.SetViewport( self.vtk_vtk.getRenderer() )

        self.setColor((1,1,1))
        
        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.turnOff()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def findEndpoints( self, origin ):
        """
        We need to compute the endpoints here, once and for all.  Otherwise,
        when you zoom in and out, a given origin gets you different endpoints.
        The result is that the little blue cell box moves away from the correct
        cell (!).
        """
        anag_utils.funcTrace()
        self.endpoints = self.builder.GetRay( origin['x'], origin['y'] )
        # That's two points on the selector ray.  The format is
        # [x_0,y_0,z_0,x_1,y_1,z_1].



class _BoxInfo( SelfControl ):
    """
    Information on boxes -- we cache a collection of these to keep track of
    what's skewered.

    Not to be confused with class _HiliteBox.
    """
    def __init__( self, level, box_num, distance, local_vtk_data ):
        anag_utils.funcTrace()
        SelfControl.__init__( self, dep_dict={},
             metadata = [
                { 'name':'level', 'get':1 },
                { 'name':'box_num', 'get':1 },
                { 'name':'extents', 'get':2 },
                { 'name':'distance', 'get':1 }, # From source of selector ray.
                { 'name':'local_vtk_data' }
            ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'level', 'box_num',
            'extents', 'distance', 'local_vtk_data' )

        self.level = level
        self.box_num = box_num
        self.distance = distance
        self.local_vtk_data = local_vtk_data

        decls.memberFunctionAudit( self )


    def getExtents( self ):
        """
        Returns the extents of this box.
        We call vtk_data.getVisibleBoxExtentsXYZ(), rather than cache the
        extents once and for all, because in reslice mode we have to make sure
        we return the extent coordinates appropriately permuted.
        """
        anag_utils.funcTrace()
        return self.local_vtk_data.getBoxExtentsXYZ( self.level,self.box_num,1 )

    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()
