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

# File: dialog_inventory.py
# Author: TDSternberg
# Created: 10/18/02

import time
import anag_utils
from self_control import SelfControl

class DialogInventory( SelfControl ):
    def __init__( self, dep_dict ):
        """
        Keep track of dialogs.  Open and close all of them at once.
        Every dialog we construct, we register with the (single) instance of
        this class.

        Persistence of dialog open|closed state is still handled by each
        anag_megawidgets.SelfDescribingDialog, in its self.show_dialog.  This
        class keeps track of the dialogs by their memory references only, and
        those aren't good across sessions.
        """
        anag_utils.funcTrace()

        instance_data = [
            {'name':'dialogs', 'initval':[]},  # All of them
            {'name':'previously_open_dialogs', 'initval':[]}
                # Dialogs that were open before the last call to closeDialogs().
        ]
        SelfControl.__init__( self, dep_dict, instance_data )

        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls", "instance_data",
            "dep_dict" )
        decls.memberFunctionAudit(self)

    def _refresh( self ):
        anag_utils.funcTrace()

    def cleanup( self ):
        anag_utils.funcTrace()
        self.closeAllDialogs()

    def registerDialog( self, dialog ):
        """
        Arg dialog can be anything, actually, that has show() and withdraw()
        methods.
        """
        anag_utils.funcTrace()
        self.dialogs.append( dialog )


    def unregisterDialog( self, dialog ):
        """
        Extremely important to have every child of SelfDescribingDialog call
        this on itself, in its cleanup() method.
        """
        anag_utils.funcTrace()
        self.dialogs.remove( dialog )


    def closeAllDialogs( self, time_delay=0 ):
        anag_utils.funcTrace()
        self.previously_open_dialogs = []
        for d in self.dialogs:
            try:  # Gotta be prepared for dead (ref-count deleted) dialogs.
                if d.winfo_ismapped() == 1:
                    self.previously_open_dialogs.append( d )
                try:
                    d.withdrawGUI()
                    # That's the correct way to close a SelfDescribingDialog,
                    # but not all the things we register in the dialog inventory
                    # is of that class; some a mere Tkinter.Dialogs.  So for
                    # them we have...
                except:
                    d.withdraw()
                time.sleep( time_delay )
            except:
                anag_utils.excepthook()
                anag_utils.info( "Something wrong with", str(d) )

        
    def reopenDialogs( self ):
        """
        Reopens the dialogs that were open the last time self.closeAllDialogs()
        was called.  Note this means that if you call closeAllDialogs() twice
        in a row, then reopenDialogs() will reopen nothing.
        """
        anag_utils.funcTrace()
        for d in self.previously_open_dialogs:
            try:
                d.show()
            except:
                anag_utils.info( "Something wrong with", str(d) )


    def flashDialogs( self ):
        """
        Briefly close, then reopen, all dialogs.  Helps users find the dialogs
        belonging to a particular ChomboVis process.
        """
        anag_utils.funcTrace()
        self.closeAllDialogs(time_delay=0.1)
        time.sleep(0.5)
        self.reopenDialogs()


    def registerForRestoration(self):
        """
        Overrides version in SelfControl.  We want a no-op here, as
        dialog_inventory is below saved_states in the class "hierarchy".
        """
        anag_utils.funcTrace()
