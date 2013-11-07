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

# File: self_control.py
# Author: TDSternberg
# Created: 8/19/01

""" 
Inserts instance variables (self.*) and getters and setters for them, into
an arbitrary class.  The metadata -- the variable names, whether we want a
setter, etc -- is specified in a list of dictionaries.  See class SelfControl
(below) for more comments.

Sorry about the many violations here of the 80-chars-per-line rule.  It was for
a good cause.
"""

import string
import sys
import types
import anag_utils
from notifier import Notifier

g_multi_instance_qualifier_counter = -1
g_decls = anag_utils.Declarations(
    'g_decls', 'g_multi_instance_qualifier_counter' )

def resetMultiInstanceQualifier():
    """ For use upon loading a new hdf5 file. """
    global g_multi_instance_qualifier_counter
    g_multi_instance_qualifier_counter = -1

def generateMultiInstanceQualifier():
    """ See below for explanation of the role of multi_instance_qualifier. """
    global g_multi_instance_qualifier_counter
    g_multi_instance_qualifier_counter = g_multi_instance_qualifier_counter + 1
    return g_multi_instance_qualifier_counter


class SelfControl:
    """
    This is a base class of all singleton classes in the chombovis project.

    Abstract base class.  Subclasses are able to refresh themselves to a
    consistent state after a state-restore operation, or (requiring more
    radical work) the loading of a new hdf5 file.

    It generates accessor functions and, together with SavedStates,  manages
    persistence.  For more see the comments under the __init__ function, and
    at the module level.
    """

    def __init__(self, dep_dict, metadata ):
        """
        Arg dep_dict is a dictionary whose keys are the names of instance
        variables we want to create, and whose values are the values we will
        assign to those keys.

        Arg metadata is a list of dictionaries.  Each dictionary pertains to a
        variable and has the following keys:
            'name', the variable's name;
            'get',  1 if we want a getter function generated, 2 if we want to
                    provide our own handwritten getter (so we can cause some
                    custom side effects), 0 if we don't want a getter and don't
                    plan to write one either (i.e. the variable is "private"
                    to its class);
            'set',  like 'get', but for setter functions;
            'initval', initial value;
            'trace',   1 if we want funcTrace(1) inserted in the getter & setter
            'save': If we want to save this variable upon shutdown, and load
                    it upon restart.  Legal values are 1, 2, 4 and their bitwise
                    OR's (i.e. 3, 5 and 7).  1 means the variable is a non-GUI
                    state variable.  2 means it's GUI-specific, e.g. whether
                    a dialog is open, or whether an EntryScale is in scale mode
                    or entry mode.  4 means don't restore if we're in (2D)
                    reslice mode and we're restoring a 3D state.  See
                    SavedStates.loadState().
            'notify'   1 if this is a Notifier.

        Only the 'name' key is mandatory; the others are optional.  
        No 'initval' is interpreted as a 'None'.  All the others, in their
        absence, are interpreted as 0.

        You don't need to declare these instance variables in the usual
        anag_utils.Declarations object; anag_utils._instanceAudit() won't mind
        as long as you call SelfControl.__init__ before you construct
        self.decls.
        """

        anag_utils.funcTrace()

