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

# File: control_stream.py
# Author: TDSternberg
# Created: 5/28/01

""" 
Stream is short for "streamlines", which are the paths along which a particle
would get blown about in the vector field.  The "seeds" are the points at which
you begin computing the streamlines.
"""

import Tkinter
import Pmw

import anag_utils
import algorithms
import anag_megawidgets
from self_control import SelfControl
import vtk_stream  # For g_algo_butt_tags


class ControlStream( anag_megawidgets.SelfDescribingDialog, SelfControl ):

    def getShortDescription(self):
        return "Streamlines"
    def getLongDescription(self):
        return "Paths of massless particles"


    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict, metadata=
          [
            {'name':'algo_butts'}, # Radioselect
            {'name':'field_threshold'}, # EntryField
            {'name':'relative_stepsize'}, # EntryField
            {'name':'max_points_per_line'}, # EntryField
            {'name':'integrate_forward'}, # Checkbutton
            {'name':'integrate_backward'}, # Checkbutton
            {'name':'vector_components'}, # component listboxes
            {'name':'max_level'}, # Scale
            {'name':'line_width'}, # Scale
            {'name':'seed_size'}, # Scale
            {'name':'transforms_dialog', 'initval':None},
            {'name':'colorwheel'},
            {'name':'show_dialog', 'save':6, 'initval':0},
            {'name':'tubes_activator'},
            {'name':'local_vtk_data'}
          ] )
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'self', 'dep_dict', 'master',
            'kw' )
        self.configure( title="Streamlines" )
        self.dialog_inventory.registerDialog( self )

    
    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self',
            'k', 'w', 'axis', 'max_level_frame', 'tag_map', 'tag',
            'integration_directions_frame', 'direction_names', 'num_directions',
            'angle', 'scale_from', 'scale_to', 'newSeedPointsHandler',
            'component_names', 'line_width_frame', 'newseed_and_tubes_frame' )

        self.local_vtk_data = self.vtk_stream.getLocalVtkData()

        #
        # Algorithm
        #
        Tkinter.Label( self.interior(), text='Algorithm:' ).pack(anchor='w')
        self.algo_butts = anag_megawidgets.RadioSelect(
            self.interior(),
            buttontype = 'radiobutton',
            orient = 'vertical'
        )
        self.algo_butts.configure(
            command = lambda button_tag, self=self:
                          self.vtk_stream.setAlgorithm( button_tag,do_update=1))
        for k in vtk_stream.g_algo_tag_map.keys():
            map( self.algo_butts.add, (k,) )

        
        self.algo_butts.pack()

        anag_megawidgets.HorizRule( self.interior(), 298 ).pack()

        #
        # Field threshold, relative stepsize, and max points.
        #
        self.field_threshold = Pmw.EntryField(
            self.interior(),
            labelpos = 'w', label_text='Field threshold:',
            validate = {'validator':'real', 'min':0} )
        self.field_threshold.component('entry').bind(
            '<Return>', lambda x, self=self:
                self.vtk_stream.setFieldThreshold(
                    float(self.field_threshold.get()), do_update=1 ))
        self.field_threshold.component('entry').configure(width=8)
                        
        self.relative_stepsize = Pmw.EntryField(
            self.interior(),
            labelpos = 'w', label_text='Relative stepsize:',
            validate = {'validator':'real', 'min':0, 'max':1} )
        self.relative_stepsize.component('entry').bind(
            '<Return>', lambda x, self=self:
                self.vtk_stream.setRelativeStepsize(
                    float(self.relative_stepsize.get()), do_update=1 ))
        self.relative_stepsize.component('entry').configure(width=8)
                        
        self.max_points_per_line = Pmw.EntryField(
            self.interior(),
            labelpos = 'w', label_text='Max points:',
            validate = {'validator':'integer', 'min':1} )
        self.max_points_per_line.component('entry').bind(
            '<Return>', lambda n, self=self:
                self.vtk_stream.setMaxPointsPerLine(
                    int(self.max_points_per_line.get()), do_update=1 ))
        self.max_points_per_line.component('entry').configure(width=8)

        for w in ( self.field_threshold, self.relative_stepsize,
                   self.max_points_per_line ):
            w.pack(anchor='e')

        anag_megawidgets.HorizRule( self.interior(), 298 ).pack()
                        
        #
        # Integration directions
        #
        Tkinter.Label( self.interior(), text='Integration directions:' ).pack(
            anchor='w')
        integration_directions_frame = Tkinter.Frame( self.interior() )
        self.integrate_forward = anag_megawidgets.Checkbutton(
            integration_directions_frame,
            text = 'Forward')
        self.integrate_backward = anag_megawidgets.Checkbutton(
            integration_directions_frame,
            text = 'Backward')
        self.integrate_forward.configure(
            command = lambda self=self, butt=self.integrate_forward:
                self.setIntegrateForward( butt.get() ))
        self.integrate_backward.configure(
            command = lambda self=self, butt=self.integrate_backward:
                self.setIntegrateBackward( butt.get() ))
        for w in self.integrate_forward, self.integrate_backward:
            w.pack( side='left' )
        integration_directions_frame.pack()

        anag_megawidgets.HorizRule( self.interior(), 298 ).pack()

        #
        # Components: map components to (u,v,w).
        #
        if( not self.local_vtk_data.is2DMode()
        or self.local_vtk_data.isResliceMode() ):
            num_directions = 3
            direction_names = ('U','V','W')
        else:
            num_directions = 2
            direction_names = ('U','V')        
        component_names = self.local_vtk_data.getComponentList()
        self.vector_components = anag_megawidgets.ComponentSelectionLists(
            master_frame=self.interior(),
            orientation='vertical',
            main_label_text='Vector components',
            num_lists = num_directions,
            list_label_texts = direction_names,
            list_items = component_names,
            max_visible_list_items = min(5, len(component_names)),
            callback = self._componentSelectionHandler )
        self.vector_components.pack()

        anag_megawidgets.HorizRule( self.interior(), 298 ).pack()

        #
        # Max Level scale.
        #
        max_level_frame = Tkinter.Frame( self.interior() )
        self.max_level = anag_megawidgets.EntryScale(
            max_level_frame,
            button_text = 'Max level:',
            discrete=1,
            dep_dict = {'saved_states':self.saved_states},
            scale_callback = lambda n, self=self: self.vtk_stream.setMaxLevel(
                int(n), do_update=1))
        self.max_level.configure(
            from_=0, to=self.local_vtk_data.getMaxAvailableLevel(),
            resolution = 1 )
        self.max_level.pack( side='left' )
        max_level_frame.pack()
        anag_megawidgets.HorizRule( self.interior(), 298 ).pack()

        #
        # Line width scale.
        #
        line_width_frame = Tkinter.Frame( self.interior() )

        self.line_width = anag_megawidgets.EntryScale(
            line_width_frame,
            button_text = 'Line width:',
            no_validation=1,
            discrete=1,
            dep_dict = {'saved_states':self.saved_states},
            scale_callback = lambda n, self=self: self.vtk_stream.setLineWidth(
                int(n) ))
        self.line_width.configure(
            from_=1, to=10,
            resolution = 1 )
        self.line_width.pack( side='left' )

        self.seed_size = anag_megawidgets.EntryScale(
            line_width_frame,
            button_text = 'Seed size:',
            no_validation=1,
            dep_dict = {'saved_states':self.saved_states},
            scale_callback = lambda n, self=self: self.vtk_stream.setSeedSize(
                float(n) ))
        self.seed_size.configure(
            from_=1, to=20,
            resolution = 1.0 )
        self.seed_size.pack( side='left' )

        line_width_frame.pack()
        anag_megawidgets.HorizRule( self.interior(), 298 ).pack()

        newseed_and_tubes_frame = Tkinter.Frame( self.interior() )
        newseed_and_tubes_frame.pack(anchor='w')

        #
        # Seed points
        #
        Tkinter.Button( newseed_and_tubes_frame, text='New seed points',
            command = self.newTransformDialog ).pack(side='left',
                                                     anchor=Tkinter.W)
        self.transforms_dialog = _SeedPointTransformations(
            dep_dict =
                {'saved_states':self.saved_states,
                 'local_vtk_data':self.local_vtk_data,
                 'vtk_stream':self.vtk_stream,
                 'dialog_inventory':self.dialog_inventory
                })
        anag_megawidgets.HorizRule( self.interior(), 298 ).pack()

        #
        # Tubes activator
        #
        self.tubes_activator = anag_megawidgets.Checkbutton(
            newseed_and_tubes_frame, text='tubular',
            borderwidth=2, relief='groove' )
        self.tubes_activator.configure( command = lambda self=self,
            butt=self.tubes_activator:
                self.vtk_stream.setTubesActive(butt.get()) )
        self.tubes_activator.pack(anchor=Tkinter.E, padx=20)

        #
        # Color
        #
        self.colorwheel = anag_megawidgets.ColorWheel(
            master = self.interior(),
            colorwheel_image = self.local_vtk_data.getChomboVisDataDir() +\
                "/ColorWheel.ppm",
            label_text = "Color",
            show_rgb_entries = 1,
            rgb_initial_vals = {'r':1, 'g':1, 'b':1},
            command = lambda rgb, self=self :
                self.vtk_stream.setRgb( rgb, do_update=1 ),
            relief = Tkinter.GROOVE, borderwidth = 2
            )
        self.colorwheel.pack( padx=2, pady=2, anchor=Tkinter.N )

        decls.memberFunctionAudit( self )


    def _refresh( self ):
        anag_utils.funcTrace()
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        if self.local_vtk_data.getNumComponents() == 0:
            return                                        # Early return

        # The dialog itself -- visible or hidden.
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        self.algo_butts.invoke( self.vtk_stream.getAlgorithm() )

        self.field_threshold.setentry( self.vtk_stream.getFieldThreshold() )
        self.relative_stepsize.setentry(
            self.vtk_stream.getRelativeStepsize() )
        self.max_points_per_line.setentry(
            self.vtk_stream.getMaxPointsPerLine() )

        self.integrate_forward.set( self.vtk_stream.getIntegrateForward() )
        self.integrate_backward.set( self.vtk_stream.getIntegrateBackward() )
        self.tubes_activator.set( self.vtk_stream.getTubesActive() )

        if not self.local_vtk_data.isResliceMode():
            if self.local_vtk_data.is2DMode():
                axes = 'u', 'v'
            else:
                axes = 'u', 'v', 'w'
            selections = []
            for axis in axes:
                axis_num = {'u':0,'v':1,'w':2}[axis]
                selections.insert(
                    axis_num, self.vtk_stream.getComponentMap()[ axis ] )
            self.vector_components.setSelections( selections )

        self.max_level.setScaleSterile( self.vtk_stream.getMaxLevel() )
        self.line_width.setScaleSterile( self.vtk_stream.getLineWidth() )
        self.seed_size.setScaleSterile( self.vtk_stream.getSeedSize() )
        self.colorwheel.setRgbEntries( self.vtk_stream.getRgb() )

        self.registerCallback(
            self.local_vtk_data.getNotifierNewComponentNotifier(),
            self._newComponentNotifierHandler )


    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()


    def _newComponentNotifierHandler( self, compname, unused ):
        anag_utils.funcTrace()
        self.vector_components.appendNewComponentName( compname )

    def cleanup( self ):
        anag_utils.funcTrace()
        self.transforms_dialog.withdrawGUI()
        self.transforms_dialog.cleanup()
        self.dialog_inventory.unregisterDialog( self )
        self.withdrawGUI()


    def _componentSelectionHandler( self, component_selection, ndx ):
        """ Handler for selection events in the component-choice menu """
        anag_utils.funcTrace()
        self.vtk_stream.setComponentMap(
            component_selection, {0:'u',1:'v',2:'w'}[ndx], do_update=1 )


    def setIntegrateForward( self, on_off ):
        """
        Toggle integration forward from seed points.
        Don't let user turn forward off, if backward is already off; that
        gets us a segfault.  (Besides, if they don't want anything, they have
        another way to turn off the streamlines.)
        """
        anag_utils.funcTrace()
        if on_off != 0  or  self.vtk_stream.getIntegrateBackward() != 0:
            self.vtk_stream.setIntegrateForward( on_off, do_update=1 )
        else:
            anag_utils.warning( "Don't turn off both forward and backward." )
            self.integrate_forward.set( 1 )


    def setIntegrateBackward( self, on_off ):
        """
        Toggle integration backward from seed points.
        Don't let user turn backward off, if forward is already off; that
        gets us a segfault.  (Besides, if they don't want anything, they have
        another way to turn off the streamlines.)
        """
        anag_utils.funcTrace()
        if on_off != 0  or  self.vtk_stream.getIntegrateForward() != 0:
            self.vtk_stream.setIntegrateBackward( on_off, do_update=1 )
        else:
            anag_utils.warning( "Don't turn off both forward and backward." )
            self.integrate_backward.set( 1 )


    def newTransformDialog( self ):
        anag_utils.funcTrace()

        # Set an algorithm that shows something:
        if(   self.vtk_stream.getAlgorithm()
           == vtk_stream.g_algo_butt_tags.Nothing ):
            self.algo_butts.invoke( vtk_stream.g_algo_butt_tags.Fixed_Step )

        self.transforms_dialog.showGUI()


