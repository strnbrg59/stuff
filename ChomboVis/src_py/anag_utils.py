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

# File: anag_utils.py
# Purpose: Debug output, variable declarations, timers.
# Author: TDSternberg
# Created: 5/18/01

import os
import sys
import string
import time
import traceback
from notifier import Notifier


#################################
# debug output
#################################

# Debug levels: 0=totally silent, 1=fatal, 2=error, 3=warning, 4=info, 5=trace
# At debug levels 3 and below (see isAuditingDisables() below), we disable
# variable-declaration auditing (i.e. moduleAudit(), etc.)
g_debug_level = 3     # This is where we set the systemwide default debug level.


def setDebugLevel( level ):
    """ Set the debug level.

        Level 0 => totally silent.
        Level 1 => Only fatal() prints
        Level 2 => error() (as well as fatal()) prints.
        Level 3 => warning() (as well as error() and fatal()) prints.
        Level 4 => info() (as well as all the rest) prints.
        Level 5 => Function tracing, in addition to everything else.

        Example: setDebugLevel(2)
    """
    global g_debug_level
    if( 0 <= int(level) <= 5 ):
        g_debug_level = int(level)
    else:
        error( "Tried to set illegal debug level: " + str(level) )

def getDebugLevel():
    return g_debug_level

def isAuditingDisabled():
    if g_debug_level <= 3:
        return 1
    else:
        return 0

def printMessage( debug_level_cutoff, message ):
    """ This is the guts of fatal(), error(), warning(), and info(). """

    error_types = {1:'fatal', 2:'error', 3:'warning', 4:'info'}

    if g_debug_level >= debug_level_cutoff:
        sys.stderr.write( error_types[debug_level_cutoff] + ":" +
                          getModuleName(3) + ":" +
                          getFunctionName(3) + "():" )
        for i in message: sys.stderr.write( str(i) + " " )
        sys.stderr.write("\n")

def fatal( *message ):
    """ Prints message if debug level >=1.  Aborts program. """
    printMessage( 1, message )

    # Print the call stack if the debug level is above a certain threshold.
    # We'll take that as the same threshold that determines if variable-
    # declaration auditing is enabled.  (Even though this has nothing to do
    # with auditing; it'll just be easier to remember what to expect if we have
    # a single such threshold kicking around.)
    if not isAuditingDisabled():
        printCallStack()

    sys.exit(1)

def error( *message ):
    """ Prints message if debug level >=2.  Doesn't abort program (use fatal()
        if you want that).
    """
    printMessage( 2, message )

def warning( *message ):
    """ Prints message if debug level >=3. """
    printMessage( 3, message )

def info( *message ):
    """ Prints message if debug level >=4. """
    printMessage( 4, message )

def funcTrace( override=0, updepth=1 ):
    """ Prints name of current function, showing call-stack depth with
        indentation.  Only if g_debug_level==5 or override==1.
    """
    if( g_debug_level < 5 and override == 0 ): return
    # Don't do the functionAudit() stuff here; functionAudit calls funcTrace()!

    module_name = getModuleName(2)
    depth = len(traceback.extract_stack())

    sys.stderr.write("**"*(depth-updepth))
    sys.stderr.write( " " + module_name + ":"
        + funcArgs(1+updepth, max_chars_to_print=20) + "\n")


##################################################################
# Utilities for finding where we are on the call stack.
##################################################################

def excepthook():
    """ Prints the stack trace, same as what we get from a fatal Python
        error, except here it doesn't have to be fatal; you can call this
        in an "except:" clause.
    """
    sys.excepthook( sys.exc_info()[0],
                    sys.exc_info()[1],
                    sys.exc_info()[2])

def printCallStack():
    for l in traceback.extract_stack():
        indent=0
        sys.stderr.write( "(" )
        for m in l:
            sys.stderr.write( indent*"     " + str(m) + ", " )
            indent = indent+1
        sys.stderr.write( ")\n" )