#       NOTE: You don't need self.decls here, because this class is always
#       used as a superclass.  The self.decls will come from the derived class.
#       self.decls = anag_utils.Declarations( "decls","dep_dict", "metadata",
#           instance=self )

        self.metadata = metadata
        self.dep_dict = dep_dict

        self.registered_callbacks = [] # See self.registerCallback():

        self.multi_instance_qualifier = ''
            # Define unique values, when there's
            # more than one instance of a class.  You need the qualifiers then
            # so the state dictionary will have unique keys.

        # Create instances corresponding to the items in arg dep_dict.
        cdict = self.__dict__
        for k in dep_dict.keys():
            cdict[k] = dep_dict[k]

        # Generate getter and setter functions for the variables mentioned in
        # arg metadata:
        self._makeAccessors()

        # Register with the Notifiers that control the restore routine.
        self.registerForRestoration()


    def _loadedNewHDF5( self, x, extra_info=None ):
        """
        Callback for Notifier num_hdf5s_loaded.
        
        Delegates to several functions that must be overloaded in every
        subclass.

        WARNING: Do not override this function.
        """
        anag_utils.funcTrace()

        self._initForFirstHDF5()
        self._refresh()


    def _initForFirstHDF5( self ):
        anag_utils.info( "str(self.__class__) =", str(self.__class__))
        anag_utils.fatal(
        """
        You must override this function in every subclass of SelfControl.

        This function performs initialization necessary upon loading the first
        hdf5 file -- set up vtk pipelines, whatever.
        But it only fires if saved_states appears in the subclass' dep_dict.
        """
        )


    def _refresh( self ):
        anag_utils.fatal(
        """
        You must override this function in every subclass of SelfControl.

        This function gets called after restoring an old state.  Before it's
        called, we restore the value of all the 'save':* variables (except
        the 'save':4 variables if we're in reslice mode and we're restoring a
        3D state).
        So the role of _refresh() is to do whatever else it takes to bring the
        system back to a consistent state (e.g. setting GUI controls so they
        reflect the corresponding values in the vtk_*.py layer).
        """
        )


    def cleanup( self ):
        anag_utils.info( "str(self.__class__) =", str(self.__class__))
        anag_utils.fatal(
        """
        You must override this function in every subclass of SelfControl.

        This function releases resources and is called just before we try to
        load a new hdf5 file.  Examples of resources to release: close Tkinter
        widgets, remove Vtk actors.  No need, though, to set all self.*
        variables to None or unregister callbacks, as the call to this function
        is followed immediatesly by calls to unregisterCallbacks() and
        zeroSelfVariables().
        """
        )


    def _resetSelfVariables( self ):
        """
        Reset all the self.* variables to either "None" or whatever their
        'initval' field was in the metadata.
        """
        anag_utils.funcTrace()
        for line in self.metadata:
            if line['initval'] != None:
                if line['notify'] == 1:
                    exec( 'self.' + line['name'] + '.set('
                        + line['initval'] + ')' )
                else:
                    exec( 'self.' + line['name'] + '=' + line['initval'] )
            else:
                if line['notify'] == 1:
                    exit( 'self.' + line['name'] + '.set(None)' )
                else:
                    exec( 'self.' + line['name'] + '=' + 'None' )


    def zeroSelfVariables( self ):
        """
        Set all the self.* variables to None.
        """
        anag_utils.funcTrace()
        for line in self.metadata:
            exec( 'self.' + line['name'] + '=' + 'None' )


    def registerForRestoration(self):
        """
        Register callbacks with the variables that control the restore routine.
        """
        anag_utils.funcTrace()

        # Bail out if we're in a class (at this writing only CmdLine is like
        # that) which is lower-level than SavedStates:
        if str(self.__class__) != 'saved_states.SavedStates':
            try:
                foo = self.saved_states
            except:
