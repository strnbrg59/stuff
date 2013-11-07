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

# File: vtk_eb.py
# Author: TDSternberg
# Created: 11/15/01

import anag_utils
from self_control import SelfControl
from vtk_data import g_ebBlankCoveredName
import vtk_glyphs
import vtk_slice
import algorithms

import vtkpython
import libVTKChomboPython

import types

# These are for blanking EB covered cells:
g_blankCoveredFunc = None
g_blankingCallback = None
g_eb_discriminator_name = '__covered-' # 0=covered, 1=irregular, 2=regular
g_decls = anag_utils.Declarations( 'g_decls', 'g_blankCoveredFunc',
    'g_blankingCallback', 'g_eb_discriminator_name' )

class VtkEb( SelfControl ):
    
    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        instance_data = [
            {'name':'is_visible', 'get':1, 'set':2, 'save':5, 'initval':0 },
            {'name':'selected_ebs', 'initval':[], 'get':1, 'set':1,'save':5 },
            {'name':'selected_eb_notifier', 'notify':1},
            {'name':'eb_func'},
            {'name':'accum'},
            {'name':'mapper', 'get':1},
            {'name':'prop', 'get':1},
            {'name':'clip_others', 'get':1, 'set':2, 'initval':0, 'save':1},
            {'name':'actor', 'get':1, 'set':1},
            { 'name':'use_one_color', 'get':1, 'initval':1, 'save':5},  
                # If 1, shade embedded boundary with constant color
            { 'name':'the_one_color', 'get':1, 'initval':(1.0,1.0,1.0),
              'save':5}, # For when use_one_color==1
            { 'name':'offset_transform'},
            { 'name':'offset_transform_polydatafilter'},
            { 'name':'capping', 'get':1, 'set':2, 'save':5, 'initval':1 },
                # 0 or 1
            { 'name':'normal_outward', 'get':1, 'set':2,'initval':0,'save':5},
                # 1=>outward, 0=>inward.
            { 'name':'deferred_update', 'initval':1},
                # Set to 1 when we do something that, were eb visible, would
                # require a vtkUpdate to see the results of.  We check this var
                # in setIsVisible(); that's how we know if we need to update or
                # if it's enough just to add the actor.
            { 'name':'is_clipped', 'get':1, 'set':2, 'initval':0, 'save':5},
            { 'name':'opacity', 'get':1, 'set':2, 'save':5, 'initval':1.0},
            { 'name':'line_width', 'get':1, 'set':2, 'initval':1, 'save':1},
            { 'name':'blank_covered_cells', 'get':1, 'set':2, 'initval':0,
                'save':1},
            { 'name':'local_vtk_data', 'get':1 },
            { 'name':'glyphs'},
            { 'name':'show_glyphs', 'initval':0, 'set':2, 'save':1}
        ]
        SelfControl.__init__( self,dep_dict, instance_data)

        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'instance_data', 'dep_dict' )

        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):        
        anag_utils.funcTrace()

        if(  ( self.vtk_data.getNumEBs() == 0 )
          or ( self.vtk_data.isResliceMode() == 1 ) ):
            return

        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=1,
            follow_global_curcomponent=1,
            follow_global_padded_boxes=1,
            always_use_real_boxes=0,
            instance_identifier='eb' )

        # Initialize self.selected_ebs -- a list of flags.
        for i in range(0, self.local_vtk_data.getNumEBs()):
            self.selected_ebs.append(0)
        self.selected_ebs[0] = 1

        self.eb_func = libVTKChomboPython.vtkChomboEmbeddedBoundaryFilter()
        if self.local_vtk_data.is2DMode():  ndims = 2
        else:                               ndims = 3
        self.eb_func.SetnDim( ndims )
        self.eb_func.SetInput( self.local_vtk_data.getReader().GetOutput() )

        self.accum = libVTKChomboPython.vtkChomboAccumulatePolyData()
        self.accum.SetInput( self.eb_func.GetOutput() )

        self.mapper = self.vtk_vtk.newMapper()
        self.mapper.SetInput( self.accum.GetOutput() )
        self.mapper.SetColorModeToMapScalars()
        self.mapper.SetScalarModeToUseCellData()
        self.mapper.ScalarVisibilityOff()
        self.mapper.SetLookupTable( self.vtk_cmap.getActiveColormap() )
        self.mapper.ImmediateModeRenderingOn()

        self.prop = self.vtk_vtk.newProperty()
        self.prop.SetColor( 1.0, 1.0, 1.0 )
        if self.local_vtk_data.is2DMode():
            self.prop.SetDiffuse( 1.0 )
            self.prop.SetAmbient( 0.0 )
        else:
            self.prop.SetDiffuse( 0.8 )
            self.prop.SetAmbient( 0.2 )

        self.actor = vtkpython.vtkLODActor()
        self.actor.SetProperty( self.prop )
        self.actor.SetMapper( self.mapper )

        # This 2D stuff is all about putting the embedded boundary out in front
        # of the boxes, so you can see it better.
        if self.local_vtk_data.is2DMode():
            self.offset_transform = vtkpython.vtkTransform()
            self.offset_transform.Identity()
            self.offset_transform_polydatafilter =\
                vtkpython.vtkTransformPolyDataFilter()
            self.offset_transform_polydatafilter.SetTransform(
                self.offset_transform )
            self.offset_transform_polydatafilter.SetInput(
                self.eb_func.GetOutput() )
            self.accum.SetInput(
                self.offset_transform_polydatafilter.GetOutput())

        self.glyphs = vtk_glyphs.VtkGlyphs(
            dep_dict={'vtk_vtk':self.vtk_vtk,
                      'vtk_data':self.vtk_data,
                      'vtk_cmap':self.vtk_cmap,
                      'saved_states':self.saved_states},
            multi_instance_qualifier='eb',
            caller_updater = lambda self=self, source=str(self.__class__) :
                self.local_vtk_data.vtkUpdate(source=source) )


    def _refresh( self ):
        anag_utils.funcTrace()
        if(  (self.vtk_data.getNumEBs() == 0)
          or ( self.vtk_data.isResliceMode() == 1 )):
            return

        self.changeColor( self.use_one_color, self.the_one_color )
        self.setCapping( self.capping )
        self.setNormalOutward( self.normal_outward )
        self.mapper.SetLookupTable( self.vtk_cmap.getActiveColormap() )
        self.setIsVisible( self.is_visible, force=1 )
        self.setIsClipped( self.is_clipped )
        if not self.local_vtk_data.is2DMode():
            self.setOpacity( self.opacity )
        self.setLineWidth( self.line_width )
        self.setBlankCoveredCells( self.blank_covered_cells )

        # Reregister with Notifiers.
        self.registerCallback(
            self.vtk_cmap.getNotifierActiveColormap(), self.updateColormap )
        self.registerCallback(
            self.local_vtk_data.getNotifierAccumStart(), self.updateAccumStart )
        self.registerCallback(
            self.local_vtk_data.getNotifierAccumMiddle(),self.updateAccumMiddle)
        self.registerCallback(
            self.local_vtk_data.getNotifierAccumEnd(), self.updateAccumEnd )
        self.registerCallback(
            self.local_vtk_data.getNotifierCurComponent(), self.changeComponent)
        map( lambda notifier, self=self :
            self.registerCallback( notifier, self.cmappedRangeCallback ),
            self.local_vtk_data.getCmappedRangeMinNotifiers()
           +self.local_vtk_data.getCmappedRangeMaxNotifiers() )
        self.registerCallback(
            self.local_vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicCallback )
        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )

        self.local_vtk_data.vtkUpdate( source = str(self.__class__) )


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        apply( self.mapper.SetScalarRange,
               self.vtk_cmap.getDefactoRange() )


    def cleanup( self ):
        anag_utils.funcTrace()
        if self.local_vtk_data:
            self.setIsVisible( 0 )


    def _anisotropicCallback( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self.updateVis()

    def updateAccumStart( self, dummy, extra_info ):
        anag_utils.funcTrace()
        if self.notFromHereOrVtkData( extra_info ):
            return
        if self.is_visible==0:
            self.deferred_update = 1
            return
        self.accum.StreamExecuteStart()

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
        if self.notFromHereOrVtkData( extra_info ) or (self.is_visible==0):
            return

        decls = anag_utils.Declarations( 'self', 'box_num', 'decls',
            'extra_info', 'i', 'piece' )

        for i in range(0, self.local_vtk_data.getNumEBs()):
            if self.selected_ebs[i] == 1:
                self.local_vtk_data.getReader().SetEBIndex( i )
                self.local_vtk_data.getReader().Execute()
                if self.local_vtk_data.is2DMode():
                    self.offset_transform.Identity()
                    self.offset_transform.Translate( 0.0, 0.0, 0.10 )
                    self.offset_transform_polydatafilter.Update()
                else:
                    self.eb_func.Execute() # Update() doesn't cut it; it doesn't
                                           # always call Execute().
                self.accum.Append()

        if self.show_glyphs == 1:
            piece = self.local_vtk_data.getReader().GetCurrentPieceIndex()
            self.glyphs.updateAccumMiddle( piece, self.eb_func.GetOutput() )

        decls.memberFunctionAudit(self)


    def updateAccumEnd( self, dummy, extra_info ):
        anag_utils.funcTrace()
        if self.notFromHereOrVtkData( extra_info ) or (self.is_visible==0):
            return
        self.accum.StreamExecuteEnd()

        if self.show_glyphs == 1:
            self.glyphs.updateAccumEnd()


    def cmappedRangeCallback( self, visible_range, extra_info ):
        """
        Callback function for the Notifier local_vtk_data.visible_range_max|min.
        """
        anag_utils.funcTrace()
        self.changeComponent()


    def updateColormap( self, new_colormap, extra_info ):
        """
        Callback registered with vtk_cmap.active_colormap.
        """
        anag_utils.funcTrace()
        self.mapper.SetLookupTable( new_colormap )


    def changeComponent( self, dummy1=None, dummy2=None ):
        """ 
        What to do when the component changes.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls", "self", "dummy1", "dummy2" )
    
        apply( self.mapper.SetScalarRange,
               self.vtk_cmap.getDefactoRange() )


    def setShowGlyphs( self, on_off ):
        anag_utils.funcTrace()
        assert( on_off==0  or  on_off==1 )
        self.show_glyphs = on_off
        if on_off == 1:
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
        else:
            self.glyphs.hide()


    def setDiffuse( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.prop:  # None if no file loaded
            self.prop.SetDiffuse( x )
            self.vtk_vtk.render()
    def setAmbient( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.prop:  # None if no file loaded
            self.prop.SetAmbient( x )
            self.vtk_vtk.render()
    def setSpecular( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        if self.prop:  # None if no file loaded
            self.prop.SetSpecular( x )
            self.vtk_vtk.render()


    def setIsClipped( self, on_off ):
        """ Toggle clipping """
        anag_utils.funcTrace()
        self.is_clipped = on_off
        if on_off == 1:
            self.vtk_clip.clipPipeline( self.accum, self.mapper )
        else:
            self.vtk_clip.unclipPipeline( self.accum, self.mapper )


    def setLineWidth( self, w ):
        self.line_width = w
        self.prop.SetLineWidth(w)
        self.vtk_vtk.render()


    def selectBoundary( self, boundary_num, on_off ):
        """
        Selects the particular embedded boundary to display (or not).
        """
        anag_utils.funcTrace()
        assert( boundary_num != None )

        if self.selected_ebs[boundary_num] == on_off:
            return

        self.selected_ebs[boundary_num] = on_off
        self.local_vtk_data.getReader().SetEBIndex( boundary_num )

        if self.is_visible == 0:
            self.deferred_update = 1
        else:
            self.updateVis()
        self.setSelectedEbNotifier(1)


    def getFirstSelectedBoundary( self ):
        """
        Returns number of lowest-numbered boundary that's being displayed.
        """
        anag_utils.funcTrace()
        for b in range(0,len(self.selected_ebs)):
            if self.selected_ebs[b] == 1:
                return b
        return None


    def setIsVisible( self, on_off, force=0 ):
        """
        Toggle visibility of all selected EBs together.  That is, with
        self.selectBoundary() you select the particular EBs you want visible,
        but with this function you can turn them all off (or back on), say, if
        you just want to see what's behind the EBs.
        """
        anag_utils.funcTrace()
        assert( (on_off==0) or (on_off==1) )

        self.local_vtk_data.getReader().SetEBIsVisible( on_off )

        if (on_off != self.is_visible) or (force==1) :
            self.is_visible = on_off
            if on_off == 1:
                self.vtk_vtk.addActor( self.actor )
                if (self.deferred_update == 1) or (force==1):
                    self.updateVis()
                    self.deferred_update = 0
                else:
                    self.vtk_vtk.render()
            else:
                self.vtk_vtk.removeActor( self.actor )
                self.vtk_vtk.render()


    def updateVis( self ):
        anag_utils.funcTrace()

        if self.local_vtk_data.isResliceMode():
            # A mere slice of the 3D data won't have what we need to really
            # compute a 2D EB.
            return

        if self.getNumSelectedEBs() > 0:
            if self.is_visible == 1:
                self.vtk_vtk.addActor( self.actor )
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
        else:
            self.vtk_vtk.removeActor( self.actor )
        self.vtk_vtk.render()


    def setOpacity( self, x ):
        """
        Set the opacity (between 0 and 1 inclusive) of the EB(s) displayed.
        """
        anag_utils.funcTrace()
        if self.local_vtk_data.is2DMode():
            anag_utils.error( "EB opacity only works in 3D mode." )
            return
        self.opacity = x
        self.prop.SetOpacity( x )
        self.vtk_vtk.render()


    def changeColor( self, use_one_color, rgb=None ):
        """
        Render the boundary in a single color, or at least save that color
        for use as soon as self.use_one_color is set to 1.

        Arg use_one_color==1 means, color all values of the boundary with
        a single color -- specified by arg rgb.
        When use_one_color==0, color different values differently
        (according to the loaded colormap).

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
            self.mapper.ScalarVisibilityOn()
        else:
            self.use_one_color = 1
            self.mapper.ScalarVisibilityOff()
            self.prop.SetColor( self.the_one_color[0],
                                self.the_one_color[1],
                                self.the_one_color[2] )
    
        self.vtk_vtk.render()

    
    def setCapping( self, yes_no ):
        """
        0 => no capping in EB representation.
        1 => capping
        """
        anag_utils.funcTrace()
        assert( (yes_no == 0) or (yes_no == 1) )
        self.capping = yes_no
        self.eb_func.SetCapping( yes_no )
        if self.is_visible == 0:
            self.deferred_update = 1
        else:
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )


    def setNormalOutward( self, yes_no ):
        """
        0 => normal is inward (toward fluid -- EB Chombo convention)
        1 => outward (out of fluid)
        """
        anag_utils.funcTrace()
        assert( (yes_no == 0) or (yes_no == 1) )
        self.normal_outward = yes_no
        self.eb_func.SetNormalDir( yes_no )
        if self.is_visible == 0:
            self.deferred_update = 1
        else:
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )


    def getNumSelectedEBs( self ):
        """
        Returns the number of EBs that are selected to be displayed (i.e. the
        number of 1 elements in self.selected_ebs).
        """
        anag_utils.funcTrace()
        return len(filter(lambda i: i==1, self.selected_ebs))


    def setBlankCoveredCells( self, yes_no ):
        """
        If arg yes_no==1, then don't show covered cells.
        """
        global g_blankCoveredFunc, g_blankingCallback
        anag_utils.funcTrace()
        assert( yes_no==0  or  yes_no==1 )

        """
        Implementation note:
        Defines a new component, just like the current one, but where the
        covered cells have had their values changed to something a little
        off the left end of the range, so we can use the "black outliers"
        feature to hide them.  Note this feature assumes you're displaying
        only one EB; it only looks for the covered cells of the first
        (lowest-numbered) displayed EB.  So you can display more than one EB
        but you'll only get blanking on the first EB's covered cells.
        """
        self.blank_covered_cells = yes_no

        if( (yes_no == 0)
        and (not g_ebBlankCoveredName in self.local_vtk_data.getComponentList())
        ) :
            return
            # That was for if this is called when we've never called it with
            # a 1 arg even.

        if not g_blankingCallback:
            g_blankingCallback = lambda d1,d2 : self.setBlankCoveredCells(1)


        if yes_no == 0:
            self.vtk_slice.setCurrentComponent( self.vtk_data.getCurComponent())
            self.vtk_cmap.setOutlierColors(((0,0,0,1),(0,0,0,1)))
            self.vtk_cmap.setBlackOutliers(0)
            map( lambda notifier, self=self :
                     self.unregisterCallback(
                         notifier,
                         g_blankingCallback ),
                 (self.vtk_data.getNotifierMinVisibleLevel(),
                  self.vtk_data.getNotifierMaxVisibleLevel(),
                  self.vtk_data.getNotifierCurComponent(),
                  self.getNotifierSelectedEbNotifier()))
            return

        comp_name = self.vtk_data.getCurComponent()
        comp_num = self.vtk_data.getComponentList().index(comp_name)
        span = list(
               self.vtk_data.getReader().GetLevelComponentRange(0,comp_num))
        for l in range(0, self.vtk_data.getReader().GetNumLevels()):
            span_l =\
                self.vtk_data.getReader().GetLevelComponentRange(l,comp_num)
            span[0] = min(span[0],span_l[0])
            span[1] = max(span[1],span_l[1])
        outlier = span[0] - 0.01*(span[1]-span[0])
        if( span[1] == span[0] ):
            outlier = span[0] - 0.01*span[0]
            if( span[0] == 0.0 ): outlier = -0.01

        def shiftOff( frac, comp, outlier ):
            # I.e. shift covered cells' values off the left end, to "outlier".
            if algorithms.floatEquals(frac, 0.0, 0.0000001):
                return outlier
            else:
                return comp
        
        eb_index = self.getFirstSelectedBoundary()
        if eb_index == None:
            anag_utils.error( 'Must select an EB before calling this '
                'function' )
            return
        boundary_name = g_eb_discriminator_name + str(eb_index)

        self.vtk_cmap.setOutlierColors(((0,0,0,0),(0,0,0,1)))
        self.vtk_cmap.setBlackOutliers(1)

        if comp_name != g_ebBlankCoveredName:
            if g_blankCoveredFunc:
                g_blankCoveredFunc = lambda f, c: shiftOff(f,c,outlier)
                self.vtk_data.redefineNewComponent( g_ebBlankCoveredName,
                    g_blankCoveredFunc, (boundary_name, comp_name) )
            else:
                g_blankCoveredFunc = lambda f, c: shiftOff(f,c,outlier)
                self.vtk_data.defineNewComponent( g_ebBlankCoveredName,
                    g_blankCoveredFunc, (boundary_name, comp_name) )

            self.vtk_slice.setCurrentComponent(
                self.vtk_data.getComponentList()[0] ) # flushes data

            map( lambda notifier, self=self :
                     self.registerCallback(
                         notifier,
                         g_blankingCallback ),
                 (self.vtk_data.getNotifierMinVisibleLevel(),
                  self.vtk_data.getNotifierMaxVisibleLevel(),
                  self.vtk_data.getNotifierCurComponent(),
                  self.getNotifierSelectedEbNotifier()))
        self.vtk_slice.setCurrentComponent( g_ebBlankCoveredName )
        if self.local_vtk_data.is2DMode():
            self.vtk_slice.getLocalVtkData().setSliceOffset(0.0)
            # Otherwise, lower level cells can be visible under clipped-away
            # higher-level layer, and it looks like the clipping only followed
            # the lower-level data.  We could go with a zero slice offset
            # at all times, just like in 3d, but it's unnecessary and there's a
            # cost from generating and loading subdivided boxes.
    
    def unitTest( self ):
        anag_utils.funcTrace()
        if( (self.saved_states.getNumHDF5sLoaded() > 0)
        and (self.local_vtk_data.getNumEBs() > 0) ):
            self.setIsVisible( 1 )
            self.changeColor( use_one_color=1, rgb=(1.0,0.4,0.0) )