def printModuleSymbols( module_name ):
    module = sys.modules[ module_name ]
    for sym in dir(module):
        sys.stderr.write( sym + "\n" )

def getFunctionName( uplevels ):
    """ Name of function that is arg uplevels function calls up from this
        function here.
        Thus if we want to use this function inside anag_utils.warning(),
        which in turn is meant to be called from some function func(), and we
        want this to return "func()", then arg uplevels should be 2.

        Warning.  If you call from a statement in a module, but not from a
        function per se, then the return value here will be '?'.
    """
    tb = traceback.extract_stack()
    #print "traceback=", tb
    if( 0 <= uplevels < len(tb) ):
        funcname = tb[ len(tb) - 1 - uplevels ][ 2 ]
    else: # true in unit test of this module, or if user error.
        warning( "Out of range arg 'uplevels' to getModuleName()" )
        funcname = tb[ 0 ][ 2 ]

    return funcname

def funcArgs( uplevels, max_chars_to_print=20 ):
    """ Returns a string explaining which parameters where passed to
        the calling function (*) and from which file and line number it
        was invoked.  Same comments as for func_info(). Note that
        line number information is only correct when running Python
        in non-optimized mode (i.e. without -O). Sample return string:

        'test(a=1, b=2, c=3, args=()) # called from "Tools.py":353'

        (*) uplevels indicates how far up the calling stack to look for
        the information. Default is one uplevels meaning: the calling
        function.

        By Marc-Andre Lemburg (http://groups.google.com/groups?q=Python+function+arguments+stack&hl=en&safe=off&rnum=1&selm=36C2B9D5.75D0003A%40lemburg.com)
    """
    try:
        1/0
    except:
        frame = sys.exc_info()[2].tb_frame
    for i in range(0,uplevels):
        frame = frame.f_back
    code = frame.f_code
    fname = code.co_name
    l = []
    callargs = code.co_argcount
    # XXX Uses hard coded values taken from Include/compile.h
    if code.co_flags & 0x0004: # CO_VARARGS
        callargs = callargs + 1
    if code.co_flags & 0x0008: # CO_VARKEYWORDS
        callargs = callargs + 1
    for v in code.co_varnames[:callargs]:
        arg_value = frame.f_locals[v]
        if type(arg_value) == types.FloatType:
            arg_value = '%g' % round(arg_value,4)
        try:
            arg_value = repr(arg_value)
        except:
            arg_value = '<repr-error>'

        if len(arg_value) > max_chars_to_print:
            arg_value = arg_value[:max_chars_to_print] + '...'
        if v != 'self':
            l.append('%s=%s' % (v,arg_value))
    if frame.f_back:
        where = '# called from "%s":%i' % \
                (frame.f_back.f_code.co_filename,frame.f_back.f_lineno)
    else:
        where = '# called from <toplevel>'
    del frame,code # you never know...
    #return '%s(%s) %s' % (fname,string.join(l,', '), where)
    return '%s(%s)' % (fname,string.join(l,', '))


def getModuleName( uplevels ):
    """ Name of module that is arg uplevels up from this function.
        Thus if we want to use this function inside another anag_utils function
        which in turn is meant to be called from another module, and we want
        to get the name of that module, then arg uplevels should be 2.

        Beware.  The return value could be <stdin>.  Or it could be an actual
        module name, but we passed that module(.py) on python's command line
        and so sys.modules contains __main__ and not that module name.
    """
    tb = traceback.extract_stack()
    #info( "traceback=", tb )
    if( 0 <= uplevels < len(tb) ):
        filename = tb[ len(tb) - 1 - uplevels ][ 0 ]
    else: # true in unit test of this module, or if user error.
        warning( "Out of range arg 'uplevels' to getModuleName()" )
        filename = tb[ 0 ][ 0 ]

    # Module name is filename from last '/' to next '.'
    last_slash = filename.rfind("/")
    last_dot = filename.rfind(".")
    if( last_dot == -1 ):
        module_name = filename[last_slash+1:]
    else:
        module_name = filename[last_slash+1:last_dot]
    return module_name

