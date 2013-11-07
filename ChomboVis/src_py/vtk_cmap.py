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

# File: vtk_cmap.py
# Author: TDSternberg
# Created: 6/11/01

import anag_utils
from self_control import SelfControl

import math
import vtkpython
import libVTKChomboPython

class VtkCmap( SelfControl ):

    def __init__(self, dep_dict):

        SelfControl.__init__( self, dep_dict,
          metadata = [ 
            { 'name':'user_colormap_filename', 'save':1, 'get':1,
              'initval':None },
            { 'name':'default_colormap'},
            { 'name':'default_greymap'},
            { 'name':'user_colormap', 'get':1, 'set':1 },
            { 'name':'cmap_editor', 'get':1},
            { 'name':'active_colormap', 'get':1, 'set':2, 'notify':1 },
            { 'name':'cmap_options', 'get':1 },
            { 'name':'active_colormap_choice', 'get':1, 'save':1 },
                # Default_grey, Default_color, User_file, User_generated
            { 'name':'num_colors', 'initval':256},
            { 'name':'background_color', 'initval':(0,0,0), 'get':1, 'set':2,
              'save':1, 'notify':1 },
            { 'name':'legend_background_actor' },
            { 'name':'bar_actor' },
            { 'name':'bar_actor_width', 'save':1, 'get':1, 'set':2,
                'initval':0.2 },
            { 'name':'legend_is_visible', 'initval':0, 'get':1, 'set':2,
              'save':2 },
            { 'name':'black_outliers', 'get':1, 'set':2, 'initval':0,
              'save':1, 'notify':1 },
            { 'name':'outlier_colors', 'get':1, 'set':2, 'save':1,
              'initval':((0,0,0,1),(0,0,0,1))}
          ])


        self.decls = anag_utils.Declarations( "decls",
            instance=self
            )
        decls = anag_utils.Declarations( 'decls', 'metadata', 'dep_dict',
            'CmapOptions' )

        class CmapOptions:
            Default_grey = "Default greyscale"
            Default_color = "Default color"
            User_file = "User file"
            User_generated = "User generated"
        self.cmap_options = CmapOptions()
        self.cmap_editor = CmapEditor(
            dep_dict={ 'vtk_vtk':self.vtk_vtk,
                       'saved_states':self.saved_states })

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        self.active_colormap_choice = self.cmap_options.Default_color
        self._initDefaultLookupTables()

        self.bar_actor=vtkpython.vtkScalarBarActor()
        self.legend_background_actor = vtkpython.vtkActor2D()
        

    def _refresh( self ):
        """
        Overrides SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )
        if self.cmd_line.getNoVtk():
            return

        self._refreshLegend()

        if self.active_colormap_choice == self.cmap_options.User_file:
            self.user_colormap = self.loadColormapFromFile(
                                                self.user_colormap_filename )
        self.switchColormap( self.active_colormap_choice )


        # No need to refresh min and max entryfields; that's done in vtk_data.

        bkgrnd = self.background_color.get()
        self.setBackgroundColor( bkgrnd )

        if self.vtk_data.getCurComponent():
            self._setColormapRange( self.vtk_data.getCmappedRangeMin(),
                                    self.vtk_data.getCmappedRangeMax() )

        self.setLegendIsVisible( self.legend_is_visible )

        self.registerCallback(
            self.vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        map( lambda notifier, self=self :
            self.registerCallback( notifier, self._cmapMinCallback ),
            self.vtk_data.getCmappedRangeMinNotifiers() )
        map( lambda notifier, self=self :
            self.registerCallback( notifier, self._cmapMaxCallback ),
            self.vtk_data.getCmappedRangeMaxNotifiers() )
        self.registerCallback(
            self.vtk_data.getNotifierNewComponentNotifier(),
            self._newComponentNotifierHandler )

        self.vtk_vtk.render()


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()


    def _refreshLegend( self ):
        """ Colormap legend """
        anag_utils.funcTrace()

        # legend_background_* stuff is the opaque background, that helps set
        # the legend off from the rest of the VTK rendering.
        legend_background_source = libVTKChomboPython.vtkChomboPlaneSource()
        legend_background_source.SetOrigin( 0, 0, 0 )
        legend_background_source.SetPoint2(
            self.vtk_vtk.getRenWin().GetSize()[0]
            * self.bar_actor_width*1.5, 0, 0 )
        legend_background_source.SetPoint1(
            0,
            self.vtk_vtk.getRenWin().GetSize()[1]*5, 0 )

        legend_background_transf = vtkpython.vtkTransform()
        legend_background_transf.Identity()

        legend_background_transf_p = vtkpython.vtkTransformPolyDataFilter()
        legend_background_transf_p.SetTransform( legend_background_transf )
        legend_background_transf_p.SetInput(
            legend_background_source.GetOutput() )

        legend_background_mapper = vtkpython.vtkPolyDataMapper2D()
        legend_background_mapper.SetInput(
            legend_background_transf_p.GetOutput() )

        legend_background_prop = vtkpython.vtkProperty2D()

        self.legend_background_actor.SetProperty( legend_background_prop )
        self.legend_background_actor.SetMapper( legend_background_mapper )
        bkgrnd = self.background_color.get()
        self.legend_background_actor.GetProperty().SetColor(
            1-bkgrnd[0], 1-bkgrnd[1], 1-bkgrnd[2] )

        # The bar_actor is the legend itself.
        self.bar_actor.GetPositionCoordinate(
            ).SetCoordinateSystemToNormalizedViewport()
        self.bar_actor.GetPositionCoordinate().SetValue( 0.01, 0.11 )
        self.bar_actor.SetOrientationToVertical()
        self.bar_actor.SetWidth( self.bar_actor_width )
        self.bar_actor.SetHeight( 0.80 )
        #self.bar_actor.ShadowOff()
        self.bar_actor.GetProperty().SetColor( bkgrnd[0], bkgrnd[1], bkgrnd[2] )

        self.vtk_vtk.render()


    def setBarActorWidth( self, w ):
        """
        As a fraction of the renderwindow's width.  A good default would be 0.2.
        """
        anag_utils.funcTrace()
        assert( 0.0 < w < 1.0 )
        self.bar_actor_width = w
        self._refreshLegend()


    def _newComponentNotifierHandler( self, compname, unused ):
        anag_utils.funcTrace()
        map( lambda notifier, self=self :
            self.registerCallback( notifier, self._cmapMinCallback ),
            self.vtk_data.getCmappedRangeMinNotifiers() )
        map( lambda notifier, self=self :
            self.registerCallback( notifier, self._cmapMaxCallback ),
            self.vtk_data.getCmappedRangeMaxNotifiers() )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.setLegendIsVisible(0)


    def _cmapMinCallback( self, x, dummy ):
        """
        Resets the cmap range.
        """
        anag_utils.funcTrace()
        if self.vtk_data.getCmappedRangeMin()==None:
            return
        self._setColormapRange( x, self.vtk_data.getCmappedRangeMax() )
        self.vtk_vtk.render()
    def _cmapMaxCallback( self, x, dummy ):
        """
        Resets the cmap range.
        """
        anag_utils.funcTrace()
        if self.vtk_data.getCmappedRangeMax()==None:
            return
        self._setColormapRange( self.vtk_data.getCmappedRangeMin(), x )
        self.vtk_vtk.render()


    def _initDefaultLookupTables(self):
        anag_utils.funcTrace()
        self.default_colormap = self.buildDefaultColormap()
        self.default_greymap = self.buildDefaultGreymap()


    def buildDefaultColormap( self ):
        anag_utils.funcTrace()
        result = libVTKChomboPython.vtkChomboLookupTable()
        result.SetHueRange( 0.0, 0.666667 )
        result.SetNumberOfColors( 256 )
        result.SetTableRange( 0,255 )
        result.SetSaturationRange( 1, 1 )
        result.SetValueRange( 1, 1 )
        result.SetAlphaRange( 1, 1 )
        result.Build()
        return result


    def buildDefaultGreymap( self ):
        anag_utils.funcTrace()
        result = libVTKChomboPython.vtkChomboLookupTable()
        result.SetHueRange( 0.0, 0.0 )
        result.SetNumberOfColors( 256 )
        result.SetTableRange( 0, 255 )
        result.SetSaturationRange( 0, 0 )
        result.SetValueRange( 0, 1 )
        result.SetAlphaRange( 1, 1 )
        result.Build()
        return result


    def setBackgroundColor( self, rgb ):
        """
        Called from control_cmap's colorwheel.
        Arg rgb is a tuple.

        self.background_color is a Notifier.
        """
        anag_utils.funcTrace()
        self.background_color.set( rgb )
        self.legend_background_actor.GetProperty().SetColor(
            1-rgb[0], 1-rgb[1], 1-rgb[2] )
        self.bar_actor.GetProperty().SetColor( rgb[0], rgb[1], rgb[2] )
        self.vtk_vtk.getRenderer().SetBackground( rgb[0], rgb[1], rgb[2] )
        self.vtk_vtk.render()


    def setActiveColormap( self, cmap ):
        """
        Should only be called from self.switchColormap().

        self.active_colormap is a Notifier.
        The extra side effect we want here is to trigger
        self._cmapMinCallback().  We need that or the newly-loaded colormap
        won't reflect cmap min and max.  (No need to trigger _cmapMaxCallback(),
        as _cmapMinCallback() is enough; we just need it to call SetRange()
        on the new colormap.)
        """
        anag_utils.funcTrace()
        self.active_colormap.set( cmap )
        if( self.vtk_data.getCurComponent() in
          self.vtk_data.cmapped_range_min.keys() ):
            # Won't be there early in initialization stage.
            self.refreshCmapRange()
        if self.legend_is_visible == 1:
            self.setLegendIsVisible( 1 ) # Updates colors on legend bar.


    def refreshCmapRange( self ):
        anag_utils.funcTrace()
        if not self.getActiveColormap():  # True if in no-VTK mode.
            return
        apply( self._setColormapRange,
               self.getDefactoRange() )


    def _setColormapRange( self, lo, hi ):
        """
        Call vtkLookupTable.SetRange(), but if self.black_outliers==1,
        we actually set the range 256/254-ths larger, so that the very highest
        and very lowest of the colormap's 256 values (convenient if they're
        black) are reserved for values outside the [lo,hi] range.
        """
        anag_utils.funcTrace()

        if self.black_outliers.get() == 1:
            xtra = (hi-lo)/253.99
            if hi == lo:
                xtra = 1.0  # Doesn't matter, as long as it's >0
            self.getActiveColormap().SetRange( lo-xtra, hi+xtra )
        else:
            self.getActiveColormap().SetRange( lo, hi )
        self.vtk_vtk.render()


    def getDefactoRange( self ):
        """
        Returned vtk_data.cmap_range, but if self.black_outliers==1, expanded
        a little.
        """
        anag_utils.funcTrace()
        lo, hi = ( self.vtk_data.getCmappedRangeMin(),
                   self.vtk_data.getCmappedRangeMax() )
        if self.black_outliers.get() == 1:
            xtra = (hi-lo)/253.99
            if hi == lo:
                xtra = 1.0  # Doesn't matter, as long as it's >0
            return (lo-xtra, hi+xtra)
        else:
            return (lo,hi)


    def setOutlierColors( self, lo_hi ):
        """
        Set the colors for the low- and high-end outliers (which will show if
        you've made the call "setBlackOutliers(1)").
        Argument lo_hi is a 2-tuple (lo & hi) of 4-tuples (colors in rgba form).
        """
        anag_utils.funcTrace()
        assert( len(lo_hi) == 2 )
        assert( len(lo_hi[0]) == 4 )
        assert( len(lo_hi[1]) == 4 )
        self.outlier_colors = lo_hi
        self.setBlackOutliers( self.black_outliers.get(), force=1 )


    def setBlackOutliers( self, one_or_zero, force=None ):
        """
        See comments under _setColormapRange().
        """
        anag_utils.funcTrace()
        if not self.getActiveColormap():
            return

        if (one_or_zero == self.black_outliers.get()) and (not force):
            return

        if one_or_zero == 1:
            apply( self.getActiveColormap().SetTableValue,
                   tuple([0,] + list(self.outlier_colors[0])))
            apply( self.getActiveColormap().SetTableValue,
                   tuple([255,] + list(self.outlier_colors[1])))
            self.getActiveColormap().Build()
        elif self.active_colormap_choice==self.cmap_options.User_generated:
            # SetTableValue(), which is how we implement "black outliers",
            # disables any further calls to SetHueRange() etc, unless you call
            # ForceBuild:
            self.getActiveColormap().ForceBuild()
        else:
            unaltered_cmap = None
            if self.active_colormap_choice == self.cmap_options.Default_grey:
                unaltered_cmap = self.buildDefaultGreymap()
            elif self.active_colormap_choice == self.cmap_options.Default_color:
                unaltered_cmap = self.buildDefaultColormap()
            elif self.active_colormap_choice == self.cmap_options.User_file:
                unaltered_cmap = self.loadColormapFromFile(
                                            self.user_colormap_filename)
            else:
                anag_utils.fatal( "self.active_colormap_choice=",
                                   self.active_colormap_choice )

            lo = list(unaltered_cmap.GetTableValue(0))
            hi = list(unaltered_cmap.GetTableValue(255))
            
            apply( self.getActiveColormap().SetTableValue,
                   tuple( [0,] + lo ) )
            apply( self.getActiveColormap().SetTableValue,
                   tuple( [255,] + hi ) )


        self.black_outliers.set( one_or_zero ) # It's a notifier
        try:
            self._setColormapRange( self.vtk_data.getCmappedRangeMin(),
                                    self.vtk_data.getCmappedRangeMax() )
        except:
            anag_utils.info( "No cmapped range yet." )


    def switchColormap( self, button_tag ):
        """
        Switch the coloration mode among grey, default-color, and user.
        If user, then load self.user_colormap.

        Callback function from control_cmap's radiobuttons.
    
        Arg button_text is the tag of the radiobutton selected
        Arg butt_names is the named constants member of the RadioSelect
            widget whose tags include button_tag.
            We pass it in so we can use its "type-safe" tag-name attributes.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( "decls",
            "button_tag", "butt_names", "cmap",
            "f"
            )

        self.active_colormap_choice = button_tag    
        if button_tag == self.cmap_options.Default_grey:
            self.setActiveColormap( self.default_greymap )  # Notifier
        elif button_tag == self.cmap_options.Default_color:
            self.setActiveColormap( self.default_colormap )
        elif button_tag == self.cmap_options.User_file:
            if self.user_colormap == None:
                anag_utils.warning( "No user colormap loaded yet." )
            else:
                self.setActiveColormap( self.user_colormap )
        elif button_tag == self.cmap_options.User_generated:
            self.setActiveColormap( self.cmap_editor.getLookupTable() )
        else:
            anag_utils.fatal( "Illegal button_tag (=", button_tag, ")" )

        self.setBlackOutliers( self.black_outliers.get(), force=1 )
        self.vtk_vtk.render()
        decls.memberFunctionAudit(self)
    
    
    def loadColormapFromFile( self, filename ):
        """
        Read in a colormap file, load it into a new vtkLookupTable object,
        and return the vtkLookupTable.

        On error, throw an exception.
        Propagate all other exceptions (e.g. invalid infile).
        
        The first line on the colormap file must if "rgba", "RGBA", "hsva", or
        "HSVA" to indicate that red/green/blue/alpha or
        hue/saturation/value/alpha entries are to follow.
        The next 256 lines must each contain 4 numbers between 0.0 and 1.0.
        See color.cmap for an example.
        """
        anag_utils.funcTrace()

        #
        # Open the colormap file and find out what type (RGBA or HSVA) it is.
        #
        decls = anag_utils.Declarations( "decls", "self",
            "filename",
            "cmap_file",
            "file_line",
            "color_params",
            "e1","e2","e3",
            "i",
            "alpha",
            "datatype",
            "r","g","b",
            "result"
            )

        self.user_colormap_filename = filename
    
        try:
            cmap_file = open( filename )
        except: # The calling function is only interested in the message part
                # of any exception.
            raise "Failed to open file" + filename
    
        file_line = (cmap_file.readline())[:-1]
        if   file_line.upper().strip() == "RGBA":
            datatype = "rgba"
        elif file_line.upper().strip() == "HSVA":
            datatype = "hsva"
        else:
            raise "Invalid first line of colormap file: should be either "\
                  "RGBA or HSVA"
    
    
        #
        # Create a vtkLookupTable and read all the colors into it.
        #
        result = libVTKChomboPython.vtkChomboLookupTable()
        result.SetNumberOfTableValues(self.num_colors)
    
        for i in range(0,self.num_colors):
    
            file_line = cmap_file.readline()[:-1]
            if file_line == None:
                raise "Incomplete colormap file.  Needs " +\
                      str(self.num_colors+1)              +\
                      " lines, but contains only " + str(i) + " lines."
    
    
            color_params = file_line.split()
            if len(color_params) != 4:
                raise "Invalid colormap line: " + file_line + ": should have "\
                      "four numbers."
            (e1,e2,e3) = (float(color_params[0]),
                          float(color_params[1]),
                          float(color_params[2]))
            alpha = float(color_params[3])
    
    
            # Check the range of the color params.
            if( e1 < 0 or e1 > 1 or
                e2 < 0 or e2 > 1 or
                e3 < 0 or e3 > 1 or
                alpha  < 0  or  alpha  > 1
            ):
                raise "Out of range numbers in colormap line[" + str(i) +\
                      "]: " + file_line
    
            # Convert the entries so we have r,g,b and h,s,v and alpha.
            if   datatype == "rgba":
                (r,g,b) = (e1,e2,e3)
    
            elif datatype == "hsva":
                (r,g,b) = self._hsv2rgb( e1,e2,e3 )
    
            else:
                raise "Invalid datatype:"+datatype + " (should never get here!)"
            
    
            # Enter the color/opacity in each table as necessary.
            result.SetTableValue( i, r,g,b, alpha )

        decls.memberFunctionAudit(self)
        return result


    def _hsv2rgb( self, h, s, v ):
        """ Convert h,s,v to r,g,b
    
            h,s,v in [0.0,1.0]
            r,g,b in [0.0,1.0]
    
            Returns r,g,b as a tuple.
    
            This algorithm taken from Foley & Van Dam
        """
    
        decls = anag_utils.Declarations( "decls", "self",
            "h","s","v",
            "f",
            "p",
            "q",
            "t",
            "r","g","b",
            "i"
        )
    
        if v == 0.0 :
            r = g = b = 0.0
        else:
            if s == 0.0 :
                r = g = b = v
            else:
                h = h * 6.0
                if h >= 6.0 : h = 0.0
              
                i = math.floor( h )
                f = h - i
                p = v*(1.0-s)
                q = v*(1.0-s*f)
                t = v*(1.0-s*(1.0-f))
              
                if   i == 0:
                    r = v; g = t; b = p
                elif i == 1:
                    r = q; g = v; b = p
                elif i == 2:
                    r = p; g = v; b = t
                elif i == 3:
                    r = p; g = q; b = v
                elif i == 4:
                    r = t; g = p; b = v
                elif i == 5:
                    r = v; g = p; b = q
    
        decls.memberFunctionAudit(self)
        return r,g,b


    def showLegend( self, lookuptable, label ):
        """
        Draw a bar showing what values the various colors correspond to.
        """
        anag_utils.funcTrace()

        if self.bar_actor:
            self.hideLegend()
            self.bar_actor.SetLookupTable( lookuptable )
#           Adding a title makes it unpleasantly large, especially now that we
#           are putting an opaque background behind the legend.
#           self.bar_actor.SetTitle( label )
            self.vtk_vtk.addActor( self.legend_background_actor )
            self.vtk_vtk.addActor( self.bar_actor )
            self.vtk_vtk.render()

    def hideLegend( self ):
        anag_utils.funcTrace()
        self.vtk_vtk.removeActor( self.bar_actor, force=1 )
        self.vtk_vtk.removeActor( self.legend_background_actor, force=1 )
        self.vtk_vtk.render()

    def setLegendIsVisible( self, yes_no ):
        anag_utils.funcTrace()
        assert( yes_no==0  or  yes_no==1 )
        if yes_no == 1:
            self.showLegend( self.getActiveColormap(),
                            'Field data coloring legend' )
        else:
            self.hideLegend()
        self.legend_is_visible = yes_no


    def unitTest(self):
        self.setBackgroundColor( (0.8, 0.8, 1.0) )


class CmapEditor( SelfControl ):
    """
    Not to be confused with the class with this name in control_cmap.py
    """
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        SelfControl.__init__( self, dep_dict,
          metadata = [
            {'name':'lookup_table', 'get':1},
            {'name':'table_params', 'get':1, 'set':1, 'save':1,
               'initval':{'hue'       :[0.06,0.06],
                          'saturation':[0.00,1.00],
                          'value'     :[1.00,1.00],
                          'alpha'     :[1.00,1.00]} }
          ])
        self.lookup_table = vtkpython.vtkLookupTable()
        self.lookup_table.SetNumberOfColors(256)
        self.lookup_table.SetTableRange(0,255)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()


    def _refresh( self ):
        anag_utils.funcTrace()
        self.setHueRange( self.table_params['hue'] )
        self.setSaturationRange( self.table_params['saturation'] )
        self.setValueRange( self.table_params['value'] )
        self.setAlphaRange( self.table_params['alpha'] )
        self.lookup_table.Build()


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()


    """ Needed by cmap editor widget in control_cmap.py """
    def setHueRange( self, lo_hi_tuple ):
        anag_utils.funcTrace()
        self.lookup_table.SetHueRange( lo_hi_tuple)
        self.table_params['hue'] = lo_hi_tuple
        self.lookup_table.Build()
        self.vtk_vtk.render()
    def setSaturationRange( self, lo_hi_tuple ):
        anag_utils.funcTrace()
        self.lookup_table.SetSaturationRange( lo_hi_tuple)
        self.table_params['saturation'] = lo_hi_tuple
        self.lookup_table.Build()
        self.vtk_vtk.render()
    def setValueRange( self, lo_hi_tuple ):
        anag_utils.funcTrace()
        self.lookup_table.SetValueRange( lo_hi_tuple)
        self.table_params['value'] = lo_hi_tuple
        self.lookup_table.Build()
        self.vtk_vtk.render()
    def setAlphaRange( self, lo_hi_tuple ):
        anag_utils.funcTrace()
        self.lookup_table.SetAlphaRange( lo_hi_tuple)
        self.table_params['alpha'] = lo_hi_tuple
        self.lookup_table.Build()
        self.vtk_vtk.render()
    def getHueRange( self ):
        anag_utils.funcTrace()
        return self.table_params['hue']
    def getSaturationRange( self ):
        anag_utils.funcTrace()
        return self.table_params['saturation']
    def getValueRange( self ):
        anag_utils.funcTrace()
        return self.table_params['value']
    def getAlphaRange( self ):
        anag_utils.funcTrace()
        return self.table_params['alpha']