#              anag_utils.info( "No self.saved_states here:", self.__class__,
#                  "so don't expect any persistence for this class." )
#              anag_utils.excepthook()
               return

        if str(self.__class__) != 'saved_states.SavedStates':
            self.registerCallback(
                self.saved_states.getNotifierGoSave(),
                self.saveClassState )
            self.registerCallback(
                self.saved_states.getNotifierStateDict(),
                self.restoreClassState )
            self.registerCallback(
                self.saved_states.getNotifierNumHDF5sLoaded(),
                self._loadedNewHDF5 )
        else:
            self.registerCallback(
                self.getNotifierGoSave(),
                self.saveClassState )
            self.registerCallback(
                self.getNotifierStateDict(),
                self.restoreClassState )
            self.registerCallback(
                self.getNotifierNumHDF5sLoaded(),
                self._loadedNewHDF5 )

        assert( str(self.__class__) != 'self_control.SelfControl' )


    def saveClassState( self, dummy, one_class_name ):
        """
        Save to a dictionary all instance variables designated 'save':1.
        Then insert that dictionary into the saved_states class's master
        dictionary.

        Arg dummy is the value the Notifier was set to, but we don't care what
        that is; we only care that it was set.

        Arg one_class_name, when not None, is used when we want to save the
        state of just one class.  It's indicated as a string -- the one you'd
        from a str(self.__class__) in the respective class.
        """
        decls = anag_utils.Declarations( "self", "dummy", "one_class_name",
            "state_dict", "line", "obj" )

        #This is for when this is called upon the registerCallback(...GoSave)
        #statement:
        if dummy == None:
            return

        if one_class_name:
            if one_class_name != str(self.__class__):
                return            

        self.preStateSaveHook()

        state_dict = {}
        for line in self.metadata:
            if line['save'] != 0:
                state_dict[line['name']] =\
                    anag_utils.deepCopy( self.__dict__[line['name']] )
        if state_dict == {}:
            return  # There were no 'save':1 variables, so no need to even make
                    # an entry in the state dictionary.

        if str(self.__class__) != 'saved_states.SavedStates':
            obj = self.saved_states
        else:
            obj = self
        obj.saveClassDict(
            str(self.__class__) + '_' + str(self.multi_instance_qualifier),
            state_dict )

        #decls.memberFunctionAudit(self)


    def preStateSaveHook( self ):
        """
        Override this if, in some base class, you need to take some special
        action prior to saving the state.
        """
        anag_utils.funcTrace()
        pass


    def disarmNotifiers( self ):
        """
        Used when preparing to restore a saved state.
        Disarm all Notifier callbacks -- prevents callbacks to lost
        references.  The callbacks will be reinstated in _refresh().
        Note these are callbacks registered with notifiers belonging to this
        class.  Compare to self.unregisterCallbacks().
        """
        anag_utils.funcTrace()
        for line in self.metadata:
            if line['notify'] == 1:
                #anag_utils.info( "notifier name=", line['name'] )
                var = self.__dict__[line['name']]
                var.removeAllCallbacks()


    def registerCallback( self, notifier, callback ):
        """
        Register arg callback with arg notifier.  Arg callback can be the name
        of any function, or a lambda.  Arg notifier must be a Notifier.  Don't
        refer to the notifier by name, though as that would violate our
        encapsulation rules.  Instead, use its associated getNotifier function.

        This is the only approved way for an object to register a callback to a
        notifier.
        """
        anag_utils.funcTrace()
        notifier.addCallback( callback )
        if not (notifier,callback) in self.registered_callbacks:
            self.registered_callbacks.append( (notifier,callback) )


    def unregisterCallback( self, notifier, callback ):
        """ If callback isn't there, then no-op. """
        notifier.removeCallback( callback )


    def unregisterCallbacks( self ):
        """
        Unregister any callbacks this class has registered with other classes.
        The classes we look at are the ones mentioned in self.dep_dict.        
        """
        anag_utils.funcTrace()
        for n_c in self.registered_callbacks:
            notifier = n_c[0]
            callback = n_c[1]
            notifier.removeCallback( callback )


    def restoreClassState( self, master_dict, extra_info ):
        """
        From a master dictionary whose keys are class names and whose values
        are class-specific state dictionaries such as those created by
        saveClassState(), pick out the class-specific dictionary for this class
        and from it set the values of all the self.metadata variables with
        'save':* (except the ones with 'save':4 if we're in reslice mode and
        we're restoring a 3D state).

        If arg extra_info is 1, then we restore only GUI-specific state (e.g.
        whether dialogs were open, whether EntryScales were in scale or entry
        mode, etc.)  In other words, variables that were 'save':2 or 'save':6.

        This is the callback function for a Notifier in saved_states.
        """
        anag_utils.funcTrace()
        decls = anag_utils.Declarations( 'self', 'master_dict', 'extra_info',
            'gui_only',
            'local_dict', 'var', 'cmd_str', 'save1_list', 'save2_list',
            'save4_list' )
        gui_only = extra_info

        # Disarm all Notifier callbacks -- prevents callbacks to lost
        # references.  The callbacks will be reinstated in refresh():
        self.disarmNotifiers()

        instance_key = str(self.__class__) + '_' \
                     + str(self.multi_instance_qualifier)
        if master_dict.has_key( instance_key ):
            local_dict = master_dict[ instance_key ]
        else:
            local_dict = {}

        #anag_utils.info( 'Restoring state of ', str(self.__class__))
        if str(self.__class__) == 'saved_states.SavedStates':
            saved_states_instance = self
        else:
            saved_states_instance = self.saved_states

        # Classify the variables according to their 'save':* code.  (A variable
        # may appear on more than one list; the codes are used as OR-able
        # bitmaps.)
        save1_list = []  # Non-GUI variables
        save2_list = []  # GUI-only variables
        save4_list = []  # 3D variables not to restore into reslice mode.

        for line in self.metadata:
            if line['save'] & 1 == 1:
                save1_list.append( line['name'] )
            if line['save'] & 2 == 2:
                save2_list.append( line['name'] )
            if line['save'] & 4 == 4:
                save4_list.append( line['name'] )
        #anag_utils.info( "save4_list=", save4_list )

        for var in local_dict.keys():

            # Exclude variables saved in 3D mode, if we are now in reslice mode.
            if( (var in save4_list) 
            and (saved_states_instance.getPrerestoreIsResliceMode()==1)
            and (saved_states_instance.getIs3DMode()==1) ):
                anag_utils.info( "Save-4 variable, not restoring." )
                continue

            # Exclude non-GUI variables, if arg gui_only == 1.
            if( (var in save1_list)
            and (gui_only == 1 ) ):
                #anag_utils.info( "Non-GUI variable -- not restoring." )
                continue

            # If here, go ahead and restore.
            if not (    (var in save1_list)
                     or (var in save2_list)
                     or (var in save4_list) ):
                anag_utils.info( "Unknown variable in state file:", var )
                # We'll get this message when we eliminate or rename a variable.
                # Don't want to see a warning each time.
            else:
                cmd_str = 'def tempsetter(x,self=self):                   \n' +\
                          '    self.' + var + ' = anag_utils.deepCopy(x)  \n'
                exec cmd_str
                tempsetter( local_dict[var] )
                # The deepCopy is necessary because otherwise when we modify the
                # (restored) state, we'd also be modifying the state from which
                # we restored.

        # Finish up the restoration with a (necessarily custom-written) function
        # that brings everything else about the class to a consistent state.
        # For example, in the GUI layer this means making the widgets reflect
        # the state of the corresponding vtk_* class.
        # That extra function should be called '_refresh'.
        # Also extremely important: _refresh() should re-issue the
        # registerCallback() calls.
        self._refresh()

        # Reregister with the Notifiers that run the restore routine.
        self.registerForRestoration()


    def setMultiInstanceQualifier( self, x ):
        self.multi_instance_qualifier = x


    def notFromHereOrVtkData( self, originating_class, self_class_name=None ):
        """
        Called in updateAccum* functions, to avoid unnecessary accumulations.
        The way it works is that pipelines that need to trigger accumulation
        pass their class name to vtk_data.vtkUpdate().  They usually give
        str(self.__class__) for their class name, but vtk_slice passes 'slice'
        instead because it's the subclasses that trigger accumulation and we
        don't want their names to be known outside vtk_slice.py.
    
        Optional arg self_class_name is for use when the class name is going
        to be anything other than str(self.__class__).
        """
        anag_utils.funcTrace()

        if not self_class_name:
            self_class_name = str(self.__class__)

        if( originating_class != self_class_name
        and (originating_class.find( 'VtkData' ) == -1) ):
            return 1
        else:
            return 0


    def genericGetter( self, cdict, var_name, is_notifier, subscripts=None ):
        if subscripts and (type(subscripts) == types.ListType):
            assert( len(subscripts) > 0 )
            munged_args = string.join(map(str,subscripts),'@')
            if is_notifier: return cdict[var_name][munged_args].get()
            else:           return cdict[var_name][munged_args]
        else:
            if is_notifier: return cdict[var_name].get()
            else:           return cdict[var_name]


    def genericSetter( self, cdict, var_name, is_notifier, 
                       x, subscripts=None, **kw ):
        if subscripts and (type(subscripts) == types.ListType):
            try:
                assert( len(subscripts) > 0 )
                munged_args = string.join(map(str,subscripts),'@')
                if is_notifier:
                    cdict[var_name][munged_args].set( x, **kw )
                else:
                    cdict[var_name][munged_args] = x
            except:
                anag_utils.funcTrace(1)
                anag_utils.excepthook()
                sys.exit(0)
        else:
            try:
                if is_notifier:
                    cdict[var_name].set( x, **kw )
                else:
                    cdict[var_name] = x
            except:
                anag_utils.funcTrace(1)
                anag_utils.excepthook()
                sys.exit(0)


    def genericNotifierGetter( self, cdict, var_name ):
        return cdict[var_name]


    def _makeAccessors( self ):
        """
        Initialize the variables, and create getter and setter functions as
        indicated in the metadata.

        Accessors for dictionary-type variables:
        If the getter is called with a list ([]) of arguments, those are
        considered subscripts of a dictionary element.  For example,
        getFoo([2,3]) returns the [2][3] element.  But don't ever say
        getFoo([]); if there are no subscripts, just say getFoo().
        For setters, the convention is setFoo(x,[2,3]) to set the [2][3] element
        to the value x.

        Implementation note: In reality, we store such
        elements in a linear dictionary; if there are multiple subscripts
        we munge them together as there's no advantage to maintaining a
        multidimensional dictionary as our internal representation.
        We munge by sticking a '@' between the arguments.  So don't ever use
        arguments with '@' in their values!  (There's an assertion in the setter
        that checks for this.)

        Add the accessor functions to self.__class__.__dict__.
        """
        anag_utils.funcTrace()
