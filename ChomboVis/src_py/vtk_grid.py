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

# File: vtk_grid.py
# Author: TDSternberg
# Created: 6/05/01

import libVTKChomboPython
import vtkpython

import anag_utils
import self_control
import sys
import math

g_decls = anag_utils.Declarations( 'g_decls', 'g_button_tags', 'g_detail_modes')


class VtkGrid( self_control.SelfControl ):
    def __init__( self, dep_dict ):
        """ Initialize module-global and global-global data. """
        anag_utils.funcTrace()

        instance_data = [
            { 'name':'cell_detail_button_tag', 'get':1, 'save':1},
            { 'name':'color', 'initval':(1,1,1), 'get':2,
                'save':1},
            { 'name':'show_domain_box', 'get':1, 'initval':1, 'save':1},
            { 'name':'show_solid_boxes', 'get':1, 'set':2, 'initval':0,
                'save':4},
            { 'name':'solid_box_shrinkage_factor', 'get':1, 'set':2, 'save':1,
                'initval':-2.75},
            { 'name':'show_slice_cells', 'get':1, 'initval':0, 'save':1,
                'notify':1},
            { 'name':'show_tick_marks', 'get':1, 'initval':0, 'save':1},
            { 'name':'domain_box'},
            { 'name':'boxes'},
            { 'name':'outline_prop'},
            { 'name':'solid_prop'},
            { 'name':'tickmarks_prop'},
            { 'name':'is_clipped', 'get':1, 'set':2, 'initval':0, 'save':5},
            { 'name':'color_outlines_by_level', 'initval':0, 'get':1, 'set':2,
                'save':5},
            { 'name':'color_solids_by_level', 'initval':0, 'get':1, 'set':2,
                'save':5},
            { 'name':'prescribed_outline_level_colors', 'initval':None, 'set':2,
                'save':5},
            { 'name':'prescribed_solid_level_colors', 'initval':None, 'set':2,
                'save':5},
            { 'name':'solid_color', 'get':1, 'set':2, 'initval':(1,1,1),
                'save':1},
            { 'name':'opacity', 'get':1, 'set':2, 'save':5, 'initval':1.0},
            { 'name':'local_vtk_data', 'get':1 },
            { 'name':'use_ghost_cells', 'get':1, 'set':2, 'save':1 },
            { 'name':'vis_disabled', 'initval':True}
        ]
        self_control.SelfControl.__init__( self, dep_dict, instance_data )

        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'level', 'instance_data',
            'dep_dict' )

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        # Re-enable at the end of this function.  Otherwise we do a lot of
        # redundant work in updateVis().
        self.vis_disabled=True

        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=1,
            follow_global_curcomponent=1,
            always_use_real_boxes = 1,
            instance_identifier='grids' )
        data_centering =  self.local_vtk_data.getDataCentering(
            permute_in_reslice_mode=0)
        self.local_vtk_data.getReader().SetOriginShift(
            0.5*data_centering[0], 0.5*data_centering[1], 0.5*data_centering[2],
            0.0, 0.0, 0.0 )

        self.cell_detail_button_tag = g_button_tags.Nothing

        self.outline_prop = self.vtk_vtk.newProperty()
        apply( self.outline_prop.SetColor, self.getOutlineColor() )
        self.outline_prop.SetDiffuse( 0.8 )
        self.outline_prop.SetAmbient( 0.2 )
        self.solid_prop = self.vtk_vtk.newProperty()
        apply( self.solid_prop.SetColor, self.getSolidColor() )
        self.tickmarks_prop = vtkpython.vtkProperty2D()
        apply( self.tickmarks_prop.SetColor, self.getOutlineColor() )

        self.domain_box = _DomainBox( dep_dict = 
            { 'outline_prop':self.outline_prop,
              'tickmarks_prop':self.tickmarks_prop,
              'saved_states':self.saved_states,
              'vtk_vtk':self.vtk_vtk,
              'vtk_data':self.vtk_data,
              'local_vtk_data':self.local_vtk_data } )

        self.boxes = _Boxes( dep_dict =
            { 'outline_prop':self.outline_prop,
              'solid_prop':self.solid_prop,
              'saved_states':self.saved_states,
              'vtk_vtk':self.vtk_vtk,
              'vtk_cmap':self.vtk_cmap,
              'local_vtk_data':self.local_vtk_data } )
        self.boxes._initForFirstHDF5()
        self.boxes._refresh()
        self.domain_box._initForFirstHDF5()
        self.domain_box._refresh()
        self.boxes.already_called_initforfirsthdf5 = 1
        self.domain_box.already_called_initforfirsthdf5 = 1
        self.vis_disabled=False


    def _refresh( self ):
        """
        Overrides SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()

        self.vis_disabled=True
        self.boxes.setVisDisabled( True )

        if not self.use_ghost_cells:  # Hasn't been initialized.
            self.use_ghost_cells = self.local_vtk_data.getAlwaysUsePaddedBoxes()
        self.setUseGhostCells( self.use_ghost_cells )
        self.registerCallback(
            self.local_vtk_data.getNotifierAlwaysUsePaddedBoxes(),
            self._paddedBoxesCallback )

        # We call _refresh() on the subclasses so that their Notifier
        # callbacks will be (re)registered when, immediately below, we restore
        # the state of the cell domain box and the cell detail.  Otherwise,
        # their accumulations won't be triggered.
        self.boxes._refresh()
        self.domain_box._refresh()

        self.showDomainBox( self.show_domain_box )
        self.showTickMarks( self.show_tick_marks )
        self.showSliceCells( self.getShowSliceCells() )
        self.setSolidBoxShrinkageFactor( self.solid_box_shrinkage_factor )
        self.setShowSolidBoxes( self.show_solid_boxes )

        if( ( self.cell_detail_button_tag == g_button_tags.Face_cells)
        and ( self.local_vtk_data.is2DMode() ) ): # Can happen on reslice.
            self.cell_detail_button_tag = g_button_tags.All_cells

        # Hack for getting around accum init problem:
        saved_detail_tag = self.cell_detail_button_tag
        self.chooseCellDetail( g_button_tags.Bounding_boxes )
        self.chooseCellDetail( saved_detail_tag )
        saved_color_by_level = self.color_outlines_by_level 
        self.setColorOutlinesByLevel(1)
        self.setColorOutlinesByLevel( saved_color_by_level )
        saved_color_by_level = self.color_solids_by_level 
        self.setColorSolidsByLevel(1)
        self.setColorSolidsByLevel( saved_color_by_level )

        self.setPrescribedOutlineLevelColors(
            self.prescribed_outline_level_colors )
        self.setPrescribedSolidLevelColors(
            self.prescribed_solid_level_colors )
        self.setOutlineColor( self.getOutlineColor() )
        self.setSolidColor( self.getSolidColor() )
        self.setIsClipped( self.is_clipped )
        self.setOpacity( self.opacity )
        self.setLineWidth( self.vtk_data.getGridLineWidth() )

        self.vis_disabled=False
        self.boxes.setVisDisabled( False )
        self.boxes.updateVis()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.domain_box.cleanup()
        self.boxes.cleanup()


    def _paddedBoxesCallback( self, on_off, dummy ):
        """ Callback for local_vtk_data.always_use_padded_boxes. """
        anag_utils.funcTrace()
        self.setUseGhostCells( on_off )


    def setUseGhostCells( self, on_off ):
        anag_utils.funcTrace()
        if self.use_ghost_cells != on_off:
            self.use_ghost_cells = on_off
            self.local_vtk_data.setAlwaysUsePaddedBoxes( on_off )
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
            if not self.vis_disabled:
                self.boxes.updateVis()

    
    def setIsClipped( self, on_off ):
        """ Toggle clipping """
        anag_utils.funcTrace()
        self.is_clipped = on_off
        if on_off == 1:
            self.vtk_clip.clipPipeline( self.boxes.getOutlineAccum(),
                                        self.boxes.getOutlineMapper() )
            self.vtk_clip.clipPipeline( self.boxes.getSolidAccum(),
                                        self.boxes.getSolidMapper() )
        else:
            self.vtk_clip.unclipPipeline( self.boxes.getOutlineAccum(),
                                          self.boxes.getOutlineMapper() )
            self.vtk_clip.unclipPipeline( self.boxes.getSolidAccum(),
                                          self.boxes.getSolidMapper() )


    def setOpacity( self, x ):
        """
        Set opacity of solid boxes.
        """
        anag_utils.funcTrace()
        assert( 0.0 <= x <= 1.0 )
        self.opacity = x
        self.boxes.setOpacity( x )
        if not self.vis_disabled:
            self.boxes.updateVis()


    def setColorOutlinesByLevel( self, yes_no ):
        """
        If arg==0, color all the level-specific grids by the single color.
        If arg==1, color them differently, by level.
        See also setPrescribedOutlineLevelColors().
        """
        anag_utils.funcTrace()
        assert( yes_no==0  or  yes_no==1 )
        self.color_outlines_by_level = yes_no
        self.boxes.getOutlineMapper().SetScalarVisibility(yes_no)
        if not self.vis_disabled:
            self.boxes.updateVis()

    def setColorSolidsByLevel( self, yes_no ):
        """
        See setColorOutlinesByLevel().
        """
        anag_utils.funcTrace()
        assert( yes_no==0  or  yes_no==1 )
        self.color_solids_by_level = yes_no
        self.boxes.getSolidMapper().SetScalarVisibility(yes_no)
        if not self.vis_disabled:
            self.boxes.updateVis()


    def setPrescribedOutlineLevelColors( self, colors ):
        """
        If arg colors==None, the colors are evenly spaced
        across the default colormap.  Otherwise, the color for level i is
        set to colors[i] (so if you set colors, you'll
        need to make it a tuple with at least as many elements as there are
        levels in your dataset).
        Elements of colors should be rgb (3-tuples) or rgba (4-tuples), with
        each of r, g, b, and a in the range [0.0,1.0].
        """
        anag_utils.funcTrace()
        self.prescribed_outline_level_colors = colors
        self.boxes.setPrescribedLevelColors( colors, b_solid=0 )


    def setPrescribedSolidLevelColors( self, colors ):
        """
        See setPrescribedOutlineLevelColors().
        """
        anag_utils.funcTrace()
        self.prescribed_solid_level_colors = colors
        self.boxes.setPrescribedLevelColors( colors, b_solid=1 )


    def setDiffuse( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.solid_prop:  # None if no file loaded
            self.solid_prop.SetDiffuse( x )
            self.vtk_vtk.render()
    def setAmbient( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.solid_prop:  # None if no file loaded
            self.solid_prop.SetAmbient( x )
            self.vtk_vtk.render()
    def setSpecular( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.solid_prop:  # None if no file loaded
            self.solid_prop.SetSpecular( x )
            self.vtk_vtk.render()


    def setLineWidth( self, w ):
        """ Set grid lines to w pixels wide. """
        anag_utils.funcTrace() 
        self.vtk_data.setGridLineWidth(w)  # vtk_slice registers -- slice cells
            # Don't call setGridLineWidth() on local_vtk_data, as vtk_slice
            # doesn't look at that.
        if self.outline_prop:  # None if no file loaded
            self.outline_prop.SetLineWidth(w)
            self.tickmarks_prop.SetLineWidth(w)
        self.vtk_vtk.render()


    def getOutlineColor(self):
        rgb = self.vtk_data.getGridColor()[:]
        rounded = []
        for c in rgb:
            rounded.append( round(c*1000)/1000.0 )
        return rounded


    def setOutlineColor( self, rgb ):
        anag_utils.funcTrace()

        self.vtk_data.setGridColor(rgb[:])
        self.vtk_data.setGridColorNotifier(1)
        apply( self.outline_prop.SetColor, rgb )
        apply( self.tickmarks_prop.SetColor, rgb )
        self.vtk_vtk.render()

    def setSolidColor( self, rgb ):
        anag_utils.funcTrace()
        apply( self.solid_prop.SetColor, rgb )
        self.solid_color = rgb
        self.vtk_vtk.render()


    def showDomainBox( self, on_off ):
        """
        Arg on_off==1 => do show the domain box.  Otherwise don't.
        """
        anag_utils.funcTrace()
        self.show_domain_box = on_off
        if on_off == 0:
            self.domain_box.hide()
        else:
            self.domain_box.show()


    def showTickMarks( self, on_off ):
        """
        Arg on_off==1 => do show the tick marks.  Otherwise don't.
        """
        anag_utils.funcTrace()
        self.show_tick_marks = on_off
        self.domain_box.toggleTickmarks( on_off )

    
    def showSliceCells( self, on_off ):
        """
        Arg on_off==1 => do show slice cells.  Otherwise don't.
        """
        anag_utils.funcTrace()
        if on_off != self.getShowSliceCells():
            self.vtk_data.setSliceGridVisibility(on_off)
            self.setShowSliceCells( on_off ) # notifier


    def setShowSolidBoxes( self, on_off ):
        """
        Arg on_off==1 => do show solid boxes.  Otherwise don't.
        """
        anag_utils.funcTrace()
        assert( on_off==0  or  on_off==1 )
        self.show_solid_boxes = on_off
        self.boxes.toggleSolidBoxes( on_off )


    def setSolidBoxShrinkageFactor( self, factor ):
        anag_utils.funcTrace()
        self.solid_box_shrinkage_factor = factor
        self.boxes.setSolidBoxShrinkageFactor( factor )
        if not self.vis_disabled:
            self.boxes.updateVis()


    def chooseCellDetail( self, button_tag ):
        """
        Arg button_tag is the tag of the invoked radiobutton.
        This is the callback for control_grid's radiobuttons.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls",
            "button_tag" )
    
        self.cell_detail_button_tag = button_tag

        if button_tag == g_button_tags.Nothing:
            self.boxes.showNothing()
        else:
            if   button_tag == g_button_tags.Bounding_boxes:
                self.boxes.showBoundingBoxes()
            elif button_tag == g_button_tags.Face_cells:
                self.boxes.showFaceCells()
            elif button_tag == g_button_tags.All_cells:
                self.boxes.showAllCells()

        decls.memberFunctionAudit(self)
    

    def getDomainBoxActor( self ): return self.domain_box.getActor()
    def getBoxesActor( self ): return self.boxes.getActor()


    def unitTest(self):
        if self.saved_states.getNumHDF5sLoaded() > 0:
            self.chooseCellDetail( "Face cells" )
            self.setColor( (1.0,0.9,0.9) )
            self.showDomainBox( 1 )


