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

# File: control_cmap.py
# Author: TDSternberg
# Created: 6/11/01

""" Create the control options area for data selection """

import algorithms
import anag_utils
import anag_megawidgets
from self_control import SelfControl


import Pmw
import Tkinter
import tkFileDialog
import tkMessageBox
import math
import sys

class ControlCmap( anag_megawidgets.SelfDescribingDialog, SelfControl ):

    def getShortDescription(self):
        return "Colormap"
    def getLongDescription(self):
        return "Select and modify colormaps"

    def __init__( self, dep_dict, master=None, **kw ):
        """ Lay out and initialize the widgets. """
        #
        # Preliminaries.
        #
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__(self, master, **kw)

        instance_vars = [
            { 'name':'colormap_min'},
            { 'name':'colormap_max'},
            { 'name':'user_colormap_button'},
            { 'name':'radiobuttons'},
            { 'name':'colormap_filename'},
            { 'name':'color_wheel'},
            {'name':'cmap_editor'},
            {'name':'cmap_editor_frame'},
            {'name':'color_legend'},              # checkbutton
            {'name':'color_legend_width'},
            {'name':'show_dialog', 'save':2, 'initval':0},
            {'name':'black_outliers'}
        ]
        SelfControl.__init__( self,dep_dict, instance_vars)
        self.decls = anag_utils.Declarations( "decls", instance=self )

        decls = anag_utils.Declarations( "decls", "self", "master", "kw",
            "dep_dict", 
            "instance_vars",
            "w",
            "user_colormap_button",
            "radioButtonHandler",
            "user_cmap_frame"
        )
        self.configure( title="Colormap" )
        self.dialog_inventory.registerDialog( self )

        #
        # Colormap radiobuttons
        #
        self.radiobuttons = anag_megawidgets.RadioSelect( 
            self.interior(),
            buttontype = 'radiobutton',
            orient = 'vertical',
            labelpos = 'nw',
            label_text = 'Current colormap:',
            hull_borderwidth = 2, hull_relief = 'groove'
            )
        map( self.radiobuttons.add, 
             (self.vtk_cmap.getCmapOptions().Default_grey,
              self.vtk_cmap.getCmapOptions().Default_color,
              self.vtk_cmap.getCmapOptions().User_file,
              self.vtk_cmap.getCmapOptions().User_generated) )
        self.radiobuttons.configure( state='disabled' )

        def radioButtonHandler( button_tag, self=self ):
            if( self.radiobuttons.getcurselection() ==
                    self.radiobuttons.butt_names.User_file
            and self.vtk_cmap.getUserColormap() == None ):
                self._activateColormapGenerator(0)
                self._getNewCmap()
            elif( self.radiobuttons.getcurselection() ==
                    self.radiobuttons.butt_names.User_generated ):
                self._activateColormapGenerator(1)
                self.vtk_cmap.switchColormap(
                    self.vtk_cmap.getCmapOptions().User_generated )
            else:
                self.vtk_cmap.switchColormap( button_tag )
                self._activateColormapGenerator(0)

        self.radiobuttons.configure(
            command = lambda button_tag, handler=radioButtonHandler:
                          handler( button_tag ))
        self.radiobuttons.pack( anchor=Tkinter.W )
        self.radiobuttons.invoke( 'Default color' )


        #
        # User colormap
        #
        user_cmap_frame = Tkinter.Frame( self.interior(), relief=Tkinter.GROOVE,
                                         borderwidth=2 )
        self.user_colormap_button = Tkinter.Button( 
            user_cmap_frame,
            text='User colormap:',
            command = self._getNewCmap )
        self.colormap_filename = Tkinter.Label( user_cmap_frame )
        self.user_colormap_button.pack( side='left' )
        self.user_colormap_button.configure( state='disabled' )
        self.colormap_filename.pack( side='left' )
        user_cmap_frame.pack( padx=22, pady=2, anchor=Tkinter.W )
    

        #
        # Colormap editor
        #
        self.cmap_editor_frame = Tkinter.Frame( self.interior() )
        self.cmap_editor_frame.pack(anchor='w')
        self.cmap_editor = CmapEditor( 
            dep_dict = {'saved_states':self.saved_states,
                        'top_frame':self.cmap_editor_frame,
                        'vtk_cmap_editor': self.vtk_cmap.getCmapEditor()} )


        anag_megawidgets.HorizRule( self.interior(), 250 ).pack()


        # Legend
        self.color_legend = anag_megawidgets.Checkbutton(
            self.interior(), text='Show color legend' )
        self.color_legend.configure(
            command = lambda butt = self.color_legend,
                self=self: self._showLegend( butt.get() ),
            state = 'disabled' )
        self.color_legend.pack(anchor='w')

        # Legend width
        self.color_legend_width = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Legend width',
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1
            )
        self.color_legend_width.configure( 
            scale_callback = lambda x, self=self:
                        self.vtk_cmap.setBarActorWidth(x),
            from_ = 0.05,
            to    = 0.95,
            scale_normal_resolution = 0.01,
            resolution = 0.01 )
        self.color_legend_width.pack()
        self.color_legend_width.configure(state='disabled')


        # Special coloring for outliers (values outside colormap range).
        self.black_outliers = anag_megawidgets.Checkbutton(
            self.interior(), text='Black outliers' )
        self.black_outliers.configure(
            command = self._blackOutliersHandler,
            state = 'disabled' )
        self.black_outliers.pack(anchor='w')

    
        #
        # Colormap minimum and maximum
        #
        self.colormap_min = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Colormap min',
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1
            )
        self.colormap_min.configure(
            scale_callback = lambda x, self=self:
                          self._colormapMinHandler( x ),
            state='disabled')
        self.colormap_min.pack( expand=1, fill=Tkinter.X,
                                padx=2, anchor=Tkinter.W )
        self.colormap_min.flip()
    
        self.colormap_max = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Colormap max',
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1
            )
        self.colormap_max.configure( 
            scale_callback = lambda x, self=self:
                        self._colormapMaxHandler( x ),
            state='disabled')
        self.colormap_max.flip()
        self.colormap_max.pack( expand=1, fill=Tkinter.X,
                                padx=2, anchor=Tkinter.W )
    
    
        #
        # ColorWheel
        #
        self.color_wheel = anag_megawidgets.ColorWheel(
            master = self.interior(),
            colorwheel_image = self.vtk_data.getChomboVisDataDir() +\
                "/ColorWheel.ppm",
            label_text = "Background color",
            show_rgb_entries = 1,
            command = lambda rgb, self=self:
                self.vtk_cmap.setBackgroundColor(rgb),
            relief = Tkinter.GROOVE, borderwidth = 2,
            rgb_initial_vals = {'r':0, 'g':0, 'b':0 }
            )
        self.color_wheel.pack( padx=2, pady=2, anchor=Tkinter.N )

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    

    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self ):
        """
        Overrides SelfControl._refresh().  See comments there.
        """
        anag_utils.funcTrace()

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        self.registerCallback(
            self.vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        self.radiobuttons.configure( state='normal' )
        self.user_colormap_button.configure( state='normal' )
        self.color_legend.configure( state = 'normal' )
        self.color_legend_width.configure(state='normal')
        self.black_outliers.configure( state = 'normal' )
        self.colormap_min.configure( state = 'normal' )
        self.colormap_max.configure( state = 'normal' )

        self.radiobuttons.setcurselection(
            self.vtk_cmap.getActiveColormapChoice() )

        user_cmap_filename = self.vtk_cmap.getUserColormapFilename()
        if user_cmap_filename != None:
            self.colormap_filename.configure( text=user_cmap_filename )

        self.color_legend.set( self.vtk_cmap.getLegendIsVisible() )
        self.color_legend_width.setScaleSterile(
            self.vtk_cmap.getBarActorWidth() )
        self.black_outliers.set( self.vtk_cmap.getBlackOutliers() )
        self.color_wheel.setRgbEntries( self.vtk_cmap.getBackgroundColor() )
        
        self._handleDataChange()

        map( lambda notifier, self=self:
            self.registerCallback(
                notifier, lambda d1,d2,self=self: self._handleDataChange()),
            (self.vtk_data.getNotifierCurComponent(),
             self.vtk_data.getNotifierMinVisibleLevel(),
             self.vtk_data.getNotifierMaxVisibleLevel()) )

        self.registerCallback( self.vtk_cmap.getNotifierBlackOutliers(),
            lambda d1,d2,self=self:
                self.black_outliers.set( self.vtk_cmap.getBlackOutliers() ))

    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.dialog_inventory.unregisterDialog( self )
        self.withdraw()


    def _blackOutliersHandler( self ):
        anag_utils.funcTrace()
        on_off = self.black_outliers.get()
        self.vtk_cmap.setBlackOutliers( on_off )

        # As long as you have "black outliers" active, the SetHueRange() etc
        # functions of the colormap editor won't work.  For more on this, see
        # vtk_cmap.py -- scan for "ForceBuild()".
        if self.cmap_editor:
            if on_off == 0:
                self.cmap_editor.enableWidgets()
            else:
                self.cmap_editor.disableWidgets()


    def _activateColormapGenerator( self, yes_no ):
        """
        Activate the 4-scale parametrized colormap definition widget.
        """
        anag_utils.funcTrace()
        if not self.cmap_editor:
            return
        if yes_no == 0:
            self.cmap_editor.forget()
        else:
            self.cmap_editor.pack()


    def _showLegend( self, on_off ):
        """
        Show or hide the color legend.
        """
        anag_utils.funcTrace()
        assert( on_off==1  or  on_off==0 )
        self.vtk_cmap.setLegendIsVisible( on_off )


    def _handleDataChange( self, dummy1=None, dummy2=None ):
        """
        Respond to a change in the displayed component, or to a change in the
        visible levels.
        The dummy args are for when this is called from the
        vtk_data.cur_component Notifier.
        """
        anag_utils.funcTrace()

        assert( self.vtk_data.getCurComponent() )

        raw_range = self.vtk_data.getRangeMax() - self.vtk_data.getRangeMin()
        if raw_range > 0:
            scale_resolution = pow( 10, math.floor(math.log10(raw_range/100)))
        else:
            scale_resolution = \
                min( algorithms.findResolution(self.vtk_data.getRangeMin()),
                     algorithms.findResolution(self.vtk_data.getRangeMax()))
        scale_from = self.vtk_data.getRangeMin()
        scale_to =   self.vtk_data.getRangeMax()

        rounded_down_min = algorithms.roundDown(
            scale_from,  -math.log10(scale_resolution))
        rounded_up_max = algorithms.roundUp(
            scale_to,  -math.log10(scale_resolution))
        self.colormap_min.configure(
            from_ = rounded_down_min,
            to    = scale_to,
            scale_soft_min = rounded_down_min,
            scale_soft_max = scale_to,
            resolution = scale_resolution,
            scale_normal_resolution = scale_resolution
        )
        self.colormap_min.setScaleSterile( 
            self.vtk_data.getCmappedRangeMin() )

        self.colormap_max.configure(
            from_ = scale_from,
            to    = rounded_up_max,
            scale_soft_min = scale_from,
            scale_soft_max = rounded_up_max,
            resolution = scale_resolution,
            scale_normal_resolution = scale_resolution
        )
        self.colormap_max.setScaleSterile( 
            self.vtk_data.getCmappedRangeMax() )


    def _getNewCmap( self ):
        """
        Pops up a tkFileDialog for the user to indicate his desired colormap
        file.  We get here in two ways: (1) user presses "Select file"
        button,
        or (2) user presses "User" radiobutton and self.user_colormap==None.
        The "File" field should contain the name of the currently-selected user
        colormap file -- self.vtk_cmap.getUserColormapFilename() (if !=None).
        """
        anag_utils.funcTrace()
    
        decls = anag_utils.Declarations( "decls", "self",
            "filename",
            "response"
            )
    
        filename = tkFileDialog.askopenfilename( 
                filetypes = [("Colormap Files", "*.cmap"), ("All Files", "*")],
                initialfile = self.vtk_cmap.getUserColormapFilename() )
    
        if filename != "":   # It's "" if user pressed dialog's "Cancel" button.
    
            try:
                self.vtk_cmap.setUserColormap(
                    self.vtk_cmap.loadColormapFromFile( filename ) )
                self.colormap_filename.configure( text=
                    filename[ filename.rfind('/') + 1: ])

                if( self.radiobuttons.getcurselection() ==
                    self.radiobuttons.butt_names.User_file
                ):
                    self.vtk_cmap.switchColormap( 
                        self.radiobuttons.butt_names.User_file )
    
            except:
                anag_utils.excepthook()
                response = tkMessageBox.askquestion( 
                    message = "Exception loading colormap file.\nTry again?" )
                if response == 'yes':
                    self._getNewCmap()
                else:
                    # This is if we got here because user clicked "User"
                    # radiobutton but then failed to load a valid colormap.
                    if( self.radiobuttons.getcurselection() ==
                        self.radiobuttons.butt_names.User_file
                    ):
                        self.radiobuttons.invoke(
                            self.radiobuttons.butt_names.Default_color )
    
        decls.memberFunctionAudit(self)
    

    def _colormapMinHandler( self, colormap_val ): 
        """ Handler for the "Colormap minimum" Entry
            Arg colormap_val is the float in the Entry widget.
        """
        anag_utils.funcTrace()

        constrained_val = min( colormap_val, self.vtk_data.getCmappedRangeMax())

        # It's extra-important to avoid unnecessary updates here, because we're
        # setting a notifier in vtk_data and all pipelines update when that
        # happens.  On the other hand, small distinctions do matter when people
        # want to draw some cells black.
        tol1 = ( self.vtk_data.getCmappedRangeMax() -
                 self.vtk_data.getCmappedRangeMin() )/1E10
        tol2 = algorithms.findResolution(self.vtk_data.getCmappedRangeMax())/10.
        tol3 = algorithms.findResolution(self.vtk_data.getCmappedRangeMin())/10.
        if algorithms.floatEquals( constrained_val,
                                   self.vtk_data.getCmappedRangeMin(),
                                   min(tol1,tol2,tol3) ):
            return

        self.vtk_data.setCmappedRangeMin( constrained_val )

        # CmappedRangeMin is a Notifier.

        self.colormap_max.configure( scale_soft_min = constrained_val )
        self.vtk_vtk.render()
    
    
    def _colormapMaxHandler( self, colormap_val ):
        """ Handler for the "Colormap maximum" Entry
            Arg colormap_val is the float in the Entry widget.
        """
        anag_utils.funcTrace()

        constrained_val = max( colormap_val,
                               self.vtk_data.getCmappedRangeMin() )


        tol1 = ( self.vtk_data.getCmappedRangeMax() -
                 self.vtk_data.getCmappedRangeMin() )/1E10
        tol2 = algorithms.findResolution(self.vtk_data.getCmappedRangeMax())/10.
        tol3 = algorithms.findResolution(self.vtk_data.getCmappedRangeMin())/10.
        if algorithms.floatEquals( constrained_val,
                                   self.vtk_data.getCmappedRangeMax(),
                                   min(tol1,tol2,tol3) ):
            return


        self.vtk_data.setCmappedRangeMax( constrained_val )
        # CmappedRangeMax is a Notifier.

        self.colormap_min.configure( scale_soft_max = constrained_val )
        self.vtk_vtk.render()


    def unitTest( self ):
        """
        OK to override this.  But self.show() is a sensible minimal test
        for a GUI control panel.
        If this function doesn't get executed (and instead you get the message
        "unitTest():You must redefine this function in every class", then
        just make sure anag_megawidgets.SelfDescribingDialog comes before
        SelfControl in the list of base classes.
        """
        self.show()


class CmapEditor( SelfControl ):
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        SelfControl.__init__( self, dep_dict,
            metadata = [
                {'name':'frame'},
                {'name':'scale_pairs', 'initval':{} }
            ])

        self.frame = Tkinter.Frame( dep_dict['top_frame'] )

        self.scale_pairs['hue'] = self.ScalePair(
            dep_dict = {'saved_states':self.saved_states,
                        'text': "Hue",
                        'getter':self.vtk_cmap_editor.getHueRange,
                        'setter': self.vtk_cmap_editor.setHueRange,
                        'frame': self.frame})
        self.scale_pairs['saturation'] = self.ScalePair(
            dep_dict = {'saved_states':self.saved_states,
                        'text': "Saturation",
                        'getter':self.vtk_cmap_editor.getSaturationRange,
                        'setter': self.vtk_cmap_editor.setSaturationRange,
                        'frame': self.frame})
        self.scale_pairs['value'] = self.ScalePair(
            dep_dict = {'saved_states':self.saved_states,
                        'text': "Value",
                        'getter':self.vtk_cmap_editor.getValueRange,
                        'setter': self.vtk_cmap_editor.setValueRange,
                        'frame': self.frame})
        self.scale_pairs['alpha'] = self.ScalePair(
            dep_dict = {'saved_states':self.saved_states,
                        'text': "Alpha",
                        'getter':self.vtk_cmap_editor.getAlphaRange,
                        'setter': self.vtk_cmap_editor.setAlphaRange,
                        'frame': self.frame})


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        initial_locks = {'hue':1, 'saturation':0, 'value':1, 'alpha':1}
        for k in initial_locks.keys():
            v = initial_locks[k]
            self.scale_pairs[k].setLocked( v )


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self ):
        anag_utils.funcTrace()
        initial_values={'hue'       :self.vtk_cmap_editor.getHueRange(),
                        'saturation':self.vtk_cmap_editor.getSaturationRange(),
                        'value'     :self.vtk_cmap_editor.getValueRange(),
                        'alpha'     :self.vtk_cmap_editor.getAlphaRange()}

        # We could have class ScalePair restore its own value, but it would need
        # to know about self.vtk_cmap_editor.
        for k in initial_values.keys():
            (lo,hi) = initial_values[k]
            self.scale_pairs[k].setHi( hi )
            self.scale_pairs[k].setLo( lo )


    def disableWidgets( self ):
        anag_utils.funcTrace()
        for mode in 'hue', 'saturation', 'value', 'alpha':
            self.scale_pairs[mode].disable()
    def enableWidgets( self ):
        anag_utils.funcTrace()
        for mode in 'hue', 'saturation', 'value', 'alpha':
            self.scale_pairs[mode].enable()


    def pack( self ):
        anag_utils.funcTrace()
        self.frame.pack()


    def forget( self ):
        anag_utils.funcTrace()
        self.frame.forget()


    def getCmap( self ):
        anag_utils.funcTrace()
        return self.vtk_cmap_editor.getLookupTable()


    class ScalePair( SelfControl ):
        def __init__( self, dep_dict ):
            anag_utils.funcTrace()

            SelfControl.__init__( self, dep_dict, metadata = [
                {'name':'lo', 'set':2},
                {'name':'hi', 'set':2},
                {'name':'locked', 'set':2, 'save':5},
                {'name':'scale0'},
                {'name':'scale1'},
                {'name':'locked_button'}
            ] )

            self.setMultiInstanceQualifier( self.text )

            subframe = Tkinter.Frame( self.frame )
            subframe.pack(side='left')
            Tkinter.Label( subframe, text=self.text ).pack()
            
            self.scale0 = Tkinter.Scale( subframe, from_=0.0, to=1.0,
                                         resolution=0.01 )
            self.scale0.pack()
            self.scale1 = Tkinter.Scale( subframe, from_=0.0, to=1.0,
                                          resolution=0.01 )
            self.scale1.pack()
            
            self.locked_button = anag_megawidgets.Checkbutton(
                subframe, text='lock' )
            self.locked_button.configure( command =
                lambda self=self, butt=self.locked_button:
                    self.setLocked( butt.get() ) )

            self.locked_button.pack()
            
            self.scale0.configure( command = lambda x, self=self :
                self.paramRangeHandler( x, min_or_max=0 ))
            self.scale1.configure(
                command = lambda x, self=self :
                self.paramRangeHandler( x, min_or_max=1 ))
    

        def _initForFirstHDF5( self ):
            anag_utils.funcTrace()


        def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
            anag_utils.funcTrace()
            self._refresh()


        def _refresh( self ):
            anag_utils.funcTrace()

            # The GUI layer should have very few persistent variables; GUI
            # classes should in almost all cases update themselves from their
            # corresponding vtk classes.  But here we have one of those rare
            # exceptions: a GUI feature that doesn't make any sense below the
            # GUI layer.
            self.setLocked( self.locked )
            if self.locked_button.get() != self.locked:
                self.locked_button.toggle()


        def disable( self ):
            anag_utils.funcTrace()
            self.scale0.configure( state='disabled' )
            self.scale1.configure( state='disabled' )
        def enable( self ):
            anag_utils.funcTrace()
            self.scale0.configure( state='normal' )
            self.scale1.configure( state='normal' )


        def setLo( self, x ):
            anag_utils.funcTrace()
            self.scale0.set( x )
            self.lo = x

        def setHi( self, x ):
            anag_utils.funcTrace()
            self.scale1.set( x )
            self.hi = x

        def setLocked( self, b ):
            anag_utils.funcTrace()
            assert( b==0  or  b==1 )
            self.locked = b


        def paramRangeHandler( self, x, min_or_max ):
            anag_utils.funcTrace()
            cur_range = self.getter()
            if( (self.locked == 1)
            and (abs(  self.scale1.get()
                     - self.scale0.get() ) > 0.001) ):
                if min_or_max == 0:
                    self.scale1.set( x )
                else:
                    self.scale0.set( x )
        
            if min_or_max == 0: # min
                self.setter( (float(x), cur_range[1]) )
            else:
                self.setter( (cur_range[0], float(x)) )
