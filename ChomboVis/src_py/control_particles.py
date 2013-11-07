#
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

# File: control_particles.py
# Author: TDSternberg
# Created: 5/28/01

""" 
GUI for particle visualization.
Calls into vtk_particles.py for most VTK operations.
"""

import math
import Tkinter
import Pmw
import tkMessageBox

import algorithms
import anag_utils
import anag_megawidgets
from self_control import SelfControl
import vtk_particles # For g_* variables

class ControlParticles( anag_megawidgets.SelfDescribingDialog, SelfControl ):

    def getShortDescription(self):
        return "Particles"
    def getLongDescription(self):
        return "Particles"


    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict, metadata=
          [
            {'name':'marker_type_radiobuttons'},
            {'name':'marker_size_scale'},
            {'name':'positional_components'}, # component menus
            {'name':'opacity_scale'},
            {'name':'orientation_components'}, # component menus
            {'name':'show_dialog', 'save':2, 'initval':0},
            {'name':'local_vtk_data'},
            {'name':'color_wheel'},
            {'name':'filter_min', 'initval':{} },           # entryscales
            {'name':'filter_max', 'initval':{} },           # entryscales
            {'name':'filtering_component', 'initval':{}},   # listboxes
            {'name':'offset_filter_offset'},        # entryscale
            {'name':'glyph_scale_mode_radiobuttons'},
            {'name':'glyph_scaling_component'},  # listbox
            {'name':'already_warned_about_too_many_glyphs'},
#           {'name':'color_legend'},             # checkbutton
            {'name':'log_decimation_factor'}     # entryscale
          ] )
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'dep_dict', 'master',
            'kw' )
        self.configure( title="Particles" )
        self.dialog_inventory.registerDialog( self )

    
    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        if self.vtk_data.getNumParticles() == 0:
            return
        decls = anag_utils.Declarations( 'decls', 'self',
            'filter_frame',
            'component_names', 'axis', 'glyph_scaling_frame', 'butt_tags',
            'filter_range', 'domain_size_l_inf', 'visibility_frame',
            'filter_range', 'button_frame', 'filter_type',
            'butt_tags', 'default_scaling_component', 'color_frame',
            'num_orientation_components', 'orientation_component_names',
            'num_positional_components', 'positional_component_names',
            'minmax_frame', 'pos_comp_frame', 'orient_comp_frame' )

        self.local_vtk_data = self.vtk_particles.getLocalVtkData()

        visibility_frame = Tkinter.Frame( self.interior() )
        visibility_frame.pack()


        #
        # Radiobuttons: 'glyphs' vs 'points' vs 'nothing'.
        #
        self.marker_type_radiobuttons = anag_megawidgets.RadioSelect(
            visibility_frame,
            buttontype = 'radiobutton',
            orient = 'horizontal',
            labelpos = 'n',
            label_text = 'Particle markers'
            )
        self.marker_type_radiobuttons.configure(
            command = lambda button_tag, self=self:
                   self._setMarkerType( button_tag ))
    
        butt_tags = vtk_particles.g_marker_types
        if self.vtk_data.getNumParticles() > 0:
            map( self.marker_type_radiobuttons.add,
                 (butt_tags.nothing, butt_tags.points,
                  butt_tags.circles,
                  butt_tags.spheres, butt_tags.cones ) )
        if self.vtk_data.getNumParticles() == 1:
            # Implementation artifact -- see vtkChomboParticlesImpl_cxx
            self.marker_type_radiobuttons.button(
                butt_tags.points).configure(state='disabled')

        self.marker_type_radiobuttons.pack(anchor='w')
        self.marker_type_radiobuttons.setcurselection('nothing')


        # 
        # Marker size (used as a constant multiple, when we're scaling by a
        # scalar).
        #
        self.marker_size_scale = anag_megawidgets.EntryScale( visibility_frame,
            button_text='Marker size factor',
            dep_dict={'saved_states':self.saved_states}, no_validation=1,
            from_=self.vtk_particles.getReasonableMarkerSizes()['min'],
            to=self.vtk_particles.getReasonableMarkerSizes()['max'],
            resolution=self.vtk_particles.getReasonableMarkerSizes()['min'],
            scale_callback=self.vtk_particles.setMarkerSize,
            scale_normal_resolution=\
                algorithms.roundDown10(
                    self.vtk_particles.getReasonableMarkerSizes()['min'] ) )
        self.marker_size_scale.setScaleValue(
            self.vtk_particles.getReasonableMarkerSizes()['init'] )
        self.marker_size_scale.pack( anchor='w')

        #
        # Opacity (0 to 1 inclusive)
        #
        self.opacity_scale = anag_megawidgets.EntryScale(
            visibility_frame,
            button_text = 'Opacity',
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1,
            from_=0.0, to=1.0, scale_normal_resolution=0.1,
            scale_callback = lambda x, self=self:
                self.vtk_particles.setOpacity( x )
            )
        self.opacity_scale.pack(anchor='w')


        #
        # Marker scaling/coloring.
        #
        
        # Component
        glyph_scaling_frame = Tkinter.Frame(self.interior(),
                                relief='groove', borderwidth=2 )
        component_names = self.vtk_particles.getComponentNames()
        self.glyph_scaling_component = anag_megawidgets.ComponentSelectionLists(
            master_frame = glyph_scaling_frame,
            orientation='horizontal',
            main_label_text = 'Marker scaling/coloring component',
            num_lists=1,
            list_items = component_names,
            max_visible_list_items=6,
            callback = self._glyphScalingComponentHandler )
        self.glyph_scaling_component.pack()
        self.glyph_scaling_component.setSelections((
            self.vtk_particles.getGlyphScalingComponent(), ))


        # Colorwheel
        color_frame = Tkinter.Frame( glyph_scaling_frame )
        color_frame.pack()
        self.color_wheel = anag_megawidgets.ColorWheel(
            master = color_frame,
            colorwheel_image = self.local_vtk_data.getChomboVisDataDir() +\
                "/ColorWheel.ppm",
            label_text = "Constant color:",
            show_rgb_entries = 1,
            rgb_initial_vals =
                {'r':self.vtk_particles.getMarkerConstantColor()[0],
                 'g':self.vtk_particles.getMarkerConstantColor()[1],
                 'b':self.vtk_particles.getMarkerConstantColor()[2] },
            command = lambda rgb_, self=self:
                self.vtk_particles.setMarkerConstantColor(rgb_),
            relief = Tkinter.GROOVE, borderwidth = 2
            )
        self.color_wheel.pack( side='left', anchor=Tkinter.NW, pady=20 )

        button_frame = Tkinter.Frame( color_frame )
        button_frame.pack( side='left' )

        # Coloring/scaling mode
        self.glyph_scale_mode_radiobuttons = anag_megawidgets.RadioSelect(
            button_frame, buttontype = 'radiobutton',
            orient = 'vertical' )
        self.glyph_scale_mode_radiobuttons.configure(
            command = lambda butt, self=self:
                self.vtk_particles.setGlyphScaleMode( butt ))
        butt_tags = vtk_particles.g_glyph_scale_modes
        map( self.glyph_scale_mode_radiobuttons.add,
             (butt_tags.none, butt_tags.colorOnly, butt_tags.colorAndScale))
        self.glyph_scale_mode_radiobuttons.pack(side='top')

        """
        # Legend
        self.color_legend = anag_megawidgets.Checkbutton(
            button_frame, text='Show color legend' )
        self.color_legend.configure( command = lambda butt = self.color_legend,
            self=self: self._showLegend( butt.get() ) )
        self.color_legend.pack(anchor='w')
        """

        glyph_scaling_frame.pack()

        #
        # Filtering components (ordinary with decimation factor, and offset
        # with offset amount) and their minimums and maximums.
        #
        filter_frame = {}
        minmax_frame = {}
        for filter_type in vtk_particles.g_filter_types:
            filter_frame[filter_type] = Tkinter.Frame( self.interior(),
                                             relief='groove', borderwidth=2 )
            filter_frame[filter_type].pack()

            Tkinter.Label( filter_frame[filter_type], text=
                filter_type + ' filtering component' ).pack()

            self.filtering_component[filter_type] =\
              anag_megawidgets.ComponentSelectionLists(
                master_frame = filter_frame[filter_type],
                orientation='horizontal',
                main_label_text = '',
                num_lists=1,
                list_items = component_names,
                max_visible_list_items = 6,
                callback = lambda selection, dummy, self=self, t=filter_type:
                    self._filteringComponentHandler(t, selection, dummy),
                is_expanded=0 )
            self.filtering_component[filter_type].setSelections((
                    self.vtk_particles.getFilteringComponent([filter_type,]),))
            self.filtering_component[filter_type].pack(side='left', anchor='nw')
    
            minmax_frame[filter_type] = Tkinter.Frame(filter_frame[filter_type])
            minmax_frame[filter_type].pack( side='left' )
            self.filter_min[filter_type] = anag_megawidgets.EntryScale(
                minmax_frame[filter_type],
                button_text = 'Min',
                dep_dict = {'saved_states':self.saved_states },
                no_validation=1
                )
            filter_range = self.vtk_particles.getFilterRange([filter_type,])[
                self.vtk_particles.getFilteringComponent([filter_type,]) ]
            self.filter_min[filter_type].configure(
                from_= filter_range[0], to=filter_range[1],
                scale_normal_resolution = algorithms.roundDown10(
                    (filter_range[1]-filter_range[0])/1000.0),
                scale_callback = lambda x, self=self, t=filter_type:
                    self.vtk_particles.setFilter( t, lo = float(x) ))
            self.filter_min[filter_type].pack( padx=2, anchor=Tkinter.W )
            self.filter_min[filter_type].flip()
        
            self.filter_max[filter_type] = anag_megawidgets.EntryScale(
                minmax_frame[filter_type],
                button_text = 'Max',
                dep_dict = {'saved_states':self.saved_states },
                no_validation=1
                )
            self.filter_max[filter_type].configure(
                from_= filter_range[0], to=filter_range[1],
                scale_normal_resolution = algorithms.roundDown10(
                    (filter_range[1]-filter_range[0])/1000.0),
                scale_callback = lambda x, self=self, t=filter_type:
                    self.vtk_particles.setFilter( t, hi = float(x) ))
            self.filter_max[filter_type].flip()
            self.filter_max[filter_type].pack( padx=2, anchor=Tkinter.W )
    
       
            if filter_type == vtk_particles.g_filters.ordinary:
                self.log_decimation_factor = anag_megawidgets.EntryScale(
                    minmax_frame[filter_type],
                    button_text = 'log decimation factor',
                    dep_dict = {'saved_states':self.saved_states },
                    no_validation=1
                    )
                self.log_decimation_factor.configure(
                    from_ = 0, to=10, scale_normal_resolution=0.1,
                    scale_callback = lambda x, self=self:
                        self.vtk_particles.setDecimationFactor(
                            pow(10,float(x))))
                self.log_decimation_factor.pack( padx=2, anchor=Tkinter.W )
            else:
                self.offset_filter_offset = anag_megawidgets.EntryScale(
                    minmax_frame[filter_type],
                    button_text = 'offset',
                    dep_dict = {'saved_states':self.saved_states },
                    no_validation=1
                    )
                self.offset_filter_offset.configure(
                    scale_callback = lambda x, self=self:
                        self.vtk_particles.setOffsetFilterOffset(float(x)))
                self.offset_filter_offset.pack( padx=2, anchor=Tkinter.W )
    
        #
        # Positional components: map components to (x,y,z).
        #
        if( not self.local_vtk_data.is2DMode()
        or self.local_vtk_data.isResliceMode() ):
            num_positional_components = 3
            positional_component_names = ('X','Y','Z')
        else:
            num_positional_components = 2
            positional_component_names = ('X','Y')
        pos_comp_frame = Tkinter.Frame( self.interior(),
                                        relief='groove', borderwidth=2 )
        pos_comp_frame.pack()

        self.positional_components = anag_megawidgets.ComponentSelectionLists(
            master_frame=pos_comp_frame,
            orientation='horizontal',
            main_label_text='Positional components',
            num_lists=num_positional_components,
            list_label_texts=positional_component_names,
            list_items = component_names,
            max_visible_list_items=6,
            callback = self._positionalComponentSelectionHandler )            
        self.positional_components.pack()

        #
        # Glyph orientation components: map components to (x,y,z).
        #
        if( not self.local_vtk_data.is2DMode()
        or self.local_vtk_data.isResliceMode() ):
            num_orientation_components = 3
            orientation_component_names = ('X','Y','Z')
        else:
            num_orientation_components = 2
            orientation_component_names = ('X','Y')
        orient_comp_frame = Tkinter.Frame( self.interior(),
                                        relief='groove', borderwidth=2 )
        orient_comp_frame.pack()
        self.orientation_components = anag_megawidgets.ComponentSelectionLists(
            master_frame=orient_comp_frame,
            orientation='horizontal',
            main_label_text='Orientational components',
            num_lists=num_orientation_components,
            list_label_texts=orientation_component_names,
            list_items = component_names,
            max_visible_list_items=6,
            callback = self._orientationComponentSelectionHandler )            
        self.orientation_components.pack()

        decls.memberFunctionAudit( self )


    def _refresh( self ):
        anag_utils.funcTrace()
        if self.vtk_data.getNumParticles() == 0:
            return
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        # Point-type radiobuttons.
        self.marker_type_radiobuttons.setcurselection(
            self.vtk_particles.getMarkerType() )

        # Marker (i.e. glyph or point) size factor.
        self.marker_size_scale.setScaleSterile(
            self.vtk_particles.getMarkerSize())

        self.opacity_scale.setScaleSterile( self.vtk_particles.getOpacity() )

        # Coloring mode.
        self.glyph_scale_mode_radiobuttons.setcurselection(
            self.vtk_particles.getGlyphScaleMode() )

        # Coloring/scaling component
        self.glyph_scaling_component.setSelections((
            self.vtk_particles.getGlyphScalingComponent(), ))

        # Legend.
        #self.color_legend.set( self.vtk_particles.getLegendIsVisible() )

        # Filtering components (ordinary and offset).
        for filter_type in vtk_particles.g_filter_types:
            fcomp = self.vtk_particles.getFilteringComponent([filter_type,])
            self.filtering_component[filter_type].setSelections((fcomp,))
            self._filteringComponentHandler( filter_type, fcomp, None )
            self.filter_min[filter_type].setScaleSterile(
                self.vtk_particles.getFilterRange([filter_type,])[fcomp][0] )
            self.filter_max[filter_type].setScaleSterile(
                self.vtk_particles.getFilterRange([filter_type,])[fcomp][1] )
            self.log_decimation_factor.setScaleSterile(
                math.log10( self.vtk_particles.getDecimationFactor() ))
            xcomp = self.vtk_particles.getPositionalComponent('x')
            self._updateOffsetFilterOffsetFromToResolution( xcomp )
            self.offset_filter_offset.setScaleSterile(
                self.vtk_particles.getOffsetFilterOffset() )


        # Positional components.
        if( self.local_vtk_data.is2DMode()
        and not self.local_vtk_data.isResliceMode() ):
            axes = 'x', 'y'
        else:
            axes = 'x', 'y', 'z'
        selections = []
        for axis in axes:
            axis_num = {'x':0,'y':1,'z':2}[axis]
            selections.insert(
                axis_num,
                self.vtk_particles.getPositionalComponent( axis ))
        self.positional_components.setSelections( selections )


        # Glyph orientation components.
        if self.vtk_particles.getGlyphOrientationComponents() != {}:
            if( self.local_vtk_data.is2DMode()
            and not self.local_vtk_data.isResliceMode() ):
                axes = 'x', 'y'
            else:
                axes = 'x', 'y', 'z'
            selections = []
            for axis in axes:
                axis_num = {'x':0,'y':1,'z':2}[axis]
                selections.insert(
                    axis_num,
                    self.vtk_particles.getGlyphOrientationComponents()[axis])
            self.orientation_components.setSelections( selections )



    def cleanup( self ):
        anag_utils.funcTrace()
        self.dialog_inventory.unregisterDialog( self )
        self.withdrawGUI()


    """
    def _showLegend( self, on_off ):
        #Show or hide the color legend.
        anag_utils.funcTrace()
        assert( on_off==1  or  on_off==0 )
        self.vtk_particles.setLegendIsVisible( on_off )
    """

    def _setMarkerType( self, button_tag ):
        anag_utils.funcTrace()
        if( (    button_tag == vtk_particles.g_marker_types.cones
              or button_tag == vtk_particles.g_marker_types.spheres )
        and (self.local_vtk_data.getNumParticles() > 2000)
        and (self.already_warned_about_too_many_glyphs == None) ):
            self.already_warned_about_too_many_glyphs = 1
            response = tkMessageBox.askyesno( message="This could take "
                "a while.  Consider using points instead of glyphs, when you "
                "have more than a few thousand particles.  Or at least use "
                "the filtering controls to reduce the number of displayed "
                "particles.  Use points instead of glyphs?" )
            if response == 1:
                p = vtk_particles.g_marker_types.points
                self.marker_type_radiobuttons.setcurselection( p )
                self._setMarkerType( p )
                return
        self.vtk_particles.setMarkerType( button_tag )


    def _positionalComponentSelectionHandler( self, component_selection, ndx ):
        """ Handler for selection events in the component-choice list. """
        anag_utils.funcTrace()
        self.vtk_particles.setPositionalComponent(
            component_selection, {0:'x',1:'y',2:'z'}[ndx] )

        if ndx == 0:
            self._updateOffsetFilterOffsetFromToResolution(
                component_selection )


    def _updateOffsetFilterOffsetFromToResolution( self, x_pos_comp_name ):
        """
        Our offsetting is always done in the direction of whatever is the
        positional component for 'x'.  So when that changes, we adjust the from,
        to, and scale_normal_resolution properties of our offset_filter_offset
        slider.
        """
        anag_utils.funcTrace()
        span = self.vtk_particles.getComponentRange( x_pos_comp_name )
        biggest_abs = max(abs(span[0]), abs(span[1]))
        self.offset_filter_offset.configure(
            from_ = -biggest_abs,
            to    =  biggest_abs,
            scale_normal_resolution =
                algorithms.roundDown10((2*biggest_abs)/100.0 ))

    def _orientationComponentSelectionHandler( self, component_selection, ndx ):
        """ Handler for selection events in the glyph-orientation list. """
        anag_utils.funcTrace()
        self.vtk_particles.setGlyphOrientationComponent(
            component_selection, {0:'x',1:'y',2:'z'}[ndx] )


    def _glyphScalingComponentHandler( self, component_selection, dummy ):
        """ Handler for selection events in glyph_scaling_component list. """
        anag_utils.funcTrace()
        self.vtk_particles.setGlyphScalingComponent( component_selection )


    def _filteringComponentHandler( self, filter_type, component_selection,
                                    dummy ):
        """
        Handler for selection events in filtering- and offset-filtering-
        component list.
        """
        anag_utils.funcTrace()
        self.vtk_particles.setFilter( filter_type, name = component_selection )
        filter_range = self.vtk_particles.getFilterRange([filter_type,])[
            self.vtk_particles.getFilteringComponent([filter_type,]) ]
        component_range = self.vtk_particles.getComponentRanges(
            [self.vtk_particles.getFilteringComponent([filter_type,]),] )
        resolution = algorithms.roundDown10(
            (component_range[1] - component_range[0])/50.0 )
        self.filter_min[filter_type].configure(
            from_=component_range[0] - resolution,
            to=component_range[1] + resolution,
            resolution=resolution, scale_normal_resolution=resolution )
        self.filter_max[filter_type].configure(
            from_=component_range[0] - resolution,
            to=component_range[1] + resolution,
            resolution=resolution, scale_normal_resolution=resolution )

        self.filter_min[filter_type].setScaleValue( filter_range[0] )
        self.filter_max[filter_type].setScaleValue( filter_range[1] )