#       decls = anag_utils.Declarations( "decls", "line", "cmd",
#           "notifier_get", "notifier_set", "cdict" )
    
        cdict = self.__dict__

        for line in self.metadata:
        
            if not line.has_key('get'): line['get'] = 0
            if not line.has_key('set'): line['set'] = 0
            if not line.has_key('initval'): line['initval'] = None
            if not line.has_key('trace'): line['trace'] = 0
            if not line.has_key('save'): line['save'] = 0
            if not line.has_key('notify'): line['notify'] = 0

            if line['notify'] == 1:
                cdict[line['name']]  = Notifier( line['initval'] )
                notifier_get = '.get()'
                notifier_set = '.set(x, **notifier_kw)'
            else:
                cdict[line['name']] = line['initval']
                #FIXME: no more need to put initial values in quotes -- will
                #work faster without them since we'll avoid the "eval".
                notifier_get = ''
                notifier_set = ' = x'
    
            if line['get'] == 1:
                if line['notify'] == 1:
                    func = lambda subscripts=None, self=self,\
                                 cdict=cdict, var_name=line['name']:\
                               self.genericGetter( cdict, var_name, 1,
                                                   subscripts )
                else:
                    func = lambda subscripts=None, self=self,\
                                  cdict=cdict, var_name=line['name']:\
                               self.genericGetter( cdict, var_name, None,
                                                   subscripts )
                cdict[self._getterName(line['name'])] = func


            # If it's a Notifier, we make a setter even if 'set' is 0.
            # Indeed, use {'set':0, 'notify':1} to express the idea that the
            # setter is to be "private".
            if line['set'] == 1 or (line['notify'] == 1 and line['set'] != 2):
                if line['notify'] == 1:
                    func = lambda x, subscripts=None, self=self,\
                        cdict=cdict, var_name=line['name'], **kw:\
                            self.genericSetter(
                                cdict, var_name, 1, x, subscripts, **kw )
                else:
                    func = lambda x, subscripts=None, self=self,\
                               cdict=cdict, var_name=line['name']:\
                               self.genericSetter( 
                                   cdict, var_name, None, x, subscripts )
                cdict[self._setterName(line['name'])] = func

            # Generate getNotifier* functions for Notifiers.
            if line['notify'] == 1:
                func = lambda cdict=cdict, self=self, var_name=line['name']:\
                    self.genericNotifierGetter( cdict, var_name )
                cdict[self._getNotifierName(line['name'])] = func


#       decls.memberFunctionAudit(self)


    #
    # Utility functions
    #
    def _setterName( self, var_name ):
        """
        Produce the name of a setter function, using our convention that
        the setter of foo_bar is called setFooBar
        """
        anag_utils.funcTrace()
        return "set" + anag_utils.upcaseStyle( var_name )
    
    def _getterName( self, var_name ):
        """
        Produce the name of a getter function, using our convention that
        the getter of foo_bar is called getFooBar
        """
        anag_utils.funcTrace()
        return "get" + anag_utils.upcaseStyle( var_name )

    def _getNotifierName( self, var_name ):
        """
        Produce the name of a getNotifier function, using our convention that
        that function is called getNotifierFooBar
        """
        anag_utils.funcTrace()
        return "getNotifier" + anag_utils.upcaseStyle( var_name )
            
    
    def unitTest(self):
        pass
        #anag_utils.warning("You should redefine this function in every class")
