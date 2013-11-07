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

# File: main.py
# Author: TDSternberg
# Created: 9/5/01

"""
Entry point into ChomboVis

If user_script option appeared on the command line, imports the indicated user
script.  Otherwise just starts ChomboVis.
"""

import anag_utils
#g_imports_timer = anag_utils.Timer( label='main imports:' )
#g_imports_timer.on()

import vtkpython
import libVTKChomboPython
import os
import os.path
import sys
import visualizable_dataset
import box_layout_data

#g_imports_timer.stop()  # Takes 0.31, of which 0.30 is on vtkpython & libVTK...

import chombovis


# This "c" variable is the latest ChomboVis instance.  We violate the rule of
# using the "g_" prefix, for the sake of making it easier for users to use the
# Python prompt.
c = None
sys.setrecursionlimit(50000)

class Main:
    def __init__( self ):
        pass

    def go( self, virtual_command_line={} ):
        global c
        decls = anag_utils.Declarations( 'decls', 'rc_file', 'rc_path',
            'state_file', 'user_script', 'sys_path_bak' )

        self.c = chombovis.new( virtual_command_line )
        c = self.c  # c is module-global.  Convenient at the Python prompt.

        self._handleChombovisrc()
        self._handleStateFile()
        self._handleUserScript()
        self._handleCmd()
        self._handlePythonIMode()


    def _handleChombovisrc( self ):
        anag_utils.funcTrace()

        #
        # Execute .chombovisrc, if it exists, and if we're not in some sort
        # of testing mode, or in off-screen mode.
        # Look for .chombovisrc first in $PWD, then in $HOME, then in
        # $CHOMBOVIS_HOME.
        try:
            if (    ( not self.c.cmd_line.getTestClass() )
                and ( not self.c.cmd_line.getIgnoreRc() )
                and ( self.c.cmd_line.getOffScreen() != 1 ) ):
        
                rc_file = '.chombovisrc'
                if   os.path.isfile( os.environ['PWD'] + '/' + rc_file ):
                    rc_path = os.environ['PWD'] + '/' + rc_file
                elif os.path.isfile( os.environ['HOME'] + '/' + rc_file ):
                    rc_path = os.environ['HOME'] + '/' + rc_file
                elif os.path.isfile( os.environ['CHOMBOVIS_HOME'] + '/' +
                                     rc_file ):
                    rc_path = os.environ['CHOMBOVIS_HOME'] + '/' + rc_file
                else:
                    rc_path = None
        
                if rc_path:
                    (temp_rc_name, dummy) =\
                        anag_utils.g_temp_file_mgr.makeTempFile(
                            'chombovisrc', 'py', create=0 )
                    sys_path_bak = sys.path
        
                    # A Python script file can't start with a dot, but we
                    # really do want to call it '.chombovisrc'.  So we'll copy
                    # it to a temp file that ends in .py.
                    if os.path.isfile( temp_rc_name ):
                        os.unlink( temp_rc_name )
                    os.symlink( rc_path, temp_rc_name )
                    sys.path = [anag_utils.g_temp_file_mgr.getTempDir(),]\
                             + sys.path
                    __import__( os.path.basename(temp_rc_name)[:-3],
                                globals(), locals(),[])
                    os.unlink( temp_rc_name + 'c') # .pyc
                    sys.path = sys_path_bak
        except:
            anag_utils.excepthook()
        

    def _handleStateFile( self ):
        anag_utils.funcTrace()

        #
        # Load a saved state.
        #
        state_file = self.c.cmd_line.getStateFile()
        if state_file:
            self.c.misc.restoreState( state_file )


    def _handleUserScript( self ):
        """
        User script.  If there's a user_script on the command line, hand
        control over to it.
        """
        anag_utils.funcTrace()

        user_script = self.c.cmd_line.getUserScript()
        if user_script:
        
            # Sanity check.
            if user_script[-3:] != '.py':
                anag_utils.fatal( 'Your script must end in ".py"' )
        
        
            # If the user script is in a directory not on the PYTHONPATH, then
            # append to PYTHONPATH.
            # First, obtain the absolute path to the file.
            if user_script.rfind('/') != -1:
                if   user_script[0] == '/':
                    script_path = user_script[ : user_script.rfind('/') ]
                else:
                    script_path = (os.getcwd() + '/' +
                                   user_script[ : user_script.rfind('/')])
                user_script = user_script[ user_script.rfind('/')+1 : ]
            else:
                script_path = os.getcwd()
        
        
            sys_path_bak = sys.path
            sys.path = [script_path,] + sys.path
        
            exec( 'import ' + user_script[:-3] )
            sys.path = sys_path_bak
        

    def _handleCmd( self ):
        """ Execute the "cmd" cmd-line argument. """
        anag_utils.funcTrace()

        if self.c.cmd_line.getCmd():
            exec( self.c.cmd_line.getCmd() )
        
    def _handlePythonIMode( self ):
        """
        If user used "-i" cmd-line arg, then fall back into Python interpreter
        (same as "python -i").  Otherwise fall into mainloop().
        """
        anag_utils.funcTrace()

        if( (not self.c.cmd_line.getPythonIMode()) 
        and (self.c.cmd_line.getOffScreen() != 1) ):
            if c.misc:
                self.c.misc.mainloop()
            else: # This is often the case in test_class mode.
                c.vtk_vtk.getTopWindow().mainloop()                

if __name__ == '__main__':
    m = Main()
    m.go()