class _SeedPointTransformations( SelfControl,
                                 anag_megawidgets.SelfDescribingDialog ):
    """
    Pops up when user presses "New seed points" button.
    Lets user translate and rotate the points.
    """
    def getShortDescription(self):
        return 'transformations'
    def getLongDescription(self):
        return 'translate and rotate the seed points'

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        self.configure( title="Seed point transformations" )
        SelfControl.__init__( self, dep_dict, metadata=
          [{'name':'seeds'}, # Instance of vtk_stream._Seeds
           {'name':'show_dialog', 'save':6, 'initval':0},
           {'name':'translation_scale', 'initval':{}},
           {'name':'rotation_scale', 'initval':{}},
           {'name':'n_seeds_scale'},
           {'name':'length_scale'}
          ])

        # Grab a _Seed instance from vtk_stream.  For now, we support just one
        # here in the GUI.  But vtk_stream is set up for an unlimited number.
        self.seeds = self.vtk_stream.getSeedObject()
        assert( self.seeds )

        # Translation controls.
        Tkinter.Label( self.interior(), text='Translate:' ).pack(anchor='w')
        for axis in 'x','y','z':
            if axis=='z' and self.local_vtk_data.is2DMode():
                break
            scale_from = 0
            scale_to = ( self.local_vtk_data.getDomainMax()[axis] -
                         self.local_vtk_data.getDomainMin()[axis] )
            self.translation_scale[axis] = anag_megawidgets.EntryScale(
                self.interior(),
                button_text=axis,
                scale_callback = lambda value, self=self, axis=axis:
                    self.translate( float(value), axis ),
                from_ = scale_from,
                to = scale_to,
                dep_dict = {'saved_states':self.saved_states },
                scale_normal_resolution = algorithms.roundDown10(
                                            (scale_to - scale_from)/50.0 )
            )

            self.translation_scale[axis].pack()
        

        # Rotation controls.
        Tkinter.Label( self.interior(), text='Rotate:' ).pack(anchor='w')
        for angle in 'phi', 'rho':
            if angle=='rho' and self.local_vtk_data.is2DMode():
                break
            self.rotation_scale[angle] = anag_megawidgets.EntryScale(
                self.interior(),
                button_text=angle,
                scale_callback = lambda value, self=self, angle=angle:
                    self.rotate( float(value), angle ),
                from_=-90.0,
                to=90.0,
                scale_normal_resolution=1.0,
                dep_dict = {'saved_states':self.saved_states }
              )
            self.rotation_scale[angle].pack()

        # Population (number of seeds) control.
        self.n_seeds_scale = anag_megawidgets.EntryScale(
            self.interior(),
            discrete=1,
            button_text='Number of seeds:',
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1,
            scale_callback = lambda value, self=self: self.populate( int(value))
            )
        self.n_seeds_scale.configure( from_=2, to=100, resolution=1 )
        self.n_seeds_scale.pack()


        # Rake length control.
        self.length_scale = anag_megawidgets.EntryScale(
            self.interior(),
            button_text='Length:',
            dep_dict = {'saved_states':self.saved_states },
            from_=0.0, to=1.0,
            scale_normal_resolution=0.01,
            no_validation=1,
            scale_callback = lambda value, self=self:
                self.stretch( float(value) )
            )
        self.length_scale.pack()


        self.dialog_inventory.registerDialog( self )


    def _initForFirstHDF5( self ):
        pass


    def _refresh( self ):
        # The dialog itself -- visible or hidden.
        anag_utils.funcTrace()

        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )

        # Restore EntryScale positions.  Gotta call populate(), translate(),
        # rotate() and stretch() too, because when this object was constructed,
        # all the scales were initialized to their defaults.  So we have to
        # re-restore them.
        transforms_state = self.seeds.getTransformsState()

        self.n_seeds_scale.setScaleSterile( transforms_state['n_seeds'] )
        self.populate( transforms_state['n_seeds'] )

        self.length_scale.setScaleSterile( transforms_state['length'] )
        self.stretch( transforms_state['length'] )
        
        if self.local_vtk_data.is2DMode():
            axes = 'x','y'
            angles = ('phi',)
        else:
            axes = 'x','y','z'
            angles = 'phi', 'rho'
        for axis in axes:
            self.translation_scale[axis].setScaleSterile(transforms_state[axis])
            self.translate( transforms_state[axis], axis )
        for angle in angles:
            self.rotation_scale[angle].setScaleSterile(transforms_state[angle] )
            self.rotate( transforms_state[angle], angle )

        self._anisotropicFactorsNotifier(None,None)

        self.registerCallback(
            self.local_vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicFactorsNotifier )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.dialog_inventory.unregisterDialog( self )
        self.zeroSelfVariables()


    def _anisotropicFactorsNotifier( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        extents = self.local_vtk_data.getDomainExtentsXYZ()
        for axis in 'x', 'y', 'z':
            if axis=='z' and self.local_vtk_data.is2DMode():
                continue
            axis_num = (axis=='y') + 2 * (axis=='z')
            scale_from = 0
            scale_to = extents[axis_num + 3] - extents[axis_num]
            self.translation_scale[axis].configure(
                from_=scale_from, to=scale_to )


    def populate( self, value ):
        """
        Set the number of seed points.
        """
        anag_utils.funcTrace()
        if value < 2:
            anag_utils.error( "2 is the minimum legal number of streamlines." )
            return
        self.seeds.populate( n_seeds = value )
        self.vtk_stream.updateVis()


    def stretch( self, value ):
        """
        Set the length of the rake, as a fraction of the domain's longest
        diagonal.
        """
        anag_utils.funcTrace()
        self.seeds.stretch( length = value )
        self.vtk_stream.updateVis()            


    def translate( self, value, axis ):
        """
        Translate the seeds by arg value along arg axis, and rerender.
        Callback for the three (x,y,z) translation EntryScales.
        """
        anag_utils.funcTrace()
        assert( axis=='x' or axis=='y' or axis=='z' )

        if   axis == 'x':
            self.seeds.translate( x = value )
        elif axis == 'y':
            self.seeds.translate( y = value )
        elif axis == 'z':
            self.seeds.translate( z = value )

        self.vtk_stream.updateVis()


    def rotate( self, value, angle ):
        """
        Rotate the seeds by arg value along arg angle, and rerender.
        Callback for the two (rho, phi) rotation EntryScales.
        """
        anag_utils.funcTrace()
        assert( angle=='rho' or angle=='phi' )

        if   angle == 'rho':
            self.seeds.rotate( rho = value )
        elif angle == 'phi':
            self.seeds.rotate( phi = value )

        self.vtk_stream.updateVis()