#################################
# Miscellaneous utilities
#################################

class Timer:
    """
    A stopwatch.
    Typical usage:

        timer = anag_utils.Timer( label='foo() time:' )
        timer.on()
        foo()
        timer.stop()

    or, a little fancier...

        timer = anag_utils.Timer( label='foo() total time:', verbose=1 )
        for i in range(0,10):
            bar()
            timer.on()
            foo()
            timer.pause()
        timer.stop()

    To avoid adding much overhead of its own, this class doesn't
    do much error checking, so you'll have to observe some rules:
    1. After the constructor, you may call on().
    2. After on(), you may call pause() or stop().
    3. After pause(), you may call on() or stop().
    4. After stop(), you may call on() (but be aware the timer starts from 0).
    """

    static_verbose = 0 # Verbose output if static_verbose==1 or self.verbose==1.

    def __init__(self, label, verbose=0 ):
        """
        Arg label is what stop() prints on the line it reports the time.
        Arg verbose=1 means, print something from on() as well.
        """
        self.label = label
        self.verbose = verbose
        self._reset()

    def on( self ):
        """
        Start, or restart, timer.  Timer takes off from last call to
        constructor, reset() or pause().
        """
        if self.verbose == 1 or Timer.static_verbose == 1:
            sys.stderr.write( 'Starting ' + self.label + ' timer...\n' )
        self.time_at_on = time.time()
        
    def pause( self, msg='' ):
        """ Stop timer but don't reset it. """
        increment = time.time() - self.time_at_on
        self.cum = self.cum + increment
        if msg != '':
            sys.stderr.write( msg + ':' + str(self.cum) + '\n' )

    def stop( self ):
        """ Stop timer and reset it. """
        self.pause( 'Time in ' + self.label )
        self._reset()

    def setStaticVerbose( self, v ):
        """
        We print verbose output if static_verbose==1 or self.verbose==1.
        """
        Timer.static_verbose = v

    def _reset( self ):
        """ Reset timer to zero. """
        self.cum = 0.0


def sed( a_str, a_from, a_to ):
    """
    Arg a_str is a string.  Replace all instances of arg a_from to a_to.
    """
    funcTrace()
    result = ""
    pieces = a_str.split(a_from)
    for item in pieces[:-1]:
        result = result + item + a_to
    if pieces[-1] != '':
        result = result + pieces[-1]
    return result


def upcaseStyle( var_name ):
    """
    Turn foo_bar into FooBar.
    """
    #funcTrace()
    decls = Declarations( "decls", "var_name",
        "result", "pos", "c"
        )
    result = var_name[0].upper()
    pos = 1
    for c in var_name[1:]:
        if var_name[pos] == '_' : pass
        elif var_name[pos-1] == '_' :
            result = result + c.upper()
        else:
            result = result + c
        pos = pos + 1

    decls.functionAudit()
    return result

def isAtomicType(x):
    """ BooleanType, IntType, LongType, FloatType, StringType or NoneType """
    t = type(x)
    if( t == types.IntType or t == types.LongType or t == types.FloatType
    or  t == types.StringType or t == types.NoneType
    or  t == type(1==1) ):  # types.BooleanType introduced in Python 2.3
        return 1
    else:
        return 0

def deepCopy( x ):
    """
    Return a copy of x, whether x is a scalar, a list, a tuple or a dictionary.
    But the leaves of x have to be scalars or strings (i.e. return 1 from
    isAtomicType(): if not, we abort.
    """
    if type(x) == types.ListType or type(x) == types.TupleType:
        result = []
        for i in x:
            result.append( deepCopy(i) )
        if type(x) == types.TupleType:
            result = tuple(result)
    elif type(x) == types.DictionaryType:
        result = {}
        for k in x.keys():
            result[k] = deepCopy(x[k])
    elif isAtomicType(x):
        result = x
    elif type(x) == types.InstanceType and x.__class__ == Notifier:
        result = Notifier( x.get() )
    else:
        fatal( "Leaves must be numerical or string types, or Notifiers. ",
               "We've hit:", str(x), ":which is ", type(x) )

    return result


