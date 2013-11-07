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

# File: notifier.py
# Author: TDSternberg
# Created: 8/21/01

import anag_utils
import types

class Notifier:
    """
    Wrapper for a variable that, when modified, invokes callback functions that
    (normally external) objects have registered with it.  When anyone calls
    Notifier.set(), the callbacks are all invoked.  The callbacks are passed
    two arguments: (1) self, i.e. a reference to the Notifier whose set()
    method has been called, and (2) an arbitrary object that was passed to
    set() -- this is to help registering objects avoid infinite recursive loops
    (as when the callback modifies a Notifier that calls that callback).
    """

    def __init__( self, x ):
        """ Arg x is the value that get() would return. """
        anag_utils.funcTrace()
        self.decls = anag_utils.Declarations( 'decls',
            'rep',
            'callbacks',
            'unremovable_callbacks',
            instance=self
            )
        self.rep = x  # The thing that the getter gets, and the setter sets.
        self.callbacks = []  # The functions we call from set().
        self.unremovable_callbacks = []
    
    def get( self ):
        #anag_utils.funcTrace()
        return self.rep

    def set( self, x, **kw ):
        """
        Arg kw can have the following keys:
          extra_info -- anything you like.
          suppress_callbacks -- useful during system initialization.
        """
        anag_utils.funcTrace()

        self.rep = x
        if kw.has_key('extra_info'): extra_info = kw['extra_info']
        else:                        extra_info = None
        if kw.has_key('suppress_callbacks'):
              suppress_callbacks = kw['suppress_callbacks']
        else: suppress_callbacks = 0

        if suppress_callbacks == 0:
            for c in self.callbacks:
                apply( c, (x, extra_info))


    def addCallback( self, callback, unremovable=None ):
        #anag_utils.funcTrace()
        if not callback in self.callbacks:
            self.callbacks.append( callback )
        if unremovable and (not callback in self.unremovable_callbacks):
            self.unremovable_callbacks.append( callback )


    def removeAllCallbacks( self, registered_by=None ):
        """
        Happens when we restore a saved state.  Prevents callbacks to lost
        references.
        """
        anag_utils.funcTrace()
        if not registered_by:
            self.callbacks = self.unremovable_callbacks[:]
        else:
            temp = self.callbacks[:]
            for c in temp:
                anag_utils.info( "callback=", c )
                anag_utils.info( "dir(callback)=", dir(c) )
                if( (registered_by == c.im_self)
                and (not c in self.unremovable_callbacks) ):
                    self.removeCallback( c )


    def removeCallback( self, c ):
        anag_utils.funcTrace()
        if (c in self.callbacks) and (not c in self.unremovable_callbacks):
            self.callbacks.remove( c )
