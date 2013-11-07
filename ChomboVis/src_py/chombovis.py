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

# File: chombovis.py
# Author: TDSternberg
# Created: 5/18/01

""" Entry point into ChomboVis """

import anag_utils

#g_imports_timer = anag_utils.Timer( label='chombovis imports:' )
#g_imports_timer.on()

import Pmw
import os
import os.path
import sys

import self_control
from dialog_inventory import DialogInventory
from cmd_line import CmdLine
from saved_states import SavedStates
from vtk_axes import VtkAxes
from vtk_data import VtkData
from vtk_cmap import VtkCmap
from vtk_clip import VtkClip
from control_clip import ControlClip
from vtk_grid import VtkGrid
from vtk_iso import VtkIso
import vtk_iso # For g_constant_color and g_self_color
from vtk_slice import VtkSlice
from vtk_stream import VtkStream
from vtk_vol import VtkVol
from vtk_particles import VtkParticles
from vtk_eb import VtkEb
from vtk_iso_eb import VtkIsoEb
from control_eb import ControlEb
from vtk_print import VtkPrint
from vtk_vtk import VtkVtk
from vtk_cameras import VtkCameras
from vtk_annotation import VtkAnnotation
from control_cameras import ControlCameras
from vtk_fab_tables import VtkFabTables
from control_data import ControlData
from control_cmap import ControlCmap
from control_grid import ControlGrid
from control_slice import ControlSlice
from control_iso import ControlIso
from control_stream import ControlStream
from control_particles import ControlParticles
from control_vol import ControlVol
from control_fab_tables import ControlFabTables
from control_annotation import ControlAnnotation
from selector import Selector
from control_print import ControlPrint
from menubar import Menubar
from vol_tk_interactor import VolTkInteractor

from reader_api import ReaderApi
from grid_api import GridApi
from cmap_api import CmapApi
from clip_api import ClipApi
from eb_api import EbApi
from slice_api import SliceApi
from iso_api import IsoApi
from stream_api import StreamApi
from volume_api import VolumeApi
from misc_api import MiscApi
from annotation_api import AnnotationApi
from particles_api import ParticlesApi
from network_api import NetworkApi

import visualizable_dataset

#g_imports_timer.stop()


##################### Factories ####################
"""
Users don't call the ChomboVis constructor directly.  Instead, they obtain
references to ChomboVis objects from the following two factories.  The
this() factory, whenever and however often it's called, always returns a
reference to the same ChomboVis object.  The new() factory returns a reference
to a new ChomboVis object.
"""

g_decls = anag_utils.Declarations( 'g_decls', 'g_ChomboVis_instances',
    'g_obscure_value' )
g_obscure_value=19964 # To make it hard for users to call ChomboVis ctor.
g_ChomboVis_instances = []
def getChomboVisInstances():
    """
    Return all the live ChomboVis instances.  So, first, delete those whose
    readers have been deleted (that being a sign they are dead).
    """
    global g_ChomboVis_instances
    def nonzeroReader(c):
        return c.vtk_data.getReader() != 0
    g_ChomboVis_instances = filter( nonzeroReader, g_ChomboVis_instances )
    return g_ChomboVis_instances

def this( virtual_command_line={} ):
    """
    Deprecated, use new() or latest() instead.
    """
    return latest( virtual_command_line )

def latest( virtual_command_line={} ):
    """
    Returns most recently constructed ChomboVis instance.  If none have ever
    been constructed, construct one and return that.
    """
    global g_ChomboVis_instances
    if g_ChomboVis_instances == []:
        g_ChomboVis_instances.append( ChomboVis( virtual_command_line,
                                                 g_obscure_value ) )
    return g_ChomboVis_instances[ -1: ][0]

def new( virtual_command_line={} ):
    """
    Constructs a new ChomboVis instance and returns it.
    """
    global g_ChomboVis_instances
    anag_utils.info("Constructing a ChomboVis...")
    c = ChomboVis( virtual_command_line, g_obscure_value )
    g_ChomboVis_instances.append( c )
    return g_ChomboVis_instances[ -1: ][0]