def deprecator( f, deprecation_message, args ):
    """
    Print a message about this function being deprecated, and then do whatever
    f would have done.
    """
    warning( deprecation_message )
    if type(f) == types.TupleType:
        return apply( f, args )
    else:
        return apply( f, (args,) )


#################################
# class Declarations
#################################
import types

class Declarations:
    """ This is a facility for "declaring" variables and then checking that
        only the declared variables have been used.  You can do this at the
        scope of a module, or of a function.

        Usage: At the beginning of a module or a function, say...

        decls = Declarations( "decls", "var1", "var2", ... )

        where var1, var2 etc are variables we want to declare.
        Be sure to include the instance name (here, "decls").
        At the end of a module say...

        decls.moduleAudit()

        and at the end of a function say...

        decls.functionAudit().  There's also a memberFunctionAudit().

        If in the meantime you created any module-global (alternatively
        function-scope) variables other than the ones you declared, then
        moduleAudit() (alternatively functionAudit()) will abort your program.
        
        If you don't want your program aborted, set the instance variable
        self.kill_on_undeclared to 0, and the penalty for undeclared variables
        becomes a mere error message.

        Certain low debug levels (see isAuditingDisabled()) disable
        variable-declaration auditing (i.e. moduleAudit(), etc.)

        Limitations: It doesn't work in nested functions.
    """
    def __init__( self, *declared_vars, **instance ):
        """
        Arg instance is for when this Declarations object is meant for class
        instance auditing (i.e. when it's, typically, called self.decls).  In
        that case, arg instance should be "self" of the class in which we're
        constructing self.decls.  We need arg instance in order to make an
        inventory of inherited instance variables.  Thus, it's necessary to 
        construct the base classes (call their __init__ methods) before
        constructing this Declarations object.

        Arg declared_vars is the names of all the declared variables.
        """

        #funcTrace()
        if isAuditingDisabled() == 1:
            self.uninitialized = 1 # Prevents name and attribute errors that
            # would occur if the debug level is upped between the time of this
            # constructor and the time one of the audit methods is called; those
            # audit methods assume a bunch of variables have been defined --
            # variables that are defined below this return...
            return
        else:
            self.uninitialized = 0

        self.kill_on_undeclared = 1  # User can set this in self.doKill()

        module_name = getModuleName(2)
        if( not sys.modules.has_key(module_name) ):
            module_name = "__main__"

        if len(instance) == 0:
            self.preexisting_vars = dir( sys.modules[ module_name ] )
        elif len(instance) == 1:
              # We're doing instance auditing.  Be sure you've already 
              # constructed the base classes.
            #info( "instance['instance']=", instance['instance'] )
            self.preexisting_vars = dir( instance['instance'] )
            #info( "preexisting_vars=", self.preexisting_vars )
            #info( "dir(instance['instance'].__class__)=",
            #      dir(instance['instance'].__class__) )
        else: 
            fatal( "There's only one legal keyword for the **instance arg ",
                   "and it's 'instance'." )

        self.declared_vars = []
        for i in declared_vars: self.declared_vars.append( i )


    global _unboundModuleAudit
    def _unboundModuleAudit( module_name ):
        """
        Called from functionAudit() and memberFunctionAudit(), where we don't
        immediately have a Declarations object (the global declarator) to call
        moduleAudit() on.
        """
        if isAuditingDisabled() == 1:
            return

        module = sys.modules[ module_name ]
        global_declarator = getGlobalDeclarator( module )
        if global_declarator == None:
            global_declarator = module.g_decls = Declarations( "g_decls" )
        global_declarator.moduleAudit( module_name )


    def moduleAudit( self, module_name=None ):
        """ Check that all variables have been declared in the constructor.
            You can call this at the end of a module, but in a properly
            designed module it won't have any effect there because all the
            executable statements are inside functions and such.  That's why
            we call moduleAudit() from inside functionAudit() and
            memberFunctionAudit().

            Arg module_name is name of the module we want to audit.
            It should be left unset by all callers except functionAudit()
            and memberFunctionAudit() (members of this class).
        """

        #funcTrace()
        if (isAuditingDisabled() == 1) or (self.uninitialized == 1) :
            return

        # The reason we have to specify the module name, when calling from
        # functionAudit() or memberFunctionAudit(), is that otherwise we'll
        # obtain the anag_utils module.  (FIXME: Can't we just pass a higher
        # number to getModuleName()?)
        if module_name == None:
            module_name = getModuleName(2)
            if not sys.modules.has_key(module_name):
                module_name = "__main__"

        module = sys.modules[ module_name ]
        #info( "module=", str(module) )

        for symbol in dir(module):
            # Gotta do this because module_name isn't in this namespace unless
            # we import it.
            eval_str = ("type(sys.modules['" + module_name + "'].__dict__['" +
                       symbol + "'])")

            #info( "checking symbol ", symbol, ", type=", eval(eval_str) )

            if( (not symbol in self.preexisting_vars)
            and (eval(eval_str) != types.FunctionType)
            and (eval(eval_str) != types.ModuleType)
            and (eval(eval_str) != types.ClassType)
            and (symbol != "Pmw") ): # type(Pmw) is 'instance' (!)

                if( not (symbol in self.declared_vars) ):
                    if( self.kill_on_undeclared == 1 ):
                        fatal( symbol, "was not declared!" )
                    else:
                        error( symbol, "was not declared!" )


    def _instanceAudit( self, the_instance ):
        """
        Arg the_instance is an instance of some class.

        Check that all instance variables of arg the_instance have been
        declared in the constructor of its Declarations object (usually named
        self.decls but we find what it is by calling getInstanceDeclarator()).
        This method is called on that Declarations object.

        This method gets called from memberFunctionAudit().  There's really no
        good place to call it "manually".
        """
        #funcTrace()
        if (isAuditingDisabled() == 1) or (self.uninitialized == 1) :
            return

        #info( "dir(", the_instance.__class__, ")=", dir(the_instance) )
        #info( "the_instance=", the_instance )
        for sym in dir(the_instance):
            #info( "checking symbol ", sym, ", type=", type(sym) )

            # FIXME: If the_instance is an instance of Tkinter.Frame,
            # then it comes with self._name, self._w, self.children,
            # self.tk, self.master.  The Declarations constructor should first
            # make a list of pre-existing instance variables.
            if( not (sym in self.declared_vars)
            and not (sym in self.preexisting_vars) ):
                if( self.kill_on_undeclared == 1 ):
                    fatal( sym, "was not declared!" )
                else:
                    error( sym, "was not declared!" )


    def functionAudit( self ):
        """ Check that all variables have been declared in the constructor.
            This audit function is for use within a function (but not a
            member function).  Use memberFunctionAudit() for member functions,
            and moduleAudit() to audit module-global variables.

            Don't try this in a nested function; the function has to be at
            module scope.
            FIXME: Implement a version for functions nested arbitrarily deep.
        """
        #funcTrace()
        if (isAuditingDisabled() == 1) or (self.uninitialized == 1) :
            return

        function_name = getFunctionName(2)
        #info( "auditing function", function_name )

        module_name = getModuleName(2)
        if not sys.modules.has_key(module_name):
            module_name = "__main__"
        module = sys.modules[ module_name ]

        if not module.__dict__.has_key(function_name):
            module = sys.modules[ '__main__' ]


        #print "module=", module
        #print "module.__dict__.keys()=",module.__dict__.keys()
        try:
            func_object = module.__dict__[function_name]
        except:
            excepthook()
            fatal( "Can't find " + function_name + " in this module's "
                   "dictionary.  Maybe you should call memberFunctionAudit() "
                   "instead." )

        #print "module.__dict__["+function_name+"]=", func_object

        for symbol in func_object.func_code.co_varnames:
            #info( "checking symbol", symbol )
            if not symbol in self.declared_vars:
                if( self.kill_on_undeclared == 1 ):
                    fatal( symbol, "was not declared!" )
                else:
                    error( symbol, "was not declared!" )

        # Check module-global variables.
        _unboundModuleAudit( module_name )        


    global getGlobalDeclarator
    def getGlobalDeclarator( the_module ):
        """ Find and return the first (and we should hope, only) instance of the
            anag_utils.Declarations class at module scope.  That instance,
            which we usually call g_decls, is what we want to call moduleAudit()
            on, inside functionAudit() and memberFunctionAudit().

            Return None if no such symbol is found.
        """
        if isAuditingDisabled() == 1:
            return

        dikt = sys.modules[the_module.__name__].__dict__
        for symbol_name in dikt.keys():
            symbol = dikt[symbol_name]
            #info( "checking module symbol ", symbol_name )
            if type(symbol) == types.InstanceType:
                if( str(symbol.__class__) == "anag_utils.Declarations"
                or  str(symbol.__class__) == "__main__.Declarations" ):
                    return symbol

        # If there's no module-global declarator, that's ok; it just indicates
        # the author's belief that there are no module-global variables.  We'll
        # check that belief anyway, though, from functionAudit() and
        # memberFunctionAudit().

        return None


    global getInstanceDeclarator
    def getInstanceDeclarator( the_instance ):
        """ Find and return the first (and we should hope, only) instance of the
            anag_utils.Declarations class at class scope (e.g. self.decls).
            That instance is what we want to call _instanceAudit() on, inside
            memberFunctionAudit().

            Arg the_instance should be "self" for the instance whose declarator
            we want to find.

            Return None if no such symbol is found.
        """

        dikt = the_instance.__dict__
        for symbol_name in dikt.keys():
            symbol = dikt[symbol_name]
            #info( "checking instance symbol ", symbol_name )
            if type(symbol) == types.InstanceType:
                if( str(symbol.__class__) == "anag_utils.Declarations"
                or  str(symbol.__class__) == "__main__.Declarations" ):
                    return symbol

        # It's bad practice not to have a module-global declarator, so here's
        # a rap on the knuckles...
        warning( "No instance of anag_utils.Declarations found in ",
                 str(the_instance))
        return None


    def memberFunctionAudit( self, the_instance ):
        """ Check that all variables have been declared in the constructor.
            This audit function is for use within a member function.
            ModuleAudit() is for use at module scope.

            Arg the_instance is "self" where this method is called from.

            FIXME.  This doesn't work right in subclasses.  Try uncommenting
            out the calls to memberFunctionAudit() in SelfControl and
            you'll see that it fails to find base-class functions in the
            derived class.
        """
        #funcTrace()
        if (isAuditingDisabled() == 1) or (self.uninitialized == 1) :
            return

        function_name = getFunctionName(2)
        #info( "auditing member function ", the_instance + "." + function_name )

        module_name = getModuleName(2)
        if not sys.modules.has_key(module_name):
            module_name = "__main__"
        module = sys.modules[ module_name ]

        #info( "dir(the_instance.__class__)=", dir(the_instance.__class__) )
        if not the_instance.__class__.__dict__.has_key(function_name):
            #warning( "Didn't find--", function_name, "-- in dictionary of--",
            #    the_instance.__class__ )
            return
            # Maybe need to go deeper into stack to get function_name?
        else:
            func_object = the_instance.__class__.__dict__[function_name]

        for symbol in func_object.func_code.co_varnames:
            if symbol == 'self': continue
            #info( "checking symbol", symbol )
            if not symbol in self.declared_vars:
                if( self.kill_on_undeclared == 1 ):
                    fatal( symbol, "was not declared!" )
                else:
                    error( symbol, "was not declared!" )

        # Check instance variables.
        instance_declarator = getInstanceDeclarator( the_instance )
        if instance_declarator != None:
            instance_declarator._instanceAudit( the_instance )

        # Check module-global variables.
        _unboundModuleAudit( module_name )


    def doKill( self, kill_on_undeclared ):
        """ If kill_on_undeclared==1, then moduleAudit() and functionAudit()
            kill the program if there are undeclared variables.  Otherwise,
            moduleAudit() and functionAudit() just print an error.
        """
        funcTrace()
        self.kill_on_undeclared = kill_on_undeclared


