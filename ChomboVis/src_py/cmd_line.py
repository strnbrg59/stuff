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

# File: cmd_line.py
# Author: TDSternberg
# Created: 8/20/01

"""
Interpret the command line, packing it into an object with fields that
correspond to the various command-line options.
"""

import anag_utils
import algorithms
from self_control import SelfControl
import string
import sys

g_decls = anag_utils.Declarations( 'g_decls' )

class CmdLine( SelfControl ):
    def __init__(self, dep_dict):
        """
        Interpret the command line, packing it into an object with fields that
        correspond to the various command-line options.
        Usage: see self.usage_message.
        """
        anag_utils.funcTrace()

        #
        # Variable "declarations".
        # Note you can't use 'save':1 variables in this class, because CmdLine
        # is lower-level than SavedStates.
        #
        SelfControl.__init__( self, dep_dict=dep_dict, metadata =
          [
            # Each variable corresponds to a command-line option.
            { 'name':'infile', 'get':1, 'set':1, 'initval':None },
            { 'name':'slice_axis',    'get':1, 'initval':None },
            { 'name':'axis_position', 'get':1, 'initval':None },
            { 'name':'debug_level', 'get':0, 'initval':None},
                # Access to debug level is through anag_utils directly.
            { 'name':'usage_message'},
            { 'name':'user_script', 'get':1, 'initval':None},
            { 'name':'state_file', 'get':1, 'initval':None},
            { 'name':'ignore_rc', 'get':1, 'initval':None},
            { 'name':'cmd', 'get':1, 'initval':None},
            { 'name':'off_screen', 'get':1, 'initval':None},
            { 'name':'mesa', 'get':1, 'initval':None},

            # Below here, command-line options not recommended for users.
            { 'name':'test_class', 'get':1, 'initval':None},
                # name of class to test.
            { 'name':'test_mode', 'get':1}, #'min','med', or 'max' -- determines
                # how much of the rest of the class hierarchy to construct
                # before running unit tests.
            { 'name':'python_i_mode', 'get':1, 'initval':None},
            { 'name':'no_vtk', 'get':1, 'initval':None},
            { 'name':'texture', 'get':1, 'initval':1},
            { 'name':'slices_off', 'get':1, 'initval':0},
            { 'name':'use_render_widget', 'get':1, 'initval':1},
            { 'name':'iso_eb', 'get':1, 'initval':0}
          ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        self.usage_message = (
            "-------------------------------------------------------\n" +
            "Usage: chombovis [-i] [<option1>=<value1>[<option2>=<value2>[...]]] [something.hdf5]\n" +
            "Legal options:\n" +
            "  -i...leaves you at Python prompt (like 'python -i')\n"
            "  slice_axis...axis perpendicular to a desired 2D slice\n" +
            "  axis_position...position along slice_axis (required if slice_axis is used)\n" +
            "  ignore_rc...1 => don't load .chombovisrc (default=0)\n" +
            "  state_file...saved state of a previous ChomboVis session, loaded\n" +
            "               before execution of any user_script.\n" +
            "  user_script...see examples directory\n" +
            "  cmd...Python command to execute (no spaces allowed!)\n"
            "  off_screen...1 => render off-screen, rather than to monitor\n"
            "  texture...0 => don't use texture mapping (default=1)\n"
            "  slices_off...1 => come up with no slices displayed (default=0)\n"
            "  debug_level...0=nothing, 1=fatal, 2=error, 3=warning, 4=info, 5=trace (default=3)\n"
            "  use_render_widget...0 => avoids VTK bug that results in segfault on some systems (default=1) \n"
            "  iso_eb...1 => brings up new isocontour-based EB functionality even when there's a fraction-0 component (default=0)\n"
            "  --help...prints this message\n"
            "-------------------------------------------------------\n"
            )

        if len(sys.argv) > 1:
            self._gatherCmdLineOptions( self._crunch( sys.argv[1:] ))

        if self.debug_level != None:
            anag_utils.setDebugLevel( self.debug_level )

        if self.off_screen:
            self.off_screen = int(self.off_screen)
            self.mesa = 1

        if self.mesa:
            self.mesa = int(self.mesa)
            if self.mesa == 1:
                self.use_render_widget = 0
                self.off_screen = 1
                # Cuz vtkTkRenderWidget.GetRenderWindow() doesn't return a
                # vtkMesaRenderWindow but a vtkRenderWindow.

        # From here on, everybody goes to anag_utils directly, if they want to
        # know the debug level.


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()


    def cleanup( self ):
        anag_utils.funcTrace()
            

    def _gatherCmdLineOptions( self, argvee ):
        """ Arg argvee is argv[1:] after _crunch(). """
        anag_utils.funcTrace()
        for item in argvee:
            try:
                key, value = item.split( '=' )
                
                x = self.__dict__[key]
                # That will have thrown an exception, if self.key doesn't
                # exist, i.e. illegal command-line option.
                # So if we're here, things are looking good.  (But we still
                # haven't checked the reasonableness of the value.)

                # We want to interpret numbers as numbers:
                try:
                    x = float(value)
                    i = int(value)
                    if algorithms.floatEquals(x,i,max(x/1E10,1E-10)):
                        self.__dict__[key] = i
                    else:
                        self.__dict__[key] = x
                except:
                    self.__dict__[key] = value

            except:
                sys.stderr.write( self.usage_message )
                sys.exit(1)

    def _crunch( self, argvee ):
        """
        Arg argvee is argv[1:]

        Normalize cmd-line, if possible, by removing blanks from around
        '=' signs.
        """

        #FIXME: If there are spaces within cmd-line args (especially likely
        #within cmd arg, e.g. cmd='c.grid.setDetail("All cells")' ), you have
        #to handle that (and not just split on the spaces).

        # Turn argv[] into one string.
        str = string.join(argvee)

        # Replace all ' =' and '= ' into just plain '=':
        str = string.replace(string.replace(str, ' =', '='), '= ', '=')

        result = str.split(' ')
        return result


    def overrideCommandLine( self, dict ):
        """
        Elements of arg dict are the names of instance variables and the values
        we'd like to set those instance variables to.
        This is used when starting a new ChomboVis.
        """
        anag_utils.funcTrace()
        for k in dict.keys():
            self.__dict__[k] = dict[k]

    def unitTest( self ):
        pass


    def registerForRestoration(self):
        """
        Overrides version in SelfControl.  We want a no-op here, as cmd_line
        is below saved_states in the class "hierarchy".
        """
        anag_utils.funcTrace()


g_decls.moduleAudit()