def getNumChomboVisInstances(): return len(g_ChomboVis_instances )

############### End of Factories ###################


class ChomboVis:
    """
    The API consists of methods on data members of the ChomboVis class.
    These data members are
    1. self.clip -- an instance of ClipApi.
    2. self.cmap -- an instance of CmapApi.
    3. self.reader -- an instance of ReaderApi.
    4. self.eb -- an instance of EbApi.
    5. self.grid -- an instance of GridApi.
    6. self.iso -- an instance of IsoApi.
    7. self.misc -- an instance of MiscApi.
    8. self.slice -- an instance of SliceApi.
    9. self.stream -- an instance of StreamApi.
   10. self.volume -- an instance of VolumeApi.
   11. self.particles -- an instance of ParticlesApi.
   12. self.network -- an instance of NetworkApi.
   13. self.annotation -- an instance of AnnotationApi.

    See the examples directory for examples of how to use the API.
    """

#The following line is a directive for ../doc/make_doc.sh:
#Cut from here for pydoc.  Do not delete this line!!

    def __init__( self, virtual_command_line, obscure=0 ):

#       chombovis_init_timer = anag_utils.Timer(
#          label='chombovis() __init__ except loadHDF5:')
#       chombovis_init_timer.on()

        """
        Arg obscure is here to make it hard for users to call the ChomboVis
        constructor directly.  The correct way to obtain a ChomboVis object is
        from the two factory methods chombovis.this() and chombovis.new().
        """

        anag_utils.funcTrace()
        self.decls = anag_utils.Declarations( 'decls',
                    'virtual_command_line',
                    'init_data',
                    'saved_states',
                    'cmd_line',
                    'dialog_inventory',
                    'vtk_vtk',
                    'vtk_data',
                    'vtk_cmap',
                    'vtk_grid',
                    'vtk_clip',
                    'vtk_slice',
                    'vtk_iso',
                    'vtk_stream',
                    'vtk_vol',
                    'vtk_particles',
                    'vtk_eb',
                    'vtk_iso_eb',
                    'control_eb',
                    'vtk_axes',
                    'vtk_annotation',
                    'selector',
                    'vtk_cameras',
                    'control_cameras',
                    'vtk_fab_tables',
                    'vol_tk_interactor',
                    'vtk_print',
                    'control_fab_tables',
                    'control_data',
                    'control_cmap',
                    'control_grid',
                    'control_clip',
                    'control_particles',
                    'control_slice',
                    'control_iso',
                    'control_stream',
                    'control_vol',
                    'control_print',
                    'control_annotation',
                    'menubar',
                    'reader',
                    'cmap',
                    'clip',
                    'grid',
                    'slice',
                    'particles',
                    'iso',
                    'stream',
                    'volume',
                    'eb',
                    'misc',
                    'reader_api',
                    'grid_api',
                    'cmap_api',
                    'clip_api',
                    'slice_api',
                    'stream_api',
                    'volume_api',
                    'iso_api',
                    'eb_api',
                    'misc_api',
                    'annotation_api',
                    'particles_api',
                    instance=self  )
        decls = anag_utils.Declarations( 'decls', 'virtual_command_line',
            'obscure' )
        self.virtual_command_line = virtual_command_line

        # This table shows the inter-class dependencies in the entire project.
        # On each line, the first element is the name of a class we're going
        # to construct.  The second element is a list of the classes it depends
        # on.  A class cannot depend on a class mentioned further down the page,
        # e.g. you can't make vtk_data depend on vtk_slice.
        #
        # We'd make this thing a dictionary, except the order of elements would
        # then not be guaranteed.
        self.init_data = [
            ('cmd_line',  # Danger: don't make anything lower-level than
                []),      # cmd_line.  (Lots of code below assumes that.)
            ('saved_states',
                []),
            ('vtk_vtk',
                ['saved_states', 'cmd_line']),
            ('vtk_data',
                ['saved_states','cmd_line','vtk_vtk']),
            ('vtk_clip',
                ['saved_states', 'vtk_vtk', 'vtk_data']),
            ('vtk_fab_tables',
                ['saved_states', 'cmd_line', 'vtk_vtk', 'vtk_data']),
            ('vtk_cmap',
                ['saved_states', 'cmd_line', 'vtk_vtk', 'vtk_data']),
            ('vtk_grid',
                ['saved_states', 'vtk_vtk', 'vtk_cmap', 'vtk_clip','vtk_data']),
            ('vtk_slice',
                ['saved_states', 'cmd_line', 'vtk_clip', 'vtk_vtk', 'vtk_data',
                 'vtk_cmap']),
            ('vtk_stream',
                ['saved_states', 'vtk_vtk', 'cmd_line', 'vtk_data',
                 'vtk_cmap']),
            ('vtk_vol',
                ['saved_states', 'vtk_vtk', 'vtk_data']),
            ('vtk_eb',
                ['saved_states', 'vtk_clip', 'vtk_vtk', 'vtk_data',
                 'vtk_cmap', 'vtk_slice']),
            ('vtk_cameras',
                ['saved_states', 'cmd_line', 'vtk_vtk', 'vtk_data']),
            ('vtk_particles',
                ['saved_states', 'vtk_vtk', 'vtk_data', 'vtk_cmap',
                 'vtk_cameras']),
            ('vtk_axes',
                ['saved_states', 'cmd_line', 'vtk_vtk', 'vtk_cameras',
                 'vtk_data']),
            ('vtk_iso',
                ['saved_states', 'vtk_clip', 'vtk_vtk', 'vtk_data','vtk_cmap',
                 'vtk_cameras']),
            ('vtk_iso_eb',
                ['saved_states', 'cmd_line', 'vtk_clip', 'vtk_vtk', 'vtk_data',
                 'vtk_cmap', 'vtk_slice', 'vtk_iso', 'vtk_cameras']),
            ('vtk_annotation',
                ['saved_states', 'vtk_vtk', 'vtk_data']),
            ('selector',
                ['saved_states', 'vtk_vtk', 'vtk_data', 'vtk_slice',
                 'vtk_fab_tables']),
            ('vtk_print',
                ['saved_states', 'cmd_line', 'vtk_vtk', 'vtk_cmap']),

            ('network_api',
                ['vtk_vtk']),

            ('vol_tk_interactor',
                ['saved_states', 'cmd_line', 'vtk_vtk', 'vtk_data', 'vtk_axes',
                 'selector', 'vtk_particles', 'network_api']),
            ('dialog_inventory', []),


            ('control_fab_tables',
                ['saved_states', 'cmd_line', 'vtk_data', 'vtk_vtk', 'vtk_slice',
                 'vtk_fab_tables', 'dialog_inventory']),
            ('control_data',
                ['saved_states', 'vtk_data','dialog_inventory']),
            ('control_clip',
                ['saved_states', 'vtk_clip', 'vtk_data','dialog_inventory']),
            ('control_cmap',
                ['saved_states', 'vtk_cmap','vtk_data','vtk_vtk',
                 'dialog_inventory']),
            ('control_grid',
                ['saved_states', 'vtk_data','vtk_grid','dialog_inventory']),
            ('control_slice',
                ['saved_states', 'vtk_data','vtk_slice','dialog_inventory']),
            ('control_iso',
                ['saved_states', 'vtk_data','vtk_iso','dialog_inventory']),
            ('control_stream',
                ['saved_states', 'vtk_vtk', 'vtk_data','vtk_stream',
                 'dialog_inventory']),
            ('control_particles',
                ['saved_states', 'vtk_vtk', 'vtk_data','vtk_particles',
                 'dialog_inventory']),
            ('control_vol',
                ['saved_states', 'vtk_vtk', 'vtk_data','vtk_vol',
                 'dialog_inventory']),
            ('control_eb',
                ['saved_states', 'cmd_line', 'vtk_data', 'vtk_eb', 'vtk_iso_eb',
                 'dialog_inventory']),
            ('control_cameras',
                ['saved_states', 'vtk_vtk', 'vtk_cameras','vol_tk_interactor',
                 'dialog_inventory']),
            ('control_print',
                ['saved_states', 'cmd_line', 'vtk_print', 'dialog_inventory']),
            ('control_annotation',
                ['saved_states', 'vtk_data', 'vtk_vtk', 'vtk_annotation',
                 'dialog_inventory']),

            ('reader_api',
                ['saved_states', 'vtk_vtk', 'vtk_data', 'control_data']),
            ('grid_api',
                ['vtk_vtk', 'vtk_grid', 'control_grid'] ),
            ('slice_api',
                ['vtk_data', 'vtk_slice', 'control_slice']),
            ('iso_api',
                ['vtk_iso', 'vtk_vtk', 'control_iso']),
            ('stream_api',
                ['vtk_stream', 'control_stream'] ),
            ('volume_api',
                ['vtk_vol', 'control_vol']),
            ('cmap_api',
                ['vtk_cmap', 'vtk_data', 'control_cmap']),
            ('clip_api',
                ['vtk_clip', 'control_clip']),
            ('eb_api',
                ['saved_states', 'cmd_line', 'vtk_data', 'vtk_eb',
                 'vtk_iso_eb', 'vtk_vtk', 'vtk_cmap', 'control_eb']),
            ('particles_api',
                ['vtk_data', 'vtk_particles', 'control_particles']),
            ('annotation_api',
                ['vtk_annotation', 'control_annotation']),
            ('misc_api',
                ['saved_states', 'cmd_line', 'vtk_data', 'vtk_vtk', 'vtk_cmap',
                 'vol_tk_interactor', 'vtk_cameras', 'vtk_print', 'selector',
                 'dialog_inventory', 'control_fab_tables']),

            ('menubar',
                ['saved_states', 'cmd_line', 'vtk_vtk','vtk_data', 'vtk_axes',
                 'control_print', 'control_fab_tables', 'control_data',
                 'control_cmap', 'control_grid', 'control_slice', 'control_iso',
                 'control_stream', 'control_vol', 'control_eb', 'control_clip',
                 'control_particles', 'control_cameras','control_annotation',
                 'dialog_inventory',
                 'reader_api', 'grid_api', 'cmap_api',
                 'clip_api', 'eb_api', 'slice_api', 'iso_api', 'stream_api',
                 'volume_api', 'misc_api', 'network_api', 'annotation_api' ])
        ]

        if obscure != g_obscure_value:
            anag_utils.fatal(
            """
            Do not call the ChomboVis constructor directly.  Instead, call
            one of the two factory methods chombovis.this() and chombovis.new().
            """
            )