def dict_inv( dict, val ):
    """
    "Inverse" dictionary lookup; return the key associated with arg value.
    """
    return dict.keys()[dict.values().index( val )]


#
# Management of temporary files.
#
class TempFileMgr:
    """
    Management of temporary files.
    At init time, we instantiate one instance of this TempFileMgr class.
    Anybody that needs a temp file asks TempFileMgr for it.  At shut-down
    TempFileMgr deletes all the files it knows about.
    """
    def __init__( self ):
        self.files = []  # Will be pairs (name,handle)
        self.pid = str(os.getpid())
        self.file_num = 0

        # Create /tmp/chombovis_$USER
        if   os.getenv( 'USER' ):
            self.tmp_dir = '/tmp/chombovis_' + os.getenv('USER')
        elif os.getenv( 'USERNAME' ):
            self.tmp_dir = '/tmp/chombovis_' + os.getenv('USERNAME')
        elif os.getenv( 'LOGNAME' ):
            self.tmp_dir = '/tmp/chombovis_' + os.getenv('LOGNAME')
        elif os.getenv( 'HOME' ):
            self.tmp_dir = '/tmp/chombovis_' +\
                os.getenv('HOME').split('/')[-1:][0]
        else:
            fatal( 'Your environment has neither USER, USERNAME, LOGNAME nor '
                   'HOME set.  Please set one of them.' )

        if not os.path.isdir( self.tmp_dir ):
            os.mkdir( self.tmp_dir )

    def __del__( self ):
        for i in self.files:
            try:
                if i[1]:
                    i[1].close()
            except:
                pass  # Fixme: how do you find out if a file is closed?
            try:
                if os: # os==None, in Python 2.1 (!)
                    os.unlink( i[0] )
            except:
                warning( "File", i[0], " has been deleted elsewhere." )


    def getTempDir( self ):
        funcTrace()
        return self.tmp_dir

    def makeTempFile( self, base_name, extension, create=0, save=0 ):
        """
        Generates a temporary file name.
        If optional arg create==1, opens file for writing.
        If optional arg save==1, doesn't delete the file when the program exits.

        Returns a pair: (file_name, file_handle).  If create==0, file_name=None.
        The file's full name will be /tmp/chombovis-$USER + base_name + pid
        + self.file_num + extension.
        Example:
        >>> tfm = anag_utils.TempFileMgr()  # at init -- don't call it again!
        >>> (fn,fh) = tfm.makeTempFile( 'foo', 'hdf5' )
        >>> print fn
        /tmp/chombovis-ted/foo_21994_3.hdf5
        >>>

        Note extension should not start with a period; we supply it.
        """
        full_name = self.tmp_dir + '/' + base_name + '_' + self.pid + '_'\
                  + str(self.file_num) + '.' + extension
        if create==1:
            handle = open( full_name, 'w' )
        else:
            handle = None
        if save == 0:
            self.files.append( (full_name,handle) )
            self.file_num += 1
        return (full_name, handle)

    
    def addMiscFile( self, file_name ):
        """
        Give TempFileMgr responsibility to delete some miscellaneous file -- one
        it didn't create.
        """
        funcTrace()
        self.files.append( (file_name, None) )

    def getTmpDir( self ):
        return self.tmp_dir


