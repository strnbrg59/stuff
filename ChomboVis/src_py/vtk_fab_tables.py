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

# File: vtk_fab_tables.py
# Author: TDSternberg
# Created: 9/17/01

"""
Vtk-level functionality for the databrowser: a highlight around the selected
box, and a semi-transparent plane at the position corresponding to the numbers
in the databrowser.

The main class here is really just a factory; each instance it creates owns
one highlight box, one plane and one array of databrowser data.
"""

import anag_utils
from self_control import SelfControl

import vtkpython
import libVTKChomboPython

g_decls = anag_utils.Declarations( 'g_decls', 'g_red', 'g_pink', 'g_dark_red', )
g_red = (1.0,0.0,0.0)
g_pink = (1.0,0.6,0.6)
g_dark_red = (0.7,0.1,0.1)


class VtkFabTables( SelfControl ):
    
    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        SelfControl.__init__( self, dep_dict, metadata=
          [ {'name':'constructed_tables', 'initval':{}},
                # Keys are (level,box) tuples, values are references to
                # instances of class _OneVtkFabTable.  Don't ever, ever delete
                # the elements of this dictionary, as each one has registered
                # an instance function as a callback to
                # vtk_data.reslice_direction.
            { 'name':'local_vtk_data', 'get':1 },
            { 'name':'show_ghost_cells', 'get':1, 'set':2, 'save':1}
          ] )
        self.decls = anag_utils.Declarations( "decls", instance=self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        self.constructed_tables = {}
        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=1,
            follow_global_curcomponent=1,
            follow_global_padded_boxes=1,
            always_use_real_boxes=1,
            instance_identifier='vtk_fab_tables' )

        # We need to be synched with the visible levels.  We don't care
        # about the current component, but might as well track that too or we'll
        # hold references to FABs that could just as well be freed.
        self.setShowGhostCells( self.local_vtk_data.getAlwaysUsePaddedBoxes() )


    def _refresh( self ):
        anag_utils.funcTrace()
        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )


    def cleanup( self ):
        anag_utils.funcTrace()
        for i in self.constructed_tables.keys():
            self.vtk_vtk.removeActor( self.constructed_tables[i].hilite_box )
            self.vtk_vtk.removeActor( self.constructed_tables[i].plane )
            self.constructed_tables[i].cleanup()


    def _paddedBoxesCallback( self, on_off, dummy ):
        """ Callback for local_vtk_data.always_use_padded_boxes. """
        anag_utils.funcTrace()
        self.setShowGhostCells( on_off )


    def setShowGhostCells( self, on_off ):
        anag_utils.funcTrace()
        if( on_off != self.show_ghost_cells ):
            self.show_ghost_cells = on_off
            self.local_vtk_data.setAlwaysUsePaddedBoxes( on_off )


    def newFabTable( self, component, level, box ):
        anag_utils.funcTrace()
        if( self.constructed_tables.has_key( (component,level,box) )
        and (self.constructed_tables[(component,level,box)].getShowGhosts()
             == self.show_ghost_cells )):
            obj = self.constructed_tables[(component,level,box)]
            obj.setHighlightingActivated( 1 )
        else:
            obj = _OneVtkFabTable( 
                dep_dict={ 
                    'saved_states':self.saved_states,
                    'cmd_line':self.cmd_line,
                    'local_vtk_data':self.local_vtk_data,
                    'vtk_vtk':self.vtk_vtk },
                level=level, box=box, show_ghosts=self.show_ghost_cells )
            self.constructed_tables[(component,level,box)] = obj
        return obj