#       makeClasses_timer = anag_utils.Timer(label='chombovis.makeClasses()')
#       makeClasses_timer.on()
        self._makeClasses()
#       makeClasses_timer.stop()

#       chombovis_init_timer.stop()

#       loadHDF5_timer = anag_utils.Timer(label='loadHDF5 timer')
#       loadHDF5_timer.on()

        if self.cmd_line.getInfile():
            self.vtk_data.loadHDF5( self.cmd_line.getInfile() )
            self.vtk_data.vtkUpdate( source = str(self.vtk_data.__class__) )

        self.vtk_vtk.getNotifierNewHDF5Notifier().addCallback(
            self.vtkvtkNewHDF5NotifierHandler, unremovable=1 )

        if self.cmd_line.getTestClass():
            self.__dict__[self.cmd_line.getTestClass()].unitTest()

        self.initAPI()

        if( self.cmd_line
        and self.cmd_line.getSlicesOff() == 0
        and self.reader
        and self.reader.getNumComponents() > 0 ):
            self.reader.setCurrentComponent( self.reader.getComponentNames()[0])
            if self.control_data:
                self.control_data.refresh()  # Update component-selection widget

        decls.memberFunctionAudit( self )
#       loadHDF5_timer.stop()


    def initAPI( self ):
        """
        They've already been initialized, but here we provide aliases, for
        backward compatibility with how the API used to work.
        """
        self.reader = self.reader_api
        self.grid   = self.grid_api
        self.cmap   = self.cmap_api
        self.clip   = self.clip_api
        self.eb     = self.eb_api
        self.slice  = self.slice_api
        self.iso    = self.iso_api
        self.stream = self.stream_api
        self.volume = self.volume_api
        self.misc   = self.misc_api
        self.particles = self.particles_api
        self.network = self.network_api
        self.annotation = self.annotation_api

        if self.misc_api:  # Can be None, in test mode
            self.misc_api.getNotifierVtkUpdate().addCallback(
                lambda d1,d2,self=self: self._vtkUpdate() )

        # Backwards compatibility:
        if self.reader and self.misc:
            self.misc.loadHDF5 = lambda args, message=\
                "misc.loadHDF5 is deprecated, use reader.loadHDF5 instead",\
                f=self.reader.loadHDF5 :\
                    anag_utils.deprecator( f, message, args )
            self.misc.saveToHDF5 = lambda args, message=\
                "misc.saveToHDF5 is deprecated, use reader.saveToHDF5 instead",\
                f=self.reader.saveToHDF5 :\
                anag_utils.deprecator( f, message, args )
            self.misc.onNewHDF5 = lambda args, message=\
                "misc.onNewHDF5 is deprecated, use reader.onNewHDF5 instead",\
                f=self.reader.onNewHDF5 :\
                anag_utils.deprecator( f, message, args )


    def cleanupAPI( self ):
        return

        anag_utils.funcTrace()
        self.grid = self.grid_api             = None
        self.clip = self.clip_api             = None
        self.cmap = self.cmap_api             = None
        self.slice = self.slice_api           = None
        self.iso = self.iso_api               = None
        self.stream = self.stream_api         = None
        self.volume = self.volume_api         = None
        self.particles = self.particles_api   = None
        self.network = self.network_api       = None
        self.annotation = self.annotation_api = None
        self.eb  = self.eb_api                = None
        self.misc = self.misc_api             = None


    def _cleanupClasses( self ):
        """
        Call the cleanup() method of each class mentioned in self.init_data,
        except vtk_vtk.
        Each class then removes actors and releases whatever resources it's
        holding.  This happens before we load a new hdf5.

        Go through the classes in reverse order.
        """
        anag_utils.funcTrace()

        reverse_init_data = list(self.init_data)
        reverse_init_data.reverse()

        file_is_loaded = (self.saved_states.getNumHDF5sLoaded() > 0)

        for item in reverse_init_data:
            if( file_is_loaded or (item[0] == 'menubar') ):
                self._cleanupClass( item )

        for item in reverse_init_data:
            if item[0] != 'vtk_vtk':
                self.__dict__[item[0]] = None

        self.cleanupAPI()


    def _cleanupClass( self, init_datum ):
        """
        Arg init_datum is one element of self.init_data.
        See comments under self._cleanupClasses().
        """
        anag_utils.funcTrace()
        try:
            obj = self.__dict__[init_datum[0]]
            obj.cleanup()
            obj.unregisterCallbacks()
            obj.zeroSelfVariables()
            # Warning: You need to call unregisterCallbacks() and
            # zeroSelfVariables() explicitly in the cleanup() method of any
            # subclass of SelfControl which is not mentioned in self.init_data.
            # For example: anag_megawidgets.EntryScale, vtk_slice._SlicingPlane.
        except:
            if self.cmd_line.getTestClass():
                anag_utils.info( "No instance of class", init_datum[0] )
            else:
                anag_utils.excepthook()
                sys.exit(1)


    def _makeClass( self, init_datum ):
        """
        Arg init_datum is one element of self.init_data.
        """
        anag_utils.funcTrace()
