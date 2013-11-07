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

# File: vtk_iso.py
# Author: TDSternberg
# Created: 6/01/01

import anag_utils
import algorithms
import vtk_glyphs
from self_control import SelfControl
from vtk_clip import g_clip_modes

import vtkpython
import libVTKChomboPython


g_decls = anag_utils.Declarations( 'g_decls', 'g_constant_color','g_self_color')
g_constant_color = '[constant]' # For cross-coloring
g_self_color     = '[self]'

class VtkIso( SelfControl ):
    
    def __init__(self, dep_dict, multi_instance_qualifier='iso'):
        anag_utils.funcTrace()
        instance_data = [
            { 'name':'do_show_isosurfaces', 'get':1, 'set':2, 'initval':0,
              'save':1 },
            { 'name':'the_constant_color', 'get':1, 'set':1,
              'initval':(1.0,1.0,1.0), 'save':1},
            { 'name':'line_width', 'get':1, 'set':2, 'save':1, 'initval':1},

            { 'name':'min_iso', 'initval':{}, 'get':2, 'set':2, 'save':1 },
            { 'name':'max_iso', 'initval':{}, 'get':2, 'set':2, 'save':1 },
            { 'name':'num_isos', 'initval':None, 'get':1, 'set':1, 'save':1 },
                # Min, max and number of rendered isocontours/surfaces.
                # Elements of min and max are floats; of num, integers.

            { 'name':'iso_func'},
            { 'name':'iso_accum'},
            { 'name':'iso_map', 'get':1},
            { 'name':'iso_prop', 'get':1},
            { 'name':'iso_trans'},
            { 'name':'iso_trans_p'},
            { 'name':'iso_block_plane'},
            { 'name':'iso_block_trans'},
            { 'name':'iso_block_trans_p'},
            { 'name':'iso_block_accum'},
            { 'name':'iso_block_map'},
            { 'name':'iso_block_actor', 'get':1 },
            { 'name':'iso_block_prop', 'get':1 },
            { 'name':'iso_actor', 'get':1, 'set':1},
            { 'name':'local_vtk_data', 'get':1 },
            { 'name':'crosscolor_vtk_data', 'get':1 },
            { 'name':'use_ghost_cells', 'get':1, 'set':2, 'save':1 },
            { 'name':'depthsorter'},
            { 'name':'glyphs'},
            { 'name':'show_glyphs', 'initval':0, 'set':2, 'save':1},
            { 'name':'called_init_for_first_hdf5', 'initval':0},

            # Stuff for cross-coloration:
            { 'name':'probe'},
            { 'name':'cast'},
            { 'name':'normals'},
            { 'name':'cross_colorer', 'get':1, 'save':1},
            { 'name':'is_clipped', 'get':1, 'set':2, 'initval':0, 'save':5},
            { 'name':'clip_mode', 'get':1, 'set':1,
                'initval':g_clip_modes.none },
            { 'name':'opacity', 'get':1, 'set':2, 'save':5, 'initval':1.0},
            { 'name':'multi_instance_qualifier'}
        ]
        SelfControl.__init__( self,dep_dict, instance_data)
        self.multi_instance_qualifier = multi_instance_qualifier
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls", "instance_data", "dep_dict" )


    def _initForFirstHDF5( self ):
        """
        Vtk functions that have something to do with iso, and their initial
          settings.
        Also, set up the iso pipeline.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'midpoint', 'v',
            'data_centering' )

        # Had trouble with this function getting called twice, for the VtkIso
        # that's owned by VtkIsoEb.
        if self.called_init_for_first_hdf5 == 0:
            self.called_init_for_first_hdf5 = 1
        else:
            return

        # It's important to initialize our two _LocalVtkData instances in this
        # order.  That's so that when we change levels, crosscolor_vtk_data's
        # are set before local_vtk_data's.  Otherwise, when vtkUpdate() is
        # called on local_vtk_data (it's never called on crosscolor_vtk_data!)
        # crosscolor_vtk_data calls setPiece() on a piece (i.e. box) it doesn't
        # know about yet.
        self.crosscolor_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=1,
            follow_global_curcomponent=0,
            follow_global_padded_boxes=1,
            always_use_real_boxes=0,
            instance_identifier = 'crosscolor',
            never_do_vtk_update = 0 )
        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=1,
            follow_global_curcomponent=1,
            follow_global_padded_boxes=1,
            always_use_real_boxes=0,
            instance_identifier='iso' )

        data_centering = self.local_vtk_data.getDataCentering(
            permute_in_reslice_mode=0)
        self.local_vtk_data.getReader().SetOriginShift(
            0.5*data_centering[0], 0.5*data_centering[1], 0.5*data_centering[2],
            -.5*data_centering[0], -.5*data_centering[1], -.5*data_centering[2])
        self.crosscolor_vtk_data.getReader().SetOriginShift(
            0.5*data_centering[0], 0.5*data_centering[1], 0.5*data_centering[2],
            -.5*data_centering[0], -.5*data_centering[1], -.5*data_centering[2])

        self.iso_block_prop = self.vtk_vtk.newProperty()
        self.iso_block_prop.SetColor( 0.0, 0.0, 0.0 )
        self.iso_block_prop.SetDiffuse( 1.0 )
        self.iso_block_prop.SetAmbient( 0.0 )

        self.iso_prop = self.vtk_vtk.newProperty()
        self.iso_prop.SetColor( 1.0, 1.0, 1.0 )
        if self.local_vtk_data.is2DMode():
            self.setDiffuse( 1.0 )
            self.setAmbient( 0.0 )
        else:
            self.setDiffuse( 0.85 )
            self.setAmbient( 0.15 )

        self.iso_func = vtkpython.vtkMarchingContourFilter()
        self.iso_accum = libVTKChomboPython.vtkChomboAccumulatePolyData()
    
        if self.local_vtk_data.is2DMode():
            self.iso_trans = vtkpython.vtkTransform()
            self.iso_trans.Identity()
    
            self.iso_trans_p = vtkpython.vtkTransformPolyDataFilter()
            self.iso_trans_p.SetTransform( self.iso_trans )
            self.iso_trans_p.SetInput( self.iso_func.GetOutput() )
    
            self.iso_accum.SetInput( self.iso_trans_p.GetOutput() )
    
            self.iso_block_plane = libVTKChomboPython.vtkChomboPlaneSource()
            self.iso_block_plane.SetResolution( 1, 1 )
            self.iso_block_plane.SetOrigin( 0.0, 0.0, 0.0 )
            self.iso_block_plane.SetPoint1( 1.0, 0.0, 0.0 )
            self.iso_block_plane.SetPoint2( 0.0, 1.0, 0.0 )
    
            self.iso_block_trans = vtkpython.vtkTransform()
            self.iso_block_trans.Identity()
    
            self.iso_block_trans_p = vtkpython.vtkTransformPolyDataFilter()
            self.iso_block_trans_p.SetTransform( self.iso_block_trans )
            self.iso_block_trans_p.SetInput( self.iso_block_plane.GetOutput() )
    
            self.iso_block_accum =\
                libVTKChomboPython.vtkChomboAccumulatePolyData()
            self.iso_block_accum.SetInput( self.iso_block_trans_p.GetOutput() )
    
            self.iso_block_map = self.vtk_vtk.newMapper()
            self.iso_block_map.ScalarVisibilityOff()
            self.iso_block_map.SetInput( self.iso_block_accum.GetOutput() )
            self.iso_block_map.ImmediateModeRenderingOn()
    
            self.iso_block_actor = self.vtk_vtk.newActor()
            self.iso_block_actor.SetProperty( self.iso_block_prop )
            self.iso_block_actor.SetMapper( self.iso_block_map )
    
        else:
            self.depthsorter = vtkpython.vtkDepthSortPolyData()
            self.depthsorter.SetDirectionToBackToFront()
            self.depthsorter.SetVector(1,1,1)
            self.depthsorter.SetCamera( self.vtk_cameras.getCamera() )
            self.depthsorter.SortScalarsOn()            
            self.iso_accum.SetInput(
                self.iso_func.GetOutput() )

        self.iso_map = self.vtk_vtk.newMapper()
        self.iso_map.SetInput( self.iso_accum.GetOutput() )
        self.iso_map.SetColorModeToMapScalars()
        self.iso_map.ScalarVisibilityOn()
        self.iso_map.SetLookupTable( self.vtk_cmap.getActiveColormap() )
        self.iso_map.ImmediateModeRenderingOn()

        self.iso_actor = vtkpython.vtkLODActor()
        self.iso_actor.SetNumberOfCloudPoints(10000)
        self.iso_actor.SetProperty( self.iso_prop )
        self.iso_actor.SetMapper( self.iso_map )

        self.iso_func.SetInput( self.local_vtk_data.getReader().GetOutput() )

        if self.local_vtk_data.is2DMode():
            # Black contours -- so they can be seen.
            self.cross_colorer = g_constant_color
            self.setTheConstantColor( (0,0,0) )
        else:
            self.cross_colorer = g_self_color
            self.setTheConstantColor( (1,1,1) )

        self.glyphs = vtk_glyphs.VtkGlyphs(
            dep_dict={'vtk_vtk':self.vtk_vtk,
                      'vtk_data':self.vtk_data,
                      'vtk_cmap':self.vtk_cmap,
                      'saved_states':self.saved_states},
            multi_instance_qualifier=self.multi_instance_qualifier + '_glyphs',
            caller_updater = lambda self=self, source=str(self.__class__) :
                self.local_vtk_data.vtkUpdate(source=source) )

        decls.memberFunctionAudit(self)


    def _refresh( self ):
        """
        Everything that was in the constructor, that I think needs to be
        recomputed upon a restore.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'self', 'component0' )

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.local_vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        apply( self.iso_map.SetScalarRange,
               self.vtk_cmap.getDefactoRange() )

        #
        # Fire off callbacks associated with the restored 'save':1 variables.
        #
        if not self.use_ghost_cells:  # Hasn't been initialized.
            self.use_ghost_cells = self.local_vtk_data.getAlwaysUsePaddedBoxes()
        self.setUseGhostCells( self.use_ghost_cells )
        self.setDoShowIsosurfaces( self.do_show_isosurfaces )
        self.changeColor( self.cross_colorer, self.the_constant_color )
        if self.local_vtk_data.getCurComponent():
            self.changeComponent()
        self.setIsClipped( self.is_clipped )
        if not self.local_vtk_data.is2DMode():
            self.setOpacity( self.opacity )
        self.setLineWidth( self.line_width )
        self.update2DColor( self.vtk_cmap.getBackgroundColor(), None )


        # Register callbacks with Notifiers.
        self.registerCallback( self.vtk_cmap.getNotifierActiveColormap(),
                               self.updateColormap )

        self.registerCallback( self.local_vtk_data.getNotifierAccumStart(),
                               self.updateAccumStart )
        self.registerCallback( self.local_vtk_data.getNotifierAccumMiddle(),
                               self.updateAccumMiddle )
        self.registerCallback( self.local_vtk_data.getNotifierAccumEnd(),
                               self.updateAccumEnd )

        self.registerCallback( self.local_vtk_data.getNotifierCurComponent(),
                               self.changeComponent )

        self.registerCallback( self.vtk_cmap.getNotifierBackgroundColor(),
                               self.update2DColor )
        self.registerCallback( self.vtk_data.getNotifierZSliceIsVisible(),
                               self.twoDSliceVisibilityHack)
        # That's right, vtk_data not local_vtk_data, because z_slice_is_visible
        # gets set by VtkSlice.

        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )

        self.iso_func.ComputeScalarsOn()
        self.iso_map.SetLookupTable( self.vtk_cmap.getActiveColormap() )

        map( lambda notifier, self=self :
            self.registerCallback( notifier,
                                   lambda d1,d2,self=self:
                                       self.changeComponent() ),
            self.local_vtk_data.getCmappedRangeMinNotifiers()
           +self.local_vtk_data.getCmappedRangeMaxNotifiers() )

        self.local_vtk_data.vtkUpdate( source = str(self.__class__) )


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()

    #
    # Accessors.
    # Setters are for use by the ChomboVis API.
    #
    def getMinIso(self):
        anag_utils.funcTrace()
        return self.min_iso[self.local_vtk_data.getCurComponent()]
    def getMaxIso(self):
        anag_utils.funcTrace()
        return self.max_iso[self.local_vtk_data.getCurComponent()]
    def setMinIso(self,x):
        anag_utils.funcTrace()
        self.min_iso[self.local_vtk_data.getCurComponent()] = x
    def setMaxIso(self,x):
        anag_utils.funcTrace()
        self.max_iso[self.local_vtk_data.getCurComponent()] = x


    def setShowGlyphs( self, on_off ):
        anag_utils.funcTrace()
        assert( on_off==0  or  on_off==1 )
        self.show_glyphs = on_off
        if on_off == 1:
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
        else:
            self.glyphs.hide()


    #
    # updateAccum* functions -- triggered by Notifier ________.
    #
    def updateAccumStart( self, dummy, extra_info ):
        anag_utils.funcTrace()
        if( (self.notFromHereOrVtkData( extra_info ))
        or  (self.do_show_isosurfaces == 0) ):
            return

        if self.local_vtk_data.is2DMode():
            self.iso_block_accum.StreamExecuteStart()

        self.iso_accum.StreamExecuteStart()

        if self.show_glyphs == 1:
            self.glyphs.updateAccumStart(
                min_level = self.local_vtk_data.getMinVisibleLevel(),
                max_level = self.local_vtk_data.getMaxVisibleLevel(),
                padded    = self.local_vtk_data.getAlwaysUsePaddedBoxes() )


    def updateAccumMiddle( self, box_num, extra_info ):
        """
        Arg box_num is what the Notifier that triggered this call was set
        to.

        Assumes ChomboReader.SetPiece( box_num ) has just been called.
        """
        anag_utils.funcTrace()
        if( (self.notFromHereOrVtkData( extra_info ))
        or  (self.do_show_isosurfaces == 0) ):
            return

        if self.local_vtk_data.is2DMode():
            multi = self.local_vtk_data.computeLevelOffset( box_num )
            offset = multi + self.local_vtk_data.getSliceOffset()

            self.iso_block_trans.Identity()

            extents = self.local_vtk_data.getVisibleBoxExtentsXYZ(box_num)
            xmin = extents[0]
            ymin = extents[1]
            zmin = extents[2]
            xmax = extents[3]
            ymax = extents[4]
            zmax = extents[5]

            self.iso_block_trans.Translate( xmin, ymin, 10*offset )
            self.iso_block_trans.Scale( xmax - xmin, ymax - ymin, 1.0 )

            self.iso_block_trans_p.Update()
            self.iso_block_accum.Append()

            self.iso_trans.Identity()
            self.iso_trans.Translate( 0.0, 0.0, offset )
            self.iso_trans_p.Update()

        else:
            self.iso_func.Update()

        if self._crossColorationOn():
            if self.local_vtk_data.isResliceMode():
                direction = self.local_vtk_data.getResliceDirection()
                self.crosscolor_vtk_data.setPiece( box_num,
                    direction,
                    self.local_vtk_data.getReslicePosition([direction,]))
            else:
                self.crosscolor_vtk_data.setPiece( box_num )
            if self.probe:
                self.probe.Update()
                self.cast.Update()
                if( (not self.local_vtk_data.is2DMode())
                and (self.probe.GetOutput().GetNumberOfCells() != 0) ):
                    self.normals.Update()
                if( self.local_vtk_data.is2DMode()
                and (self.local_vtk_data.getReader().GetNumPieces() > 1) ):
                    scalars = self.cast.GetOutput().GetPointData().GetScalars()
                    self.iso_trans_p.GetOutput().GetPointData().SetScalars( scalars )

        self.iso_accum.Append()

        if self.show_glyphs == 1:
            piece = self.local_vtk_data.getReader().GetCurrentPieceIndex()
            self.glyphs.updateAccumMiddle( piece, self.iso_func.GetOutput() )


    def updateAccumEnd( self, dummy, extra_info ):
        anag_utils.funcTrace()
        if( (self.notFromHereOrVtkData( extra_info ))
        or  (self.do_show_isosurfaces == 0) ):
            return

        if self.local_vtk_data.is2DMode():
            self.iso_block_accum.StreamExecuteEnd()

        self.iso_accum.StreamExecuteEnd()

        if self.show_glyphs == 1:
            self.glyphs.updateAccumEnd()


    def _crossColorationOn( self ):
        """
        Returns 1 if we're coloring the isosurfaces by anything other than
        constant color or self color.  Returns None otherwise.
        """
        anag_utils.funcTrace()

        if ( self.cross_colorer != g_constant_color
        and  self.cross_colorer != g_self_color
        and  self.cross_colorer != self.local_vtk_data.getCurComponent() ):
            return 1
        else:
            return None


    def setCurComponent( self, c ):
        anag_utils.funcTrace()
        if type(c) == type(2):
            c = self.local_vtk_data.getComponentList()[c]
        self.local_vtk_data.setCurComponent( c )
    

    def setOpacity( self, x ):
        """
        Set the opacity (between 0 and 1 inclusive) of the isosurfaces shown.
        Doesn't work with isocontours.
        """
        anag_utils.funcTrace()
        if self.local_vtk_data.is2DMode():
            anag_utils.error("Opacity works on isosurfaces, not contours.")
            return
        self.opacity = x
        self.iso_prop.SetOpacity( x )
        self.updateContours()