class _DomainBox( self_control.SelfControl ):
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        self_control.SelfControl.__init__( self, dep_dict, metadata=
          [
            {'name':'source'},
            {'name':'actor', 'get':1},
            {'name':'already_called_initforfirsthdf5'},
            {'name':'tickmarks'}
          ] )
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'dep_dict', 'mapper',
            'labels', 'permuted_labels' )

        self.source = libVTKChomboPython.vtkBBoxSource()

        mapper = self.vtk_vtk.newMapper()
        mapper.SetInput( self.source.GetOutput() )
        mapper.ImmediateModeRenderingOn()

        self.actor = self.vtk_vtk.newActor()
        self.actor.SetProperty( self.outline_prop )
        self.actor.SetMapper( mapper )

        # Tickmarks
        self.tickmarks = vtkpython.vtkCubeAxesActor2D()
        self.tickmarks.SetInput(  self.source.GetOutput() )
        self.tickmarks.SetCamera( self.vtk_vtk.getRenderer().GetActiveCamera() )
        self.tickmarks.SetLabelFormat("%6.4g")
        self.tickmarks.SetFlyModeToOuterEdges()
        self.tickmarks.SetFontFactor(0.8)
        self.tickmarks.SetProperty( self.tickmarks_prop )
        if self.local_vtk_data.is2DMode():
            self.tickmarks.SetYAxisVisibility(0)
            # VTK bug: Z and Y are switched
            if self.local_vtk_data.isResliceMode():
                labels = ('X','Y','Z')
                permuted_labels = self.local_vtk_data.permuteCoords(
                    labels, self.local_vtk_data.getResliceDirection() )
                self.tickmarks.SetXLabel( permuted_labels[0] )
                self.tickmarks.SetYLabel( permuted_labels[1] )
        decls.memberFunctionAudit(self)


    def toggleTickmarks( self, on_off ):
        """ Display, or hide, the tickmarks along the edges of the domain. """
        anag_utils.funcTrace()
        assert( on_off == 0  or  on_off == 1 )
        if on_off == 0:
            self.vtk_vtk.removeActor( self.tickmarks )
        else:
            self.vtk_vtk.addActor( self.tickmarks )
        self.vtk_vtk.render()


    def _initForFirstHDF5( self ):
        if self.already_called_initforfirsthdf5:
            return
        anag_utils.funcTrace()


    def _refresh( self, is_new_hdf5=None ):
        anag_utils.funcTrace()
        self.registerCallback(
            self.local_vtk_data.getNotifierResliceDirection(),
            self.setResliceBounds )
        self.registerCallback(
            self.local_vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicCallback )
        self._initBounds()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.hide()
        self.toggleTickmarks(0)
        self.unregisterCallbacks()
        self.zeroSelfVariables()
        # Needed to call unregisterCallbacks() and zeroSelfVariables() because
        # this class isn't on ChomboVis.init_data, so ChomboVis._cleanupClass()
        # won't call those for us.

    def _anisotropicCallback( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._initBounds()

    def setResliceBounds( self, dummy, extra_info ):
        anag_utils.funcTrace()
        self._initBounds()


    def show( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.addActor( self.actor )
        self.vtk_vtk.render()

    def hide( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.removeActor( self.actor )
        self.vtk_vtk.render()
    
    def _initBounds( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls',
            'xmin', 'xmax', 'ymin', 'ymax', 'zmin', 'zmax', 'direction',
            'extents', 'dx', 'data_centering'  )

        data_centering =\
            self.vtk_data.getDataCentering(permute_in_reslice_mode=None)
            # Don't permute cuz we're about to do it in a little bit.
        dx = self.vtk_data.getReader().GetLevelDx(0)            

        extents = [
            self.local_vtk_data.getDomainMin(['x']) + dx[0]*data_centering[0]/2,
            self.local_vtk_data.getDomainMin(['y']) + dx[1]*data_centering[1]/2,
            self.local_vtk_data.getDomainMin(['z']) + dx[2]*data_centering[2]/2,
            self.local_vtk_data.getDomainMax(['x']) - dx[0]*data_centering[0]/2,
            self.local_vtk_data.getDomainMax(['y']) - dx[1]*data_centering[1]/2,
            self.local_vtk_data.getDomainMax(['z']) - dx[2]*data_centering[2]/2
                  ]

        if self.local_vtk_data.isResliceMode():
            direction = self.local_vtk_data.getResliceDirection()
            extents = ( self.local_vtk_data.permuteCoords(
                            extents[0:3], direction )
                       + self.local_vtk_data.permuteCoords(
                            extents[3:6], direction ))

        xmin,ymin,zmin,xmax,ymax,zmax = extents
        self.source.SetBounds( xmin, xmax, ymin, ymax, zmin, zmax )

        decls.memberFunctionAudit(self)


class _Boxes( self_control.SelfControl ):
    """
    All boxes other than the domain box.  The domain box is special really
    because it's controlled mostly from Python, whereas the bounding- face-
    and all-cell boxes are taken care of in C++.
    """

    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        self_control.SelfControl.__init__( self, dep_dict, metadata=
          [
            { 'name':'outline_actor', 'get':1},
            { 'name':'solid_actor', 'get':1},
            { 'name':'outline_source', 'get':1},
            { 'name':'solid_source', 'get':1},
            { 'name':'outline_accum', 'get':1},
            { 'name':'solid_accum', 'get':1},
            { 'name':'outline_mapper', 'get':1},
            { 'name':'solid_mapper', 'get':1},
            { 'name':'solid_box_shrinkage_factor', 'set':1},
            { 'name':'outline_level_colormap' }, # user-prescribed
            { 'name':'solid_level_colormap' },   # user-prescribed
            { 'name':'default_level_colormap'},
            { 'name':'already_called_initforfirsthdf5'},
            { 'name':'prescribed_outline_level_colors', 'initval':None,'set':2},
            { 'name':'prescribed_solid_level_colors',   'initval':None,'set':2},
            { 'name':'depthsorter'},
            { 'name':'solid_boxes_visible'},
            { 'name':'vis_disabled', 'set':1}
          ] )
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'self', 'dep_dict', 'decls',
            'n_levels' )

        self.outline_source = libVTKChomboPython.vtkChomboOutlineSource()
        self.outline_accum = libVTKChomboPython.vtkChomboAccumulatePolyData()
        self.outline_mapper = self.vtk_vtk.newMapper()
        self.outline_actor = self.vtk_vtk.newActor()

        self.solid_source = vtkpython.vtkCubeSource()
        self.solid_accum = libVTKChomboPython.vtkChomboAccumulatePolyData()
        self.solid_mapper = self.vtk_vtk.newMapper()
        self.solid_actor = self.vtk_vtk.newActor()

        self.depthsorter = vtkpython.vtkDepthSortPolyData()
        self.depthsorter.SetDirectionToBackToFront()
        self.depthsorter.SetDepthSortModeToBoundsCenter() # still not great
        self.depthsorter.SetVector(1,1,1)
        self.depthsorter.SetCamera(self.vtk_vtk.getRenderer().GetActiveCamera())
        self.depthsorter.SortScalarsOn()

        #
        # Set up pipeline.
        #
        self.outline_source.SetDetailMode( g_detail_modes.face_cells )
        self.outline_accum.SetInput( self.outline_source.GetOutput() )
        self.outline_mapper.SetInput( self.outline_accum.GetOutput() )
        self.outline_mapper.ImmediateModeRenderingOn()
        self.outline_actor.SetProperty( self.outline_prop )
        self.outline_actor.SetMapper( self.outline_mapper )

        self.solid_accum.SetInput( self.solid_source.GetOutput() )
        self.depthsorter.SetInput( self.solid_accum.GetOutput() )
        self.solid_mapper.SetInput( self.depthsorter.GetOutput() )
        self.solid_mapper.ImmediateModeRenderingOn()
        self.solid_actor.SetProperty( self.solid_prop )
        self.solid_actor.SetMapper( self.solid_mapper )

        self.outline_level_colormap = libVTKChomboPython.vtkChomboLookupTable()
        self.solid_level_colormap = libVTKChomboPython.vtkChomboLookupTable()
        n_levels = self.local_vtk_data.getMaxAvailableLevel() + 1
        self.outline_level_colormap.SetNumberOfTableValues( n_levels )
        self.solid_level_colormap.SetNumberOfTableValues( n_levels )

        self.default_level_colormap =  libVTKChomboPython.vtkChomboLookupTable()
        self.default_level_colormap.SetHueRange(0.0,0.6667)
        self.default_level_colormap.SetSaturationRange(0.5,0.5)
        self.default_level_colormap.SetValueRange(1.0,1.0)
        self.default_level_colormap.SetAlphaRange(1.0,1.0)
        self.default_level_colormap.SetNumberOfColors(256)
        self.default_level_colormap.SetTableRange(0,255)
        self.default_level_colormap.Build()

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        if self.already_called_initforfirsthdf5:
            return
        anag_utils.funcTrace()
        self.outline_source.SetChomboReader( self.local_vtk_data.getReader() )


    def _refresh( self ):
        anag_utils.funcTrace()

        map( lambda notifier, self=self :
            self.registerCallback( notifier,
                                   lambda d1,d2,self=self:
                                       self.updateVis() ),
            (self.local_vtk_data.getNotifierMinVisibleLevel(),
             self.local_vtk_data.getNotifierMaxVisibleLevel(),
             self.local_vtk_data.getNotifierResliceNotifier()) )
        self.registerCallback(
            self.local_vtk_data.getNotifierAnisotropicFactorsNotifier(),
            lambda d1, d2, self=self : self.updateVis())

        map( lambda notifier, self=self :
            self.registerCallback( notifier,
                                   lambda d1,d2,self=self:
                                       self.updateVis()),
            self.local_vtk_data.getCmappedRangeMinNotifiers()
           +self.local_vtk_data.getCmappedRangeMaxNotifiers() )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.showNothing()
        self.unregisterCallbacks()
        self.zeroSelfVariables()


    def vtkUpdate( self ):
        """ Force an update. """
        anag_utils.funcTrace()
        self.local_vtk_data.vtkUpdate( source = str(self.__class__) )


    def updateVis( self ):
        anag_utils.funcTrace()
        if self.vis_disabled == True:
            return

        # Don't be tempted to move the depthsorter in and out of the pipeline
        # as-needed (the way you do it in vtk_particles).  That's because
        # if you want to do that, you have to coexist with the effects clipping
        # has on the pipeline.  I've tried that and it wasn't worth it.

        self.outline_accum.StreamExecuteStart()
        self.solid_accum.StreamExecuteStart()

        cmap_range = self.vtk_cmap.getDefactoRange()
        if cmap_range[0] != None:
            self.outline_mapper.SetScalarRange( cmap_range )

        min_lev = self.local_vtk_data.getMinVisibleLevel()
        max_lev = self.local_vtk_data.getMaxVisibleLevel()
        padded  = self.local_vtk_data.getAlwaysUsePaddedBoxes()
        real    = self.local_vtk_data.getAlwaysUseRealBoxes()
        for lev in range( min_lev, max_lev+1):
            num_boxes = self.local_vtk_data.getLevelNumBoxes( lev )
            for box in range(0, num_boxes ):
                if self.local_vtk_data.isResliceMode():
                    axis = self.local_vtk_data.getResliceDirection()
                    pos = self.local_vtk_data.getReslicePosition()[axis]
                    if not self.local_vtk_data.planeIntersectsBox(
                                        lev, box, axis, pos ):
                        continue

                self.outline_source.UpdateOutput( lev, box, padded, real )
                if self.solid_boxes_visible == 1:
                    self.reshapeSolidSource( lev, box )
                    self.solid_source.Update()

                if self.outline_mapper.GetScalarVisibility() == 1:
                    self.insertBoxColors( self.outline_source.GetOutput(),
                                          lev, cmap_range, b_solid=0 )
                if( (self.solid_mapper.GetScalarVisibility() == 1)
                and (self.solid_boxes_visible == 1) ):
                    self.insertBoxColors( self.solid_source.GetOutput(),
                                          lev, cmap_range, b_solid=1 )
                self.outline_accum.Append()
                self.solid_accum.Append()

        self.outline_accum.StreamExecuteEnd()
        self.solid_accum.StreamExecuteEnd()
        self.vtk_vtk.render()


    def reshapeSolidSource( self, level, box_num ):
        """
        Set the dimensions of the solid-box source object to those of the
        indicated AMR box.
        """
        anag_utils.funcTrace()
        extents = self.local_vtk_data.getBoxExtentsXYZ( level, box_num )
        pow_shrink = math.pow(10.0, self.solid_box_shrinkage_factor)
        x_shrink = pow_shrink * (extents[3]-extents[0] ) / 2.0
        y_shrink = pow_shrink * (extents[4]-extents[1] ) / 2.0
        z_shrink = pow_shrink * (extents[5]-extents[2] ) / 2.0
        self.solid_source.SetBounds( extents[0]+x_shrink, extents[3]-x_shrink,
                                     extents[1]+y_shrink, extents[4]-y_shrink,
                                     extents[2]+z_shrink, extents[5]-z_shrink )


    def insertBoxColors( self, polydata, level, cmap_range, b_solid ):
        """
        Try to set the box's color, by setting the scalars of its polydata.
        Arg b_solid is 1 or 0 as we want to deal with solid or outlined boxes.
        """
        anag_utils.funcTrace()
        assert( b_solid==0  or  b_solid==1 )

        if b_solid == 0:
            mapper = self.outline_mapper
            prescribed_level_colors = self.prescribed_outline_level_colors
            level_colormap = self.outline_level_colormap
        else:
            mapper = self.solid_mapper
            prescribed_level_colors = self.prescribed_solid_level_colors
            level_colormap = self.solid_level_colormap

        if cmap_range[0] == None: # For when no components have been loaded
            color_range = (0,1)
        else:
            color_range = cmap_range[:]
        mapper.SetScalarRange( color_range )

        scalars = vtkpython.vtkDoubleArray()
        scalars.SetNumberOfTuples(
            polydata.GetNumberOfPoints())

        n_levels = self.local_vtk_data.getMaxAvailableLevel() + 1
        frac = level/(n_levels+0.0)
        if n_levels > 1:
            frac *= n_levels/(n_levels-1.0)
        scalars.FillComponent(0, color_range[0] * (1-frac)
                               + color_range[1] * frac )
        if prescribed_level_colors == None:
            mapper.SetLookupTable( self.default_level_colormap )
        else:
            mapper.SetLookupTable( level_colormap )

        polydata.GetPointData().SetScalars( scalars )


    def setPrescribedLevelColors( self, colors, b_solid ):
        """
        Total control over which colors to paint each level's grids.
        Arg b_solid is 1 or 0 as we want to deal with solid or outlined boxes.
        Setter for self.prescribed_level_colors, which is a tuple of
          rgb[a] tuples.
        """
        anag_utils.funcTrace()
        assert( (colors==None)
        or      (len(colors) > self.local_vtk_data.getMaxAvailableLevel()) )
        assert( (colors==None)
        or
                ( 3 <= len(colors[0]) <= 4 ) ) # 4th is alpha -- optional
        assert( b_solid==0  or  b_solid==1 )
        prescribed_level_colors = colors
        if b_solid == 0:
            prescribed_level_colors = self.prescribed_outline_level_colors \
                                    = colors
            level_colormap = self.outline_level_colormap
        else:
            prescribed_level_colors = self.prescribed_solid_level_colors \
                                    = colors
            level_colormap = self.solid_level_colormap

        if colors == None:
            return
        n_levels = self.local_vtk_data.getMaxAvailableLevel() + 1
        if len(colors) < n_levels:
            anag_utils.fatal( "Not enough colors for by-level coloring "
                "of grids -- need at least as many as the number of levels "
                "in your dataset." )

        for ndx in range(0,n_levels):
            c = colors[ndx]
            assert( 0.0 <= min(c) <= max(c) <= 1.0 )
            if   len(c)==3:
                level_colormap.SetTableValue(ndx, c[0],c[1],c[2],1)
            elif len(c)==4:
                level_colormap.SetTableValue(ndx, c[0],c[1],c[2],c[3])
            else:
                anag_utils.error( "Bad format for prescribed by-level "
                "colors -- should be a tuple of 3-tuples (rgb) or 4-tuples "
                "(rgba)." )


    def setOpacity( self, x ):
        anag_utils.funcTrace()
        self.solid_prop.SetOpacity( x )


    #
    # Show varying degrees of detail.
    #
    def showNothing( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.removeActor( self.outline_actor )
        self.vtk_vtk.render()
        self.updateVis()
    def showBoundingBoxes( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.addActor( self.outline_actor )
        self.outline_source.SetDetailMode( g_detail_modes.box_edges )
        self.updateVis()
    def toggleSolidBoxes( self, on_off ):
        anag_utils.funcTrace()
        self.solid_boxes_visible = on_off
        if on_off == 0:
            self.vtk_vtk.removeActor( self.solid_actor )
        else:
            self.vtk_vtk.addActor( self.solid_actor )
        self.updateVis()
    # No need for a showSliceCells() function.
    def showFaceCells( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.addActor( self.outline_actor )
        self.outline_source.SetDetailMode( g_detail_modes.face_cells )
        self.updateVis()
    def showAllCells( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.addActor( self.outline_actor )
        self.outline_source.SetDetailMode( g_detail_modes.all_cells )
        self.updateVis()


# Constants for use by control_grid.  These are also the possible values of
# self.cell_detail_button_tag.
class _ButtonTags:
    Nothing = 'Nothing'
    Bounding_boxes = 'Bounding boxes'
    Face_cells = 'Face cells'
    All_cells = 'All cells'    
g_button_tags = _ButtonTags()

# These have to match the #define's in ../Vtk/vtkChomboOutlineSource.h!
class _DetailModes:
    box_edges   = 1
    face_cells  = 2
    all_cells   = 3
    slice_cells = 4
g_detail_modes = _DetailModes()