#       timer = anag_utils.Timer(label='makeClass ' + init_datum[0] + ' timer')
#       timer.on()
        module_name = anag_utils.getModuleName(2)
        module = sys.modules[ module_name ]
        self.__dict__[init_datum[0]] =\
            apply( module.__dict__[anag_utils.upcaseStyle(init_datum[0])],
                   (self._makeDict(init_datum[1]),) )
#       timer.stop()


    def _makeClasses( self, skip_vtkvtk=None ):
        """
        Go through self.init_data and construct either all the classes
        or subsets of them, according to the command-line arguments test_class
        and test_mode.

        If arg skip_vtkvtk!=None, then don't construct VtkVtk.  That's for when
        we load a new hdf5.  *BUT* do call ChomboVis._resetVtkVtk().

        If test_class == None, then construct all the classes.  This sets up
        a normal GUI-controlled ChomboVis session.

        If test_class != None, that means we're going to run the unit test for
          the indicated class (e.g. test_class==vtk_data -- we use the foo_bar
          spelling, rather than FooBar).

          If test_mode == 'max', then construct all the classes, then run
          the unit test.

          If test_mode == 'mid', then construct all the classes from the 
          lowest level (cmd_line), and through test_class.

          If test_mode = 'min', then construct only the minimal necessary set
          of classes (i.e. those test_class depends on, directly or indirectly).

        Each class constructor is passed a list of references to the classes
        it depends on.  The constructors create an instance variable for each
        dependency.  For example, if 'vtk_vtk' is in the dependency list of
        class vtk_data, that means the instance of vtk_data will have an
        attribute called self.vtk_vtk, which will be a reference to the (one)
        vtk_vtk instance constructed here.

        If there appears to be some pretty hairy Python stuff in here, it's
        only to make possible a very succinct expression of dependencies.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'decls', 'ctor', 'item',
            'begin_class', 'end_class' )

        if skip_vtkvtk:
            assert( self.vtk_vtk )

        # Initialize all the instances to None, so we don't get the
        # "has no attribute" error when we pass self.* to the API ctors.  This
        # would be a problem in test_class mode.
        for item in self.init_data:
            if item[0] != 'vtk_vtk':
                self.__dict__[item[0]] = None

        # Construct everything up to class CmdLine.
        for item in self.init_data:
            self._makeClass( item )
            if item[0] == 'cmd_line':
                break
        self.cmd_line.overrideCommandLine( self.virtual_command_line )

        # Construct the rest, based on what the command line says about unit
        # testing.
        if self.cmd_line.getTestClass():
            if( self.cmd_line.getTestMode() == 'min'
            or  not self.cmd_line.getTestMode()):
                min_init_data = self._findMinClassSet(
                                    self.cmd_line.getTestClass())
                for item in self.init_data:
                    if item in min_init_data:
                        if( (item[0] != 'cmd_line')
                        and ( (skip_vtkvtk!=1) or (item[0] != 'vtk_vtk'))):
                            anag_utils.info( 'Constructing ', item[0] )
                            self._makeClass( item )
                        elif (skip_vtkvtk==1) and (item[0] == 'vtk_vtk'):
                            self._resetVtkVtk( item[1] )
            elif( (self.cmd_line.getTestMode() == 'mid')
            or    (self.cmd_line.getTestMode() == 'max') ):
                for item in self.init_data:
                    if( (item[0] != 'cmd_line')
                    and ( (skip_vtkvtk!=1) or (item[0] != 'vtk_vtk'))):
                        anag_utils.info( 'Constructing ', item[0] )
                        self._makeClass( item )
                    elif (skip_vtkvtk==1) and (item[0] == 'vtk_vtk'):
                        self._resetVtkVtk( item[1] )
                    if( (self.cmd_line.getTestMode()=='mid')
                    and (item[0] == self.cmd_line.getTestClass())):
                        break
            else:
                anag_utils.fatal( 'Illegal command-line test_mode option:',
                    self.cmd_line.getTestMode() )
        else: # Normal run (i.e. not a unit test).
            for item in self.init_data:
                if( (item[0] != 'cmd_line')
                and ( (skip_vtkvtk!=1) or (item[0] != 'vtk_vtk'))):
                        #anag_utils.info( 'Constructing ', item[0] )
                        self._makeClass( item )
                elif (skip_vtkvtk==1) and (item[0] == 'vtk_vtk'):
                    self._resetVtkVtk( item[1] )


    def _resetVtkVtk( self, vtkvtk_dependencies ):
        """
        This is called when we've cleaned up from one hdf5 and we're loading
        a new one.  During that process, we re-construct all our classes,
        except for VtkVtk (so that we maintain the same render window).  Still,
        we have to do something about the fact that VtkVtk's references to its
        dependencies (CmdLine and SavedStates at this writing) and its
        registration with any notifiers in those states are now stale.        
        """
        anag_utils.funcTrace()
        assert( self.vtk_vtk )

        for name in vtkvtk_dependencies:
            instance = self.__dict__[name]
            self.vtk_vtk.setDependency( name, instance )
        self.vtk_vtk.refresh()


    def _findMinClassSet( self, ultimate_class ):
        """
        Return a subset of the elements of self.init_data that represents the
        minimal set of classes necessary to construct arg ultimate_class.
        This is the set of all classes in ultimate_class's dependency list,
        plus all those in their dependency lists, etc.
        """
        anag_utils.funcTrace()

        # Convenience dictionary.
        dict = {}
        for i in self.init_data:
            dict[ i[0] ] = i[1]
        
        deps = [(ultimate_class, dict[ultimate_class])]
        if len( dict[ultimate_class] ) > 0:
            self._appendDeps( ultimate_class, deps, dict )

        uniq_deps = []
        uniq_class_names = []
        for c in deps:
            if not c[0] in uniq_class_names:
                uniq_deps.append( c )
                uniq_class_names.append( c[0] )

        return uniq_deps


    def _appendDeps( self, ultimate_class, deps, dict ):
        """
        Append to list deps all the direct dependencies of ultimate_class.
        Recursive.
        """
        anag_utils.funcTrace()
        for c in dict[ultimate_class]:
            deps.append( (c, dict[c]) )
            if len( dict[c] ) > 0:
                self._appendDeps( c, deps, dict )


    def _makeDict(self, obj_names):
        """
        Turn a list like ('cmd_line','vtk_vtk') into a dictionary
        {'cmd_line':self.cmd_line, 'vtk_vtk':self.vtk_vtk}
        """
        anag_utils.funcTrace()
        result = {}
        for item in obj_names:
            result[item] = self.__dict__[item]
        return result


    def _vtkUpdate(self ):
        anag_utils.funcTrace()
        for item in self.init_data:
            obj = self.__dict__[item[0]]
            if obj:
                obj._refresh()


    def vtkvtkNewHDF5NotifierHandler( self, filename, state_file ):
        """
        Triggers the process of loading a new hdf5 file.  The notifier is in
        vtk_vtk because that's the only class instance that we don't destroy
        upon loading a new hdf5.  Otherwise, vtk_vtk is a weird place to put
        such a notifier.

        Arg state_file, if not None, indicates a state file we must apply
        to the newly-loaded hdf5.
        """
        anag_utils.funcTrace()
        self._cleanupClasses()
        self._makeClasses( skip_vtkvtk=1 )
        self.initAPI()
        self.vtk_data.loadHDF5( filename )
        if state_file:
            self.misc.restoreState( state_file )


#The following line is a directive for ../doc/make_doc.sh:
#Cut to here for pydoc.  Do not delete this line!!

g_decls.moduleAudit()