class _OneVtkFabTable( SelfControl ):
    """
    VtkFabTables produces one instance of this class for every databrowser.

    This class owns one highlight box, one plane and one array of
    databrowser data.
    """

    def __init__( self, dep_dict, level, box, show_ghosts ):
        anag_utils.funcTrace()        
        SelfControl.__init__( self, dep_dict=dep_dict, metadata=[
            {'name':'hilite_box'},
            {'name':'hilite_box_source'},
            {'name':'hilite_box_prop'},
            {'name':'plane'},
            {'name':'plane_source'},
            {'name':'level', 'get':1},
            {'name':'box', 'get':1},
            {'name':'box_origin'}, # lower left-hand corner
            {'name':'box_size'},   # corner across longest diagonal.
            {'name':'box_extents', 'initval':(0,0,0,0,0,0)},
            {'name':'actor_status'},
            {'name':'highlighting_activated', 'initval':1, 'set':1},
            {'name':'transparent_plane_toggle', 'initval':1},
            {'name':'show_ghosts', 'get':1}
          ])
        self.decls = anag_utils.Declarations( "decls", instance=self )
        self.level = level
        self.box = box
        self.show_ghosts = show_ghosts

        self.box_origin =\
            self.local_vtk_data.getReader().GetBoxOrigin( level, box)
        self.box_size =\
            self.local_vtk_data.getReader().GetBoxDims( level, box )

        if not self.cmd_line.getNoVtk():
            self._initVis()

        # Putting this here because we never get to _initForFirstHDF5(), much
        # less to _refresh(), in this class.
        if self.local_vtk_data.isResliceMode():
            self.registerCallback(
                self.local_vtk_data.getNotifierResliceDirection(),
                self._adaptToNewResliceDirection )
            self.registerCallback(
                self.local_vtk_data.getNotifierResliceNotifier(),
                self._adaptToNewReslicePosition )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()


    def cleanup( self ):
        anag_utils.funcTrace()


    def _initVis( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations('self', 'decls', 'extents',
            'hilite_box_mapper', 'plane_mapper', 'plane_prop' )

        self.hilite_box_source = libVTKChomboPython.vtkBBoxSource()
        hilite_box_mapper = self.vtk_vtk.newMapper()
        self.hilite_box = self.vtk_vtk.newActor()
        self.hilite_box_prop = self.vtk_vtk.newProperty()

        self.box_extents =\
            self.local_vtk_data.getBoxExtentsXYZ( self.level, self.box )

        self.hilite_box_source.SetBounds(
            self.box_extents[0], self.box_extents[3],
            self.box_extents[1], self.box_extents[4],
            self.box_extents[2], self.box_extents[5] )

        apply( self.hilite_box_prop.SetColor, g_pink )
        self.hilite_box_prop.SetLineWidth( 3.0 )
        self.hilite_box.SetProperty( self.hilite_box_prop )
        hilite_box_mapper.SetInput( self.hilite_box_source.GetOutput() )
        hilite_box_mapper.ImmediateModeRenderingOn()
        self.hilite_box.SetMapper( hilite_box_mapper )
        
        
        self.plane_source = libVTKChomboPython.vtkChomboPlaneSource()
        plane_mapper = self.vtk_vtk.newMapper()
        self.plane = self.vtk_vtk.newActor()
        plane_prop = self.vtk_vtk.newProperty()
        
        apply( plane_prop.SetColor, g_red )
        plane_prop.SetOpacity( 0.5 )
        self.plane.SetProperty( plane_prop )
        plane_mapper.SetInput( self.plane_source.GetOutput() )
        plane_mapper.ImmediateModeRenderingOn()
        self.plane.SetMapper( plane_mapper )

        decls.memberFunctionAudit(self)


    def _adaptToNewResliceDirection( self, dummy1, dummy2 ):
        """
        Callback for vtk_data.reslice_direction.
        Moves the position of self.hilite_box -- the pink box that outlines the
        box of the currently active _OneVtkFabTable -- to reflect the axis
        orientation under the newly-set value of vtk_data.reslice_direction.
        """
        anag_utils.funcTrace()

        # We don't need to use arg direction; vtk_data.getBoxExtentsXYZ()
        # obtains it on its own.
        self.box_extents = \
            self.local_vtk_data.getBoxExtentsXYZ( self.level, self.box )
        self.hilite_box_source.SetBounds(
            self.box_extents[0], self.box_extents[3],
            self.box_extents[1], self.box_extents[4],
            self.box_extents[2], self.box_extents[5] )


    def _adaptToNewReslicePosition( self, dummy1, dummy2 ):
        """
        Callback that fires whenever the reslice position changes.
        If we have a highlight box, we check if it intersects the reslice plane.
        If it does, we show the box (in its correct color -- g_pink for active,
        g_dark_red for inactive).  If no intersection, then we turn off the box.
        """
        anag_utils.funcTrace()

        if not self.actor_status:
            return

        direction = self.local_vtk_data.getResliceDirection()
        position  = self.local_vtk_data.getReslicePosition([direction])
        if( self.local_vtk_data.planeIntersectsBox( self.level, self.box,
                                              direction, position ) ):
            self.showActors( status = self.actor_status )
        else:
            self.showActors( status = 'off' )


    def getBoxData( self, component, axis, axis_position ):
        """
        Return one plane of data, that intersects axis at axis_position.

        We want the indices' directions to reflect the layout of a 2D rendering.
        For that reason, if, say the indices are i and j, we have i increasing
        across the top and j decreasing down the side.  (So that's how our
        return value is organized.)

        In the 2D-dataset case, we ignore args axis and axis_position.
        """
        anag_utils.funcTrace()
        #timer = anag_utils.Timer( label='getBoxData()' )

        reader = self.local_vtk_data.getReader()
        componentNum = self.local_vtk_data.getComponentList().index(component)
        reader.PinFAB( self.level, self.box, componentNum )

        n = {} # Number of cells along each axis.
        for a in 'x', 'y', 'z':
            n[a] = 1 + self.getKRange( a )[1] - self.getKRange( a )[0]

        result = []
        planes = { 'x':('z','y'),
                   'y':('x','z'),
                   'z':('y','x') }


        if( self.local_vtk_data.is2DMode()
        and not self.local_vtk_data.isResliceMode() ):
            r = range(0,n['y'])
            r.reverse()

            """
            #append_timer = anag_utils.Timer(label='append test')
            aresult = []
            #append_timer.on()
            for i in r:
                aresult.append([])
                for j in range(0,n['x']):
                    datum = 0
                    aresult[n['y'] - i - 1].append(datum)
            append_timer.stop()                         
            """

            #timer.on()

            for i in r:
                result.append([])
                for j in range(0,n['x']):
                    datum = reader.GetDatum(self.level, self.box, componentNum,
                                            j,i,0 )
                    result[n['y'] - i - 1].append(datum)

            #timer.stop()                         

        else:
            r = range(0, n[planes[axis][0]])
            r.reverse()

            """
            append_timer = anag_utils.Timer(label='append test')
            aresult = []
            append_timer.on()
            for i in r:
                aresult.append([])
                for j in range(0, n[planes[axis][1]]):
                    datum = 0
                    aresult[n[planes[axis][0]] - i - 1].append( datum )
            append_timer.stop()                         
            """

            #timer.on()
            for i in r:
                result.append([])
                for j in range(0, n[planes[axis][1]]):
                    k = axis_position - self.getKRange(axis)[0]
                    if   axis == 'x':
                        coords = (k,j,i)
                    elif axis == 'y':
                        coords = (i,k,j)
                    else:
                        coords = (j,i,k)
                    datum = apply( reader.GetDatum,
                                  (self.level, self.box, componentNum)+coords )
                    result[n[planes[axis][0]] - i - 1].append( datum )

            #timer.stop()

        reader.UnpinFAB( self.level, self.box, componentNum )
        return result


    def getKRange( self, axis ):
        """
        Return the min and max legal values for 'k', the position of the
        semitransparent plane.
        """
        assert( axis=='x' or axis=='y' or axis=='z' )
        axis_num = (axis=='y') + 2 * (axis=='z')
        
        lo = self.box_origin[axis_num]
        hi = self.box_size[axis_num] + lo - 1
        return (lo,hi)


    def togglePlane( self, on_off ):
        """ Turn the semi-transparent plane on or off.  Off=0, On=1. """
        anag_utils.funcTrace()
        if on_off == 0:
            self.vtk_vtk.removeActor( self.plane )
        else:
            self.vtk_vtk.addActor( self.plane )
        self.transparent_plane_toggle = on_off
        self.vtk_vtk.render()


    def setPlane( self, axis, position ):
        """ Set the position of the semi-transparent plane.
            Arg axis is the axis (x|y|z) that the plane intersects.
            Arg position is the position on that axis.
        """
        anag_utils.funcTrace()
        if self.cmd_line.getNoVtk():
            return

        assert( axis=='x' or axis=='y' or axis=='z' )
        axis_num = (axis=='y') + 2 * (axis=='z')

        if   axis=='x':
            other_mins = self.box_extents[1], self.box_extents[2] # miny, minz
            other_maxs = self.box_extents[4], self.box_extents[5] # maxy, maxz
        elif axis=='y':
            other_mins = self.box_extents[0], self.box_extents[2] # minx, minz
            other_maxs = self.box_extents[3], self.box_extents[5] # maxx, maxz
        elif axis=='z':
            other_mins = self.box_extents[0], self.box_extents[1] # minx, miny
            other_maxs = self.box_extents[3], self.box_extents[4] # maxx, maxy

        lo,hi = self.getKRange( axis )

        offset = self.box_extents[axis_num] +\
            (position - lo + 1.0) / (hi - lo + 1.0) *\
            (self.box_extents[3+axis_num] - self.box_extents[axis_num] + 0.0) -\
            self.local_vtk_data.getReader().GetLevelDx( self.level )[axis_num]\
             / 2.0

        # Three corners to determine the plane.
        corner = []
        corner.append( [other_mins[0], other_mins[1]] )
        corner.append( [other_maxs[0], other_mins[1]] )
        corner.append( [other_mins[0], other_maxs[1]] )
        for corner_index in 0,1,2:
            if   axis=='x':
                corner[corner_index].insert( 0, offset )
            elif axis=='y':
                corner[corner_index].insert( 1, offset )
            elif axis=='z':
                corner[corner_index].insert( 2, offset )

        apply( self.plane_source.SetOrigin, corner[0] )
        apply( self.plane_source.SetPoint1, corner[1] )
        apply( self.plane_source.SetPoint2, corner[2] )

        self.vtk_vtk.render()


    def showActors( self, status ):
        """
        Arg status can be 'off', 'dim', or 'on'.  These are the modes
        in which we can show the two actors -- the hilite_box and the plane.

        In 'dim' mode, the plane doesn't show at all, while the box is dimmed.

        A further state variable, self.highlighting_activated, governs whether
        the actors can be turned on at all.  This variable became necessary
        when we discovered that in twm and gnome (but not in olvwm and fvwm2)
        the "close" button turns off the pink outline but then it turns right
        back on because the wm detects more in-focus events.

        Saves status to self.actor_status.
        """
        anag_utils.funcTrace()
        assert( status=='off' or status=='dim' or status=='on' )
        
        if self.cmd_line.getNoVtk():
            return

        if self.highlighting_activated == 0:
            return

        self.actor_status = status


        if   status == 'off':
            self.vtk_vtk.removeActor( self.hilite_box )
            self.vtk_vtk.removeActor( self.plane )
            self.vtk_vtk.render()

        elif status == 'dim':
            self.vtk_vtk.addActor( self.hilite_box )
            self.vtk_vtk.removeActor( self.plane )
            apply( self.hilite_box_prop.SetColor, g_dark_red )
            self.vtk_vtk.render()

        elif status == 'on':
            self.vtk_vtk.addActor( self.hilite_box )
            if( ( not self.local_vtk_data.is2DMode() )
            and ( self.transparent_plane_toggle == 1 ) ):
                self.vtk_vtk.addActor( self.plane )
            apply( self.hilite_box_prop.SetColor, g_pink )
            self.vtk_vtk.render()

def generate_covered3( frac, xnorm, ynorm, znorm ):
    """
    Transformation function for reader_api.defineNewComponent.  Classifies
    cells (for EB purposes) as to regular, covered or irregular.
    Returns 0 if covered, 1 if irregular, 2 if regular.

    Has to be a module function or its pointer gets released or something
    and when we try to invoke it bad things happen.
    """
    anag_utils.funcTrace()
    if xnorm==0.0 and ynorm==0.0 and znorm==0.0:
        if frac==0.0:
            return 0  # covered cell
        else:
            return 2  # regular cell
    else:
        return 1      # irregular cell

def generate_covered2( frac, xnorm, ynorm ):
    """
    2D version of generate_covered3
    """
    anag_utils.funcTrace()
    if xnorm==0.0 and ynorm==0.0:
        if frac==0.0:
            return 0  # covered cell
        else:
            return 2  # regular cell
    else:
        return 1      # irregular cell