g_temp_file_mgr = TempFileMgr()

#
# Journaling
# All API functions start with a call to anag_utils.apiTrace(), which is
# somewhat like funcTrace() except it prints to the journal file.
#

class Journal:
    """
    Journaling.  Everything the user does gets journaled in a file that
    later can be used as a legal ChomboVis Python script.
    The journal file is kept in /tmp and deleted upon exit, but there's
    a menu option for "make command journal", which has the effect of
    copying the journal file to a file of the user's choosing.
    """
    def __init__( self ):
        funcTrace()

        return  # No sense creating a file, until we're ready to put useful
                # things into it.

        (self.filename, self.handle) =\
             g_temp_file_mgr.makeTempFile( 'chombovis_journal', 'py', create=1 )
        self.handle.write( 'import chombovis\n' +
                           'c=chombovis.latest()\n' )


    def write( self, cmd ):
        """
        Arg cmd should be a legal ChomboVis API command beginning with the "c."
        indicating the canonical ChomboVis instance.  
        """
        funcTrace()
        return
        self.handle.write( cmd + '\n' )

        """
        pieces = list(cmd.split('.'))
        assert( pieces[0] == 'c' )
        assert( len(pieces) > 2 )
        self_cmd = pieces[:]
        self_cmd[0] = 'self'
        self_cmd[1] = pieces[1] + '_api'
        eval( string.join(self_cmd, '.') )
        """    


    def copyTo( self, new_filename ):
        """
        Copy journal file to another file.
        """
        funcTrace()
        self.handle.flush()
        os.system( 'cp ' + self.filename + ' ' + new_filename )

