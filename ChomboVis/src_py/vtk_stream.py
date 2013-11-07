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

# File: vtk_stream.py
# Author: TDSternberg
# Created: 6/05/01

import anag_utils
import algorithms
from self_control import SelfControl

import types
import libVTKChomboPython
import vtkpython
import math
import vtk_glyphs

g_decls = anag_utils.Declarations( 'g_decls', 'g_algo_butt_tags',
    'g_algo_tag_map' )

class VtkStream( SelfControl ):

    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        instance_vars = [
            {'name':'stream_prop'},
            {'name':'tubes'},
            {'name':'tubes_active', 'get':1, 'set':2, 'save':5, 'initval':0},
            {'name':'tube_resolution', 'get':1, 'set':2, 'save':5, 'initval':8},
            {'name':'stream_actor'},
            {'name':'stream_source'},
            {'name':'mapper'},
            {'name':'stream_seeds'},  # vtkPoints
            {'name':'seed_object'}, # instance of class _Seed
            {'name':'algorithm', 'get':1, 'set':2, 'initval':'Nothing',
             'save':1 },
            {'name':'field_threshold', 'get':1, 'set':2, 'initval':1E-06,
             'save':1},
            {'name':'relative_stepsize', 'get':1, 'set':2, 'initval':0.25,
             'save':1},
            {'name':'max_points_per_line', 'get':1, 'save':5,
             'initval':1500,'save':1},
            {'name':'integrate_forward', 'get':1, 'set':2, 'initval':1,
             'save':1 },
            {'name':'integrate_backward', 'get':1, 'set':2, 'initval':1,
             'save':1 },
            {'name':'component_map', 'get':1, 'set':2, 'initval':{},'save':1},
                # keys: 'v','w','u'
            {'name':'max_level', 'get':1, 'set':2, 'initval':0, 'save':1},
            {'name':'rgb', 'get':1, 'set':2, 'initval':(1,1,1), 'save':1},
            {'name':'line_width', 'get':1, 'set':2, 'initval':1, 'save':1},
            {'name':'line_width_multiplier'}, # for sensible scaling
            {'name':'seed_size', 'get':1, 'set':2, 'initval':4, 'save':1},
            {'name':'local_vtk_data', 'get':1},
            {'name':'glyphs'},
            {'name':'show_glyphs', 'initval':0, 'set':2, 'save':1}
        ]    
        SelfControl.__init__( self,dep_dict, instance_vars)
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls","self","instance_vars",
            "dep_dict" )

        decls.memberFunctionAudit(self)


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        # In case we're loading a 2nd hdf5:
        self.component_map = {}

        self.local_vtk_data = self.vtk_data.makeLocalVtkData(
            follow_global_levels=0,
            follow_global_curcomponent=0,
            follow_global_padded_boxes=0,
            always_use_real_boxes=0,
            instance_identifier='stream' )

        # FIXME: We crash if we change streamlines' ghost cell policy from the
        # default.  Something inside vtkAMRStreamSourceImpl_cxx gets confused
        # about box extents.  Fix this when you're ready to actually make use
        # of ghost cells in vtkAMRStreamSourceImpl_cxx.

        self.stream_source = libVTKChomboPython.vtkAMRStreamSource()
        self.stream_source.SetChomboReader( self.local_vtk_data.getReader() )

        self.tubes = vtkpython.vtkTubeFilter()

        self.mapper = self.vtk_vtk.newMapper()
        self.stream_actor = self.vtk_vtk.newActor()
        self.stream_prop = self.vtk_vtk.newProperty()
        self.stream_seeds = vtkpython.vtkPoints()
    
        self.stream_prop.SetLineWidth( self.line_width )
        self.stream_prop.SetPointSize( self.seed_size )
        self.stream_prop.SetDiffuse( 0.85 )
        self.stream_prop.SetAmbient( 0.15 )
        self.stream_source.SetSeedPoints( self.stream_seeds )
        self.stream_source.SetIntegrationMode( 1 )
        self.stream_prop.SetColor( 1.0, 1.0, 1.0 )
        self.stream_actor.SetProperty( self.stream_prop )

        self.mapper.SetInput( self.stream_source.GetOutput() )
        self.mapper.ImmediateModeRenderingOn()
        self.stream_actor.SetMapper( self.mapper )

        self.seed_object = self.lineSeedsFactory()

        self.glyphs = vtk_glyphs.VtkGlyphs(
            dep_dict={'vtk_vtk':self.vtk_vtk,
                      'vtk_data':self.vtk_data,
                      'vtk_cmap':self.vtk_cmap,
                      'saved_states':self.saved_states},
            multi_instance_qualifier='stream',
            caller_updater = lambda self=self: self.updateVis(1) )

        #
        # Get a sensible size for tube radius.
        #
        extents = self.local_vtk_data.getDomainExtentsXYZ()
        biggest_side = max( extents[3]-extents[0],
                            extents[4]-extents[1] )
        if not self.local_vtk_data.is2DMode():
            biggest_side = max( biggest_side, extents[5]-extents[2] )
        self.line_width_multiplier = biggest_side/1000.0


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self ):
        anag_utils.funcTrace()

        self.registerCallback(
            self.local_vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if self.local_vtk_data.getNumComponents() == 0:
            return                                        # Early return

        # Sanity-check, in case we're restoring from a file with more levels.
        self.max_level =\
            min( self.local_vtk_data.getMaxAvailableLevel(), self.max_level )

        # In the following, some of the setters won't do anything, but some
        # will have interesting side effects, and let's keep them all in just
        # in case you introduce more side effects.  That'll make maintenance
        # easier.
        if self.local_vtk_data.isResliceMode():
            self.setAlgorithm( g_algo_butt_tags.Nothing )
        else:
            self.setAlgorithm( self.algorithm )
        self.setFieldThreshold( self.field_threshold )
        self.setRelativeStepsize( self.relative_stepsize )
        self.setMaxPointsPerLine( self.max_points_per_line )
        self.setIntegrateForward( self.integrate_forward )
        self.setIntegrateBackward( self.integrate_backward )
        self.setLineWidth( self.line_width )
        self.setSeedSize( self.seed_size )

        if self.component_map == {}:
            self.component_map['u'] = self.local_vtk_data.getComponentList()[0]
            self.component_map['v'] = self.local_vtk_data.getComponentList()[
                1 % len(self.local_vtk_data.getComponentList()) ]
            if not self.local_vtk_data.is2DMode():
                self.component_map['w'] =self.local_vtk_data.getComponentList()[
                    2 % len(self.local_vtk_data.getComponentList()) ]
        for subscript in self.component_map.keys():
            self.setComponentMap( self.component_map[subscript], subscript )

        self.setMaxLevel( self.max_level )
        self.setRgb( self.rgb )
        self.setTubesActive( self.tubes_active )
        self.setTubeResolution(self.tube_resolution)
        self.updateVis()
 
    def cleanup( self ):
        anag_utils.funcTrace()
        self.setAlgorithm( g_algo_butt_tags.Nothing, do_update=1 )
        if self.seed_object:
            self.seed_object.cleanup()


    def getSeedObject( self ):
        anag_utils.funcTrace()
        return self.seed_object # a list


    def addSeed( self, x, y, z, do_update = 0 ):
        """
        Add a seed point to integrate from.
        Args x, y and z are the coordinates of the point.
        Arg do_update governs whether we recalculate the streamlines and
          re-render after adding this seed.  Call self.updateVis() to update
          "manually".

        This function is for programmatic control over the precise placement
        of seeds.  The GUI doesn't use this, going instead through the _Seeds
        class.
        """
        anag_utils.funcTrace()
        self.stream_seeds.InsertNextPoint( x,y,z )
        self.updateVis( do_update )


    def setIntegrateForward( self, on_off, do_update = 0 ):
        """
        Toggle forward integration along the streamline.
        """
        anag_utils.funcTrace()
        assert( on_off == 0 or on_off == 1 )
        self.integrate_forward = on_off
        self.stream_source.SetForward( on_off )
        self.updateVis( do_update )
 

    def setIntegrateBackward( self, on_off, do_update = 0  ):
        """
        Toggle backward integration along the streamline.
        """
        anag_utils.funcTrace()
        assert( on_off == 0 or on_off == 1 )
        self.integrate_backward = on_off
        self.stream_source.SetBackward( on_off )
        self.updateVis( do_update )


    def setMaxPointsPerLine( self, n, do_update = 0  ):
        """
        How many steps a streamline proceeds before stopping (unless it leaves
        the domain first).
        """
        anag_utils.funcTrace()
        assert( n >= 0 )
        self.max_points_per_line = n
        self.stream_source.SetMaxPointsPerLine( n )
        self.updateVis( do_update )


    def setRelativeStepsize( self, x, do_update = 0  ):
        """
        How far each integration step proceeds relative to the local mesh
        spacing -- backward euler line integral integration.
        """
        anag_utils.funcTrace()
        assert( 0.0 <= x <= 1.0 )
        self.relative_stepsize = x
        self.stream_source.SetRelativeStepSize( x )
        self.updateVis( do_update )


    def setFieldThreshold( self, x, do_update = 0  ):
        """
        Minimum value we want to render.
        """
        anag_utils.funcTrace()
        assert( x >= 0.0 )
        self.field_threshold = x
        self.stream_source.SetFieldThreshold( x )
        self.updateVis( do_update )


    def setComponentMap( self, component_name, subscript, do_update = 0  ):
        """
        Associate a component name with one of the three vector components.
        Arg subscript can be 'v','w' or 'u'; these are the vector components.
        """
        anag_utils.funcTrace()

        self.component_map[subscript] = component_name
        component_number = self.local_vtk_data.getComponentList().index(
                                                                component_name )

        # Call SetUComponent(), SetVComponent() or SetWComponent().
        func_name = 'Set' + subscript.upper() + 'Component'
#       methods = self.stream_source.__methods__
#       methods[methods.index(func_name)]( str(component_number) )
#       That didn't work.  Call exec(), it's not expensive here anyway.
        exec( 'self.stream_source.' + func_name + '('
              + str(component_number) + ')' )

        self.updateVis( do_update )


    def setMaxLevel( self, n, do_update = 0  ):
        anag_utils.funcTrace()
        assert( 0 <= n <= self.local_vtk_data.getMaxAvailableLevel() )
        self.max_level = n
        self.updateVis( do_update )


    def setRgb(self, rgb, do_update = 0 ):
        """ Called from control_stream """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations('decls', 'self', 'rgb', 'do_update')

        self.rgb = ( rgb[0],rgb[1],rgb[2] )
        self.stream_prop.SetColor( rgb[0], rgb[1], rgb[2] )

        self.vtk_vtk.render()

        decls.memberFunctionAudit(self)

    
    def setLineWidth( self, w ):
        anag_utils.funcTrace()
        self.line_width = w
        self.stream_prop.SetLineWidth(w)
        self.tubes.SetRadius(w * self.line_width_multiplier)
        self.vtk_vtk.render()
        

    def setDiffuse( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        self.stream_prop.SetDiffuse( x )
        self.vtk_vtk.render()
    def setAmbient( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        self.stream_prop.SetAmbient( x )
        self.vtk_vtk.render()
    def setSpecular( self, x ):
        """ Lighting parameter """
        anag_utils.funcTrace()
        self.stream_prop.SetSpecular( x )
        self.vtk_vtk.render()


    def setTubesActive( self, yes_no ):
        anag_utils.funcTrace()
        assert( yes_no == 0  or  yes_no == 1 )
        if yes_no == 0:
            self.tubes.SetInput( None )
            self.mapper.SetInput( self.stream_source.GetOutput() )
        else:
            self.tubes.SetInput( self.stream_source.GetOutput() )
            self.mapper.SetInput( self.tubes.GetOutput() )
        self.tubes_active = yes_no
        self.vtk_vtk.render()


    def setTubeResolution( self, n ):
        """
        Cross-section of tube is a regular n-gon.  This function sets n.
        """
        anag_utils.funcTrace()
        self.tubes.SetNumberOfSides(n)
        self.tube_resolution = n
        self.vtk_vtk.render()


    def setSeedSize( self, x ):
        """
        Set the size -- in pixels along the side -- of the squares that
        indicate the seeds' positions.
        """
        anag_utils.funcTrace()
        self.seed_size = x
        self.stream_prop.SetPointSize(x)
        self.vtk_vtk.render()
    

    def setAlgorithm( self, x, do_update = 0  ):
        """
        Arg algorithm is Nothing, Fixed Step or Clipped Step.
        But we map these to -1,0,1.
        """
        anag_utils.funcTrace()
        self.algorithm = x
        self.stream_source.SetIntegrationMode( g_algo_tag_map[self.algorithm] )
        self.updateVis( do_update )


    def setShowGlyphs( self, on_off ):
        anag_utils.funcTrace()
        assert( on_off==0  or  on_off==1 )
        self.show_glyphs = on_off
        if on_off == 1:
            self.local_vtk_data.vtkUpdate( source = str(self.__class__) )
        else:
            self.glyphs.hide()


    def updateVis(self, do_update = 1 ):
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'self', 'do_update', 'box',
            'num_pieces', 'seeds_polydata', 'piece' )

        if do_update == 0:
            return
        else:
            if self.algorithm == g_algo_butt_tags.Nothing :
                self.vtk_vtk.removeActor( self.stream_actor )
            else:
                self.local_vtk_data.setMaxVisibleLevel( self.max_level )
                self.stream_source.SetMaxLevel( self.max_level )
                self.vtk_vtk.addActor( self.stream_actor)
                self.stream_source.Modified()
            self.vtk_vtk.render()


            if self.show_glyphs == 1:
                # Gotta set the curcomponent.  It hasn't been set because the
                # streamlines stuff doesn't use the conventional VTK pipeline
                # discipline; it goes straight to the vtkChomboReader and asks
                # for specific field-data points it's interested in.  But the
                # self.glyphs object does use the VTK pipeline and its
                # local_vtk_data is the same object as self.local_vtk_data here.
                # If we don't set curcomponent here, there's nothing in the
                # pipeline for self.glyphs to work on.  The actual component
                # we set here doesn't matter; it's only needed to get the
                # vtkReader to load "pieces".  There's no waste in this, since
                # all the pieces had to be loaded already, to figure out where
                # to draw the streamlines.
                self.local_vtk_data.setCurComponent(
                    self.component_map['u'] )

                self.glyphs.updateAccumStart(
                    min_level=0, max_level=self.max_level,
                    padded = self.local_vtk_data.getAlwaysUsePaddedBoxes() )
                num_pieces = self.local_vtk_data.getReader().GetNumPieces()

                for piece in range(0,num_pieces):
                    seeds_polydata = self.seed_object.getSeedsAsPolyData( piece)
                    self.glyphs.updateAccumMiddle( piece, seeds_polydata )
                self.glyphs.updateAccumEnd()

        decls.memberFunctionAudit(self)


    def lineSeedsFactory( self ):
        """
        Return a new _LineSeeds object.
        """
        anag_utils.funcTrace()

        if self.seed_object:
            return self.seed_object
        else:
            result = _LineSeeds(
                dep_dict={'saved_states':self.saved_states,
                          'local_vtk_data':self.local_vtk_data},
                vtk_seeds=self.stream_seeds )

            self.updateVis()
            return result


    def unitTest( self ):
        if self.saved_states.getNumHDF5sLoaded() > 0:
            line_seeds = self.lineSeedsFactory()
            line_seeds.rotate( rho=15, phi=0 )
            line_seeds.translate( x=0.10, y=0.10 )

            self.setIntegrateForward( 1 )
            self.setIntegrateBackward( 1 )
            self.setMaxPointsPerLine( 1500 )
            self.setRelativeStepsize( 0.25 )
            self.setFieldThreshold( 1E-6 )
            self.setComponentMap( 'x-momentum', 'u' )
            self.setComponentMap( 'y-momentum', 'v' )
            self.setComponentMap( 'z-momentum', 'w' )
            self.setMaxLevel( 0 )
            self.setRgb( (1.0, 0.5, 0.0) )
            self.setAlgorithm( 'Fixed Step' )
            self.updateVis()


class _Seeds( SelfControl ):
    """
    Superclass for a variety of 3D point collections that can be transformed
    in various ways.  These points serve as the starting positions for
    streamlines.

    "Abstract base class".
    """
    def __init__( self, dep_dict,
                  vtk_seeds # vtkPoints
    ):
        SelfControl.__init__( self, dep_dict, metadata = [
            {'name':'vtk_seeds'},
            {'name':'points'},
            {'name':'transformations'},  # tuple of functions
            {'name':'transforms_state', 'get':1, 'save':1, 'initval':{} },
                # Dictionary, keys 'x','y','z','rho','phi','n_seeds', 'length'.

            {'name':'extents'},          # Of interest only to subclass
            {'name':'longest_diagonal'}  # Of interest only to subclass
            ])

        self.decls = anag_utils.Declarations( 'decls', instance=self )

        self.vtk_seeds = vtk_seeds # vtkPoints
        self.transforms_state = { 'rho':0, 'phi':0,
                                  'x':0, 'y':0, 'z':0,
                                  'n_seeds':10, 'length':1.0 }

        self.points = [] # List of dictionaries, keys 'x','y','z'.


        # We'll iterate through this collection inside each transformation:
        # elements of this collection are special null versions of the
        # transformations, that just update to the current state, rather than
        # changing the state.
        self.transformations = ( lambda self=self: self.populate(),
                                 lambda self=self: self.rotate(),
                                 lambda self=self: self.translate(),
                                 lambda self=self: self.stretch()
                                 # more when appropriate
                               )


    def getNSeeds( self ): return self.transforms_state['n_seeds']
    def getLength( self ): return self.transforms_state['length']
    def getX( self ):       return self.transforms_state['x']
    def getY( self ):       return self.transforms_state['y']
    def getZ( self ):       return self.transforms_state['z']
    def getRho( self ):     return self.transforms_state['rho']
    def getPhi( self ):     return self.transforms_state['phi']

    def getSeedsAsPolyData( self, piece ):
        """
        Returns a one-cell polydata whose points are that subset of
        self.vtk_seeds that lies within the box_num-th piece.
        """
        anag_utils.funcTrace()

        self.local_vtk_data.setPiece( piece )

        n_all = self.vtk_seeds.GetNumberOfPoints()
        assert( n_all == self.getNSeeds() )
        points_in_box = vtkpython.vtkPoints()
        level = self.local_vtk_data.getReader().GetCurrentPieceLevel()
        box_num = self.local_vtk_data.getReader().GetCurrentPieceBoxNum()

        for p_all in range(0,n_all):
            (x,y,z) = self.vtk_seeds.GetPoint(p_all)
            if( self.local_vtk_data.getReader().GetEnclosingBoxNumXYZ(
                                                 level, x, y, z ) == box_num ):
                points_in_box.InsertNextPoint( self.vtk_seeds.GetPoint(p_all) )

        n_in_box = points_in_box.GetNumberOfPoints()
        cells = vtkpython.vtkCellArray()
        cells.InsertNextCell( n_in_box )
        for p in range(0, n_in_box):
            cells.InsertCellPoint(p)
        polydata = vtkpython.vtkPolyData()
        polydata.SetPoints( points_in_box )
        polydata.SetPolys( cells )
        return polydata


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()
        self._anisotropicFactorsNotifier(None,None)
        self.registerCallback(
            self.local_vtk_data.getNotifierAnisotropicFactorsNotifier(),
            self._anisotropicFactorsNotifier )


    def cleanup( self ):
        anag_utils.funcTrace()
        self.unregisterCallbacks()
        self.zeroSelfVariables()
        

    def _anisotropicFactorsNotifier( self, dummy1, dummy2 ):
        anag_utils.fatal( "You must override this function." )

    def getPoints( self ): return self.points

    def getTransformsState( self ): return self.transforms_state

    def populate( self, n_seeds=None ):
        anag_utils.fatal( "You must override this function." )        

    def rotate( self, rho=None, phi=None ):
        anag_utils.fatal( "You must override this function." )

    def translate( self, x=None, y=None, z=None ):
        anag_utils.fatal( "You must override this function." )

    def stretch( self ):
        anag_utils.fatal( "You must override this function." )


class _LineSeeds( _Seeds ):
    """
    Seed points along a line segment.  Its initial position is in the near
    corner of the domain box.
    """
    def __init__( self, dep_dict, vtk_seeds ):
        anag_utils.funcTrace()
        _Seeds.__init__( self, dep_dict, vtk_seeds )

        self.extents = self.local_vtk_data.getDomainExtentsXYZ()
        self.longest_diagonal = math.pow(
                math.pow( self.extents[3]-self.extents[0], 2 )
               +math.pow( self.extents[4]-self.extents[1], 2 )
               +math.pow( self.extents[5]-self.extents[2], 2 ), 0.5 )
        self.transforms_state['length'] = algorithms.roundDown(
            (self.extents[3]-self.extents[0])/self.longest_diagonal, 2 )

        self.populate( self.transforms_state['n_seeds'] )
        self.rotate( rho = self.transforms_state['rho'],
                     phi = self.transforms_state['phi'] )
        self.translate( x = self.transforms_state['x'],
                        y = self.transforms_state['y'],
                        z = self.transforms_state['z'] )
        self.stretch( self.transforms_state['length'] )


    def _anisotropicFactorsNotifier( self, dummy1, dummy2 ):
        self.populate( self.transforms_state['n_seeds'] )


    def populate( self, n_seeds=None ):
        """
        Lay down n_seeds or, if None, self.transforms_state['n_seeds'].
        Lay them down along the x axis, along that edge of the domain box.

        The idea behind populate(), rotate() and the other transformations
        is that, called with an argument (be it n_seeds for populate,
        angles for rotate etc) they just set the corresponding instance variable
        (self.n_seeds, self.rho, ...) to the value of the argument.  Then they
        call all the transformations in order with no arguments.  Then, called
        with no arguments, the transformations actually do their work.
        Thus, modifying any of the transformation parameters -- n_seeds, rho,
        etc -- causes all the transformations to be called in order.
        """
        anag_utils.funcTrace()

        if n_seeds!=None:
            self.transforms_state['n_seeds'] = n_seeds
            for t in self.transformations:
                apply( t, () )
        else:
            self.points = []

            if( self.transforms_state['n_seeds'] 
            >   self.vtk_seeds.GetNumberOfPoints() ):
                for i in range( self.vtk_seeds.GetNumberOfPoints(),
                                self.transforms_state['n_seeds'] ):
                    self.vtk_seeds.InsertPoint( i, 0,0,0 )
            if( self.transforms_state['n_seeds']
            <   self.vtk_seeds.GetNumberOfPoints() ):
                self.vtk_seeds.Reset()
                self.vtk_seeds.Squeeze()
                # FIXME: is there any way to delete just individual points?
                for i in range(0, self.transforms_state['n_seeds'] ):
                    self.vtk_seeds.InsertPoint( i, 0,0,0 )

            for i in range(0,self.transforms_state['n_seeds']):
                point = {}
                frac = i/float( self.transforms_state['n_seeds'] )
                x = self.extents[0] * (1-frac) \
                  + (self.extents[0]+self.longest_diagonal) * frac
                y = self.extents[1]
                z = self.extents[2]
                point['x'] = x
                point['y'] = y
                point['z'] = z
    
                self.points.append( point )
                self.vtk_seeds.SetPoint( i, x,y,z )

            self.initial_points = []
            for p in self.points:
                p_copy = {}
                for a in 'x','y','z': p_copy[a] = p[a]
                self.initial_points.append( p_copy )


    def rotate( self, rho=None, phi=None ):
        """
        Rho rotations are in the (x,z) plane.
        Phi rotations are in the (x,y) plane.

        Angles are given in degrees.

        Rotations are relative to the initial position (where the line lies
        along the x axis); thus rotations are not incremental.
        """
        anag_utils.funcTrace()

        if rho!=None or phi!=None:
            if rho!=None:
                self.transforms_state['rho'] = rho
            if phi!=None:
                self.transforms_state['phi'] = phi
            for t in self.transformations:
                apply( t, () )
        else:
            radian = 180/math.pi
            i_point = 0
            origin = self.initial_points[0]
            for point in self.initial_points:
                r = point['x'] - origin['x']
                x = r * math.cos( self.transforms_state['rho']/radian ) \
                      * math.cos( self.transforms_state['phi']/radian )
                y = r * math.sin( self.transforms_state['phi']/radian )
                z = r * math.sin( self.transforms_state['rho']/radian )

                self.points[i_point] = {'x': x + origin['x'],
                                        'y': y + origin['y'],
                                        'z': z + origin['z']}
                self.vtk_seeds.SetPoint( i_point, x,y,z )
                i_point = i_point + 1

            
    def translate( self, x=None, y=None, z=None ):
        anag_utils.funcTrace()
        
        if x!=None or y!=None or z!=None:
            if x!=None:
                self.transforms_state['x'] = x
            if y!=None:
                self.transforms_state['y'] = y
            if z!=None:
                self.transforms_state['z'] = z
            for t in self.transformations:
                apply( t, () )
        else:
            i = 0
            for point in self.points:
                point['x'] = point['x'] + self.transforms_state['x']
                point['y'] = point['y'] + self.transforms_state['y']
                point['z'] = point['z'] + self.transforms_state['z']
                self.vtk_seeds.SetPoint( i,
                                         point['x'], point['y'], point['z'] )
                i = i + 1


    def stretch( self, length=None ):
        """
        Stretch or compress the rake.  Arg length is taken as a fraction of the
        length of the domain's longest diagonal.
        """
        anag_utils.funcTrace()

        if length!=None:
            self.transforms_state['length'] = length
            for t in self.transformations:
                apply( t, () )
        else:
            endpoints = {}
            l = self.transforms_state['length']
            n_seeds = self.transforms_state['n_seeds']
            for axis in 'x','y','z':
                endpoints[axis] = []
                endpoints[axis].append( self.points[0][axis] )
                endpoints[axis].append(
                                 self.points[0][axis] * (1-l)
                               + self.points[n_seeds-1][axis] * l )
            i=0
            for point in self.points:
                frac = i/float(self.transforms_state['n_seeds'])
                for axis in 'x','y','z':
                    point[axis] = (
                        (1-frac) * endpoints[axis][0]
                      + frac * endpoints[axis][1] )
                self.vtk_seeds.SetPoint( i, point['x'], point['y'], point['z'] )
                i = i+1


# Algorithm button tags
class AlgoButtTags:
    pass
g_algo_butt_tags = AlgoButtTags()
g_algo_butt_tags.Nothing = 'Nothing'
g_algo_butt_tags.Fixed_Step = 'Fixed Step'
g_algo_butt_tags.Clipped_Step = 'Clipped Step'
g_algo_tag_map = { g_algo_butt_tags.Nothing:           -1,
                   g_algo_butt_tags.Fixed_Step:         0,
                   g_algo_butt_tags.Clipped_Step:       1 }
