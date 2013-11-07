##   _______              __
##  / ___/ /  ___  __ _  / /  ___
## / /__/ _ \/ _ \/  ' \/ _ \/ _ \
## \___/_//_/\___/_/_/_/_.__/\___/ 
##
## This software is copyright (C) by the Lawrence Berkeley
## National Laboratory.  Permission is granted to reproduce
## this software for non-commercial purposes provided that
## this notice is left intact.
## 
## It is acknowledged that the U.S. Government has rights to
## this software under Contract DE-AC03-765F00098 between
## the U.S. Department of Energy and the University of
## California.
##
## This software is provided as a professional and academic
## contribution for joint exchange.  Thus it is experimental,
## is provided ``as is'', with no warranties of any kind
## whatsoever, no support, no promise of updates, or printed
## documentation.  By using this software, you acknowledge
## that the Lawrence Berkeley National Laboratory and
## Regents of the University of California shall have no
## liability with respect to the infringement of other
## copyrights by any part of this software.
##

# File: vtk_iso_eb.py
# Author: TDSternberg
# Created: 2/16/05

import anag_utils
from self_control import SelfControl
from vtk_clip import g_clip_modes
import vtk_iso

import vtkpython
import libVTKChomboPython

import types

g_decls = anag_utils.Declarations( 'g_decls' )