#       self.vtk_vtk.render()


    def setDiffuse( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.iso_prop:  # None if no file loaded
            self.iso_prop.SetDiffuse( x )
            self.vtk_vtk.render()
    def setAmbient( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.iso_prop:  # None if no file loaded
            self.iso_prop.SetAmbient( x )
            self.vtk_vtk.render()
    def setSpecular( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.iso_prop:  # None if no file loaded
            self.iso_prop.SetSpecular( x )
            self.vtk_vtk.render()


    def setLineWidth( self, w ):
        self.line_width = w
        if self.local_vtk_data.is2DMode():
            self.iso_prop.SetLineWidth(w)
            self.vtk_vtk.render()


    def _setCrossColoration( self, component_name ):
        """
        Color the isosurface with the indicated component.
        If component_name is the same as vtk_data.cur_component, that's a sign
        that we want to turn cross-coloration off.  We do so by routing the
        pipeline back around the probe, AND by setting crosscolor_vtk_data to
        the global current component.

        In 2D mode, doesn't work (except for g_constant_color and g_self_color).
        I don't know why.  See experi/2D_crosscoloration.py for a starting
        point.
        """
        anag_utils.funcTrace()

        # Arg component_name must name an actual component...
        assert( component_name )
        assert( component_name != g_constant_color )

        if component_name == self.local_vtk_data.getCurComponent():
            self.cross_colorer = g_self_color
        else:
            self.cross_colorer = component_name

        if component_name == self.local_vtk_data.getCurComponent():
            if self.local_vtk_data.is2DMode():
                self.iso_trans_p.SetInput( self.iso_func.GetOutput() )
            else:
                self.iso_accum.SetInput( self.iso_func.GetOutput() )
            self.iso_map.SetInput( self.iso_accum.GetOutput() )
            self.crosscolor_vtk_data.setCurComponent( component_name )

        else:
            if self._crossColorationOn() and (self.probe == None):
                self.probe = vtkpython.vtkProbeFilter()
                self.cast = vtkpython.vtkCastToConcrete()

                if not self.local_vtk_data.is2DMode():
                    self.normals = vtkpython.vtkPolyDataNormals()
                    #self.normals.SetMaxRecursionDepth(100)
                    #This was necessary with vtk3.2, but not even part of the
                    #API in vtk4.2.
                    self.normals.SetFeatureAngle(45)

            self.crosscolor_vtk_data.setCurComponent(component_name)

            self.probe.SetInput( self.iso_func.GetOutput() )
            self.probe.SetSource(
                self.crosscolor_vtk_data.getReader().GetOutput() )
            self.cast.SetInput( self.probe.GetOutput() )
            if self.local_vtk_data.is2DMode():
                self.iso_trans_p.SetInput( self.cast.GetOutput() )
            else:
                self.normals.SetInput( self.cast.GetOutput() )
                self.iso_accum.SetInput( self.normals.GetOutput() )

            self.iso_map.SetInput( self.iso_accum.GetOutput() )

        self.setIsClipped( self.is_clipped )
        self.local_vtk_data.vtkUpdate( source = str(self.__class__) )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.setDoShowIsosurfaces( 0 )
        self.vtk_vtk.removeActor( self.iso_block_actor, force=1 )

        if self.local_vtk_data.is2DMode():
            self.iso_trans_p.SetTransform( None )
            self.iso_trans_p.SetInput( None )
            self.iso_block_map.SetInput( None )
            self.iso_block_actor.SetProperty( None )
            self.iso_block_actor.SetMapper( None )
        self.iso_accum.SetInput( None )
        self.iso_map.SetInput( None )
        self.iso_actor.SetProperty( None )
        self.iso_actor.SetMapper( None )
        self.iso_func.SetInput( None )
        if self.probe: 
            self.probe.SetInput( None )
            self.normals.SetInput( None )


    def _paddedBoxesCallback( self, on_off, dummy ):
        """ Callback for local_vtk_data.always_use_padded_boxes. """
        anag_utils.funcTrace()
        self.setUseGhostCells( on_off )


    def setIsClipped( self, on_off ):
        """ Toggle clipping """
        anag_utils.funcTrace()
        self.is_clipped = on_off
        if on_off == 1:
            if self.clip_mode == g_clip_modes.eb:
                self.iso_map.ScalarVisibilityOff()
                self.vtk_clip.clipPipeline( self.iso_accum, self.iso_map,
                    clip_against_isocontour=True,
                    isocontour_clip_value =
                        self.vtk_data.getIsocontourClipValue() )
            else:
                self.vtk_clip.clipPipeline( self.iso_accum, self.iso_map )
        else:
            if self.clip_mode == g_clip_modes.eb:
                self.vtk_clip.unclipPipeline( self.iso_accum, self.iso_map,
                                              clip_against_isocontour=True )
                self.iso_map.ScalarVisibilityOn()
            else:
                self.vtk_clip.unclipPipeline( self.iso_accum, self.iso_map )


    def changeComponent( self, new_component=None, dummy2=None ):
        """
        Triggered by vtk_data.cur_component Notifier.
        """
        anag_utils.funcTrace()

        if not new_component:
            new_component = self.local_vtk_data.getCurComponent()
        if not new_component: # Case of no component loaded at all
            return                                      # Early return

        # Set min and max rendered isocontours.
        if not new_component in self.min_iso.keys():
            if self.local_vtk_data.is2DMode():
                self.setMinIso( self.local_vtk_data.getCmappedRangeMin() )
                self.setMaxIso( self.local_vtk_data.getCmappedRangeMax() )
                if self.num_isos == None:
                    self.setNumIsos( 10 )
            else:
                midpoint = (self.local_vtk_data.getCmappedRangeMin() +
                            self.local_vtk_data.getCmappedRangeMax()) / 2.0
                self.setMinIso( midpoint )
                self.setMaxIso( midpoint )
                if self.num_isos == None:
                    self.setNumIsos( 1 )

        # Cross-coloring.
        if self.cross_colorer == g_self_color:
            self.crosscolor_vtk_data.setCurComponent( new_component )
            self.crosscolor_vtk_data.getReader().Update()
            self._setCrossColoration( new_component )

        self.iso_func.SetValue( 0, self.getMinIso() )

        self.updateContours()

    
    def updateColormap( self, new_colormap, extra_info ):
        """
        Callback registered with vtk_cmap.active_colormap.
        """
        anag_utils.funcTrace()
        self.iso_map.SetLookupTable( new_colormap )


    def update2DColor( self, rgb, extra_info ):
        """
        Callback registered with vtk_cmap.background_color.
        """
        anag_utils.funcTrace()
        if self.local_vtk_data.is2DMode():
            self.iso_block_prop.SetColor( rgb[0], rgb[1], rgb[2] )


    def updateContours( self ):
        """
        Like self.changeContours() except uses existing values.
        """
        anag_utils.funcTrace()
        if not self.local_vtk_data.is2DMode():
            if self.opacity < 1.0:
                self.depthsorter.SetInput( self.iso_accum.GetOutput() )
                self.iso_map.SetInput( self.depthsorter.GetOutput() )
            else:
                if self.depthsorter.GetInput():
                    self.iso_map.SetInput( self.iso_accum.GetOutput() )

        comp = self.local_vtk_data.getCurComponent()
        if( (self.min_iso and self.max_iso and self.num_isos)
        and (comp in self.min_iso.keys())
        and (comp in self.max_iso.keys()) ):
            self.changeContours( self.getNumIsos(),
                                 self.getMinIso(), self.getMaxIso(),
                                 force_update = 1 )
        if self.is_clipped == 1:
            self.setIsClipped(1)

    
    def changeContours( self, num_isos, min_iso, max_iso, force_update=0 ):
        """ Make Vtk rerender the displayed isosurfaces.

            Arg num_isos is the number of isosurfaces.
            Arg min_iso is, of multiple isosurfaces, the one corresponding to
                the lowest value of self.local_vtk_data.getCurComponent().
            Arg max_iso is for the highest value of getCurComponent().
        """
        anag_utils.funcTrace()

        if((      num_isos != self.getNumIsos()
               or (not algorithms.floatEquals( min_iso, self.getMinIso(),
                                               abs(max_iso-min_iso)/10E8 ))
               or (not algorithms.floatEquals( max_iso, self.getMaxIso(),
                                               abs(max_iso-min_iso)/10E8 ))
           )
          or
             force_update == 1
          ):

            apply( self.iso_map.SetScalarRange,
                   self.vtk_cmap.getDefactoRange() )

            self.num_isos = num_isos
            self.setMinIso( min_iso )
            self.setMaxIso( max_iso )

            if self.do_show_isosurfaces == 0:
                return

            self.iso_func.GenerateValues( num_isos,
                                          self.getMinIso(),
                                          self.getMaxIso() )
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
        
    
    def changeColor( self, colorer, rgb=None ):
        """
        Render the isosurfaces in a single color, or color-coded according to
        the values of one of the components.
        
        Arg colorer can be the name of a component, or g_constant_color, or
        g_self_color.

        Arg rgb (a tuple) indicates the constant color to apply to all values
        on the isosurfaces.

        If colorer != g_constant_color but rgb!=None, that means save rgb for
        the next time colorer == g_constant_color.
        If colorer == g_constant_color but rgb==None, then use 
        self.the_constant_color (which happens to be the last rgb that was
        passed in here).

        In 2D mode, cross-coloration doesn't work.
        I don't know why.  See experi/2D_crosscoloration.py for a starting
        point.
        """
            
        anag_utils.funcTrace()

        if rgb != None:
            self.the_constant_color = rgb[:]
    
        if colorer == g_constant_color:
            self.iso_map.ScalarVisibilityOff()
            self.iso_prop.SetColor( self.the_constant_color[0],
                                    self.the_constant_color[1],
                                    self.the_constant_color[2] )
            self.cross_colorer = g_constant_color
        else:
            if self.is_clipped:
                self.iso_prop.SetColor( self.the_constant_color[0],
                                        self.the_constant_color[1],
                                        self.the_constant_color[2] )
            else:
                self.iso_map.ScalarVisibilityOn()
            if colorer:
                self.cross_colorer = colorer
            if colorer == g_self_color:
                self._setCrossColoration( self.local_vtk_data.getCurComponent())
            else:
               self._setCrossColoration( colorer )
    
        self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
    

    def setUseGhostCells( self, on_off ):
        """
        Without ghost cells, there are gaps at the box boundaries.
        """
        anag_utils.funcTrace()
        if self.use_ghost_cells != on_off:
            self.use_ghost_cells = on_off
            self.local_vtk_data.setAlwaysUsePaddedBoxes( on_off )
            self.crosscolor_vtk_data.setAlwaysUsePaddedBoxes( on_off )
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
    
    
    def setDoShowIsosurfaces( self, on_off ):
        """ 
        Make the isosurfaces visible or invisible.
        This is the callback from the visible_checkbutton in control_iso.py,
        and arg on_off is the state of the checkbutton (0=off, 1=on).
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "on_off" )

        self.do_show_isosurfaces = on_off
        if on_off == 0:
            self.vtk_vtk.removeActor( self.iso_actor )
            self.vtk_vtk.render()
            self.crosscolor_vtk_data.setFollowGlobalLevels(0)
        else:
            self.updateContours()
            self.crosscolor_vtk_data.setFollowGlobalLevels(1)
            self.vtk_vtk.addActor( self.iso_actor )
            self.vtk_vtk.render()

        decls.memberFunctionAudit(self)


    def twoDSliceVisibilityHack( self, on_off, extra_info ):
        """
        Callback for Notifier vtk_data.z_slice_is_visible.
        Sees to turning on the blocking actors, whenever the slice is invisible.

        It's not really all that hackish.  If there's anything hackish about
        this, it's that there would have been a more direct way to turn the
        blockers on and off, but that would have required vtk_iso to depend on
        vtk_slice.
        """
        anag_utils.funcTrace()
        if self.local_vtk_data.is2DMode():
            if on_off == 0:
                self.vtk_vtk.addActor( self.iso_block_actor )
            else:
                self.vtk_vtk.removeActor( self.iso_block_actor )


    def printScalars( self, object ):
        """
        For debugging: prints the scalar values in the isocontours/isosurfaces.
        """
        anag_utils.funcTrace()
        if not object.GetOutput():
            return
        scalars = object.GetOutput().GetPointData().GetScalars()
        if not scalars:
            return
        for i in range(0, scalars.GetNumberOfTuples()):
            print algorithms.prettyRound( scalars.GetTuple1(i), 3 )


    def unitTest(self):
        if self.saved_states.getNumHDF5sLoaded() > 0:
            self.setDoShowIsosurfaces( 1 )
            self.changeColor( g_constant_color, rgb=(1.0,0.4,0.0) )
            iso_val = self.local_vtk_data.getCmappedRangeMin() * 0.1 +\
                      self.local_vtk_data.getCmappedRangeMax() * 0.9
            self.changeContours( 3, iso_val, iso_val )