g_journal = Journal()



def apiTrace( functrace=0 ):
    """
    Called from the top of every API function.  Prints, to the journal file,
    the name and arguments of the function it was called in (with a few tweaks
    to make it look the way one would invoke it in a ChomboVis Python script).

    If arg functrace==1, then trace (to stderr) always (like funcTrace(1)).
    """
    if functrace==1 or g_debug_level==5:
        funcTrace(1, updepth=2)
    
    module_name = getModuleName(2)
    depth = len(traceback.extract_stack())
    g_journal.write( 'c.' + module_name + '.' + funcArgs(2,200) )


def journalCopyTo( new_filename ):
    g_journal.copyTo( new_filename )

def journalCleanup():
    g_journal = None



#################################
# Unit test
#################################
if( __name__ == "__main__"):

    #
    # Debug logging
    #
    setDebugLevel(8)
    setDebugLevel(0)
    error("If you can see me, this module is broken")
    setDebugLevel(5)
    info("OK to see this")

    def level1(): funcTrace()
    def level2():
        funcTrace()
        level1()
    def level3():
        funcTrace()
        level2()

    level3()
    level2()
    level1()

    print "================================================="

    #
    # Declarations
    #
    g_decls = Declarations( "g_decls", "fubar", "barbaz", "fc" )
    g_decls.doKill( 0 )

    fubar = 5
    barbaz = 3.13
    gubar = 8  # undeclared

    def dummyFunc():
        pass

    class FooClass :
        def __init__(self):
            self.decls = Declarations( "decls" )
            self.decls.doKill(0)
            self.class_foo = 55

        def memberFunc(self):
            funcTrace()
            decls = Declarations( "decls", "doo" )
            decls.doKill(0)
            doo = 11
            bee = 22         # undeclared
            global g_doobee  # undeclared
            g_doobee = 33
            decls.memberFunctionAudit( self )

    # Test the function-level declarations facilities
    def testFunc():
        funcTrace()
        decls = Declarations( "decls", "func_foo", "func_bar" )
        decls.doKill(0)
        func_foo = 55
        func_bar = 66
        func_baz = 77  # undeclared
        global g_more  # undeclared
        g_more = 312
        
        decls.functionAudit()


    testFunc()

    fc = FooClass()
    fc.memberFunc()
    fc.decls._instanceAudit( fc )

    print ""
    g_decls.moduleAudit()

#   print "testFunc.func_code.co_varnames=", testFunc.func_code.co_varnames
    # This func_code.co_varnames is in Python/compile.c and, according to
    # http://groups.google.com/groups?q=python+locals()&hl=en&lr=&safe=off&rnum=8&ic=1&selm=v02130500add535a5e0fc%40%5B198.92.142.56%5D
    # is subject to change upon Guido's whim.