class VtkIsoEb( SelfControl ):
    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        instance_data = [
            {'name':'is_visible', 'get':1, 'set':2, 'save':5, 'initval':0 },
            {'name':'levelset_component', 'initval':0, 'get':1, 'set':2,
                'save':5 },
            {'name':'fluid2', 'get':1, 'set':2, 'initval':None, 'save':1},
            {'name':'fluid2_slice_ids', 'initval':{}},
            {'name':'use_one_color', 'get':1, 'initval':1, 'save':5},  
                # If 1, shade embedded boundary with constant color
            {'name':'the_one_color', 'get':1, 'initval':(1.0,1.0,1.0),
              'save':5}, # For when use_one_color==1
            {'name':'normal_outward', 'get':1, 'set':1,'initval':0,'save':5},
                # 1=>outward, 0=>inward.
            {'name':'opacity', 'get':1, 'set':2, 'save':5, 'initval':1.0},
            {'name':'line_width', 'get':1, 'set':2, 'initval':1, 'save':1},
            {'name':'use_ghost_cells', 'get':1, 'set':2, 'save':1, 'initval':0},
            {'name':'blank_covered_cells', 'get':1, 'set':2, 'initval':0,
                'save':1},
            {'name':'local_vtk_data', 'get':1 },
            {'name':'local_vtk_iso'},
        ]
        SelfControl.__init__( self,dep_dict, instance_data)

        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'instance_data', 'dep_dict' )

        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):        
        anag_utils.funcTrace()
        if self.isOldStyle():
            return

        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=1,
            follow_global_curcomponent=1,
            follow_global_padded_boxes=1,
            always_use_real_boxes=0,
            instance_identifier='iso_eb' )

        self.local_vtk_iso = vtk_iso.VtkIso( dep_dict=
            {'saved_states':self.saved_states,
             'vtk_clip':self.vtk_clip,
             'vtk_vtk':self.vtk_vtk,
             'vtk_data':self.vtk_data,
             'vtk_cmap':self.vtk_cmap,
             'vtk_cameras':self.vtk_cameras},
            multi_instance_qualifier='iso_eb' )

        self.local_vtk_iso._initForFirstHDF5()
        self.local_vtk_iso.getLocalVtkData().setFollowGlobalCurcomponent(0)


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.isOldStyle():
            return

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if self.local_vtk_data.getCurComponent()==None:
            return   # Will come back here upon loading first component.

        self.registerCallback( self.local_vtk_data.getNotifierAccumStart(),
                               lambda d1, d2, self=self: self.updateVis() )

        self.setLevelsetComponent( self.levelset_component )
        self.local_vtk_iso._refresh()
        self.local_vtk_iso.setNumIsos( 1 )
        self.setUseGhostCells( self.use_ghost_cells )
        self.setFluid2( self.fluid2 )
        self.setIsVisible( self.is_visible )
        self.updateVis()
        if (self.blank_covered_cells == 1) and (self.vtk_iso.getIsClipped()==1):
            self.vtk_iso.setIsClipped(1)


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def cleanup( self ):
        anag_utils.funcTrace()


    def updateVis( self ):
        anag_utils.funcTrace()

        if( self.is_visible==0
        and self.blank_covered_cells==0 ):
            return

        if self.is_visible:
            self.local_vtk_iso.setCurComponent( self.levelset_component )
        self.local_vtk_iso.setMinIso( self.vtk_data.getIsocontourClipValue() )
        self.setUseGhostCells( self.use_ghost_cells )
        isovalue = self.vtk_data.getIsocontourClipValue()
        self.local_vtk_iso.changeContours(1, isovalue, isovalue)

        self.setBlankCoveredCells( self.blank_covered_cells )


    def isOldStyle( self ):
        """
        Returns True iff hdf5 file contains 'fraction-0' component.
        Shouldn't be called until there's an hdf5 file loaded.
        """
        anag_utils.funcTrace()
        return (    ('fraction-0' in self.vtk_data.getComponentList())
               and not self.cmd_line.getIsoEb() )


    def setUseGhostCells( self, yes_no ):
        """ Toggle ghost cells on eb-as-isocontour """
        anag_utils.funcTrace()
        self.use_ghost_cells = yes_no
        self.local_vtk_iso.setUseGhostCells( yes_no )
        

    def setIsVisible( self, yes_no ):
        anag_utils.funcTrace()
        self.is_visible = yes_no
        self.updateVis()
        self.local_vtk_iso.setDoShowIsosurfaces( yes_no )


    def hideFluid2( self ):
        for ax in self.fluid2_slice_ids.keys():
            plane = self.vtk_slice.getSlicingPlanes()[
                ax + self.fluid2_slice_ids[ax]]
            plane.toggleSliceVisibility(0)


    def setBlankCoveredCells( self, yes_no ):
        """
        Tell vtk_clip to clip against the EB.
        Individual pipelines (so far: just slice) have to also ask to be clipped
        before anything actually gets clipped.

        If yes_no==1, then self.levelset needs to be defined.
        """
        anag_utils.funcTrace()
        assert( yes_no==0 or yes_no==1 )

        self.blank_covered_cells = yes_no

        if yes_no == 0:
            self.hideFluid2()
            self.vtk_slice.setClipMode( g_clip_modes.none )
            self.vtk_iso.setClipMode(   g_clip_modes.none )
        else:
            if self.fluid2 == None:
                self.hideFluid2()
            else:
                if not 'z' in self.fluid2_slice_ids.keys():
                    self.fluid2_slice_ids['z'] = str(self.vtk_slice.newPlane(
                            'z', 
                            independent_vtk_data=True,
                            component = self.fluid2 ))
                new_z_plane = self.vtk_slice.getSlicingPlanes()[
                                              'z' + self.fluid2_slice_ids['z']]
                new_z_plane.local_vtk_data.setAlwaysUsePaddedBoxes(0)
                new_z_plane.setCurrentComponent( self.fluid2 )
                position = self.vtk_slice.getSlicingPlanes()[
                                                    'z0'].getPlanePosition()
                new_z_plane.movePlane( position )

                if self.vtk_data.is2DMode():
                    axes = ('z',)
                else:
                    axes = ('z','x','y')
                for ax in axes:
                    if ax != 'z':
                        if not ax in self.fluid2_slice_ids.keys():
                            self.fluid2_slice_ids[ax] = str(
                                self.vtk_slice.newPlane( ax,
                                    independent_vtk_data=False,
                                    component=self.fluid2,
                                    data_tied_to_plane=new_z_plane ) )
                    plane = self.vtk_slice.getSlicingPlanes()[
                                    ax + self.fluid2_slice_ids[ax]]
                    position = self.vtk_slice.getSlicingPlanes()[
                                                     ax+'0'].getPlanePosition()
                    plane.movePlane( position )
                    plane.setClipmodeInsideOut( 1 )
                    plane.toggleSliceVisibility(1)

            self.vtk_slice.setClipMode( g_clip_modes.eb,
                                        self.levelset_component )
            self.vtk_iso.setClipMode( g_clip_modes.eb )

            # In 3D, gotta give it another kick, for the updating to occur:
            if not self.vtk_data.is2DMode():
                self.vtk_slice.updateVis()
                self.vtk_slice.setClipMode(g_clip_modes.eb,
                                           self.levelset_component)


    def changeColor( self, use_one_color, rgb=None ):
        """
        Render the EB in a single color, or at least save that color
        for use as soon as self.use_one_color is set to 1.

        Arg use_one_color==1 means, color all values of the boundary with
        a single color -- specified by arg rgb.
        When use_one_color==0, color according to values of the global
        curcomponent.

        Arg rgb (a tuple) indicates a constant color to apply to all values
        on the boundaries.

        If use_one_color==0 but rgb!=None, that means save rgb for the next
        time use_one_color is set to 1.

        If use_one_color==1 but rgb==None, then use self.the_one_color (which
        happens to be the last rgb that was passed in here).
        """
            
        anag_utils.funcTrace()

        if rgb != None:
            self.the_one_color = rgb[:]
    
        if use_one_color == 0:
            self.use_one_color = 0
            self.local_vtk_iso.changeColor(
                self.local_vtk_data.getCurComponent() )
        else:
            self.use_one_color = 1
            self.local_vtk_iso.changeColor( colorer=vtk_iso.g_constant_color,
                                            rgb=rgb )
    

    def setLevelsetComponent( self, c ):
        anag_utils.funcTrace()
        if type(c) == type(2):
            c = self.local_vtk_data.getComponentList()[c]
        self.levelset_component = c

    def setFluid2( self, c ):
        anag_utils.funcTrace()
        if type(c) == type(2):
            c = self.local_vtk_data.getComponentList()[c]
        self.fluid2 = c


    def setLineWidth( self, n ):
        anag_utils.funcTrace()
        self.line_width = n
        self.local_vtk_iso.setLineWidth( n )


    def setOpacity( self, x ):
        anag_utils.funcTrace()
        assert( 0.0 <= x <= 1.0 )
        self.local_vtk_iso.setOpacity( x )


    def setDiffuse( self, x ):
        anag_utils.funcTrace()
    def setAmbient( self, x ):
        anag_utils.funcTrace()
    def setSpecular( self, x ):
        anag_utils.funcTrace()
    

    def unitTest( self ):
        anag_utils.funcTrace()
