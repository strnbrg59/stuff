import os
import os.path
import sys
import anag_utils

#Cut to here

'digraph_foo'
# See doc/digraph.py for an explanation for what d-i-g-r-a-p-h-f-o-o is about.

def makeModule( func_filename, extra_CFLAGS='', extra_LDFLAGS='',
                maximum_arity=10, save_cpp=0, debug_compilation=0  ):
    """
    Builds a module that can call a user-supplied C++ function.
    Imports the module and returns it.
    On failure, returns None.

    Arg func_filename is the absolute path to a C++ file that contains a
    function called F, with signature: "int F(vector<int>)", or
    "int F( vector<int>, float * )", or "int F( vector<int>, double * ), or
    "int F( vector<int>, float *, float * )" etc (in other words,
    std::vector<int> followed by an arbitrary number of 
    arguments all of which must be either float* or double* (but not a mixture
    thereof!)).  (We use boost::shared_array's everywhere else in ChomboVis,
    but here we stay with raw pointers to keep things simple for users.)

    After the call to makeModule(), you will be able to call your
    (C++) function F() from Python.  The float* or double* arguments are the 
    addresses of box data in Fortran order and correspond, on the Python side,
    to the return value of the BoxLayoutData.getFArray() function.  (You can't
    really pass a pointer from Python to C++; BoxLayoutData.getFArray() returns
    a key into an internal table of pointers.)

    Those are raw, non-const pointers; you therefore have unlimited power to do
    good or evil to your data.  In particular, it is up to you (i.e. your F()
    function) to know how many elements there are in the FArray, and what shape
    of 2D or 3D matrix of cells they correspond to.
    
    That's where the vector<int> argument comes in: it corresponds to a tuple
    of ints in Python.  Those can be any numbers but we anticipate most folks
    will use them to pass information about the dimensions of the box or boxes
    whose FArray(s) the other arguments refer to.

    You are also responsible for knowing whether the hdf5 file you are working
    on has float or double data; if it has float and you write F() with double*
    arguments, your stuff will build and Python will call F() but the result
    could be startling.  (Programmatically, you can determine the precision from
    VisualizableDataset.getPrecision().)

    Optional args extra_CFLAGS and extra_LDFLAGS are for use if func_filename
    is not self-contained.  Arg extra_CFLAGS indicates extra stuff to put on
    the compile line, while extra_LDFLAGS is for extra stuff to put on the
    link line.

    Optional arg maximum_arity need not be touched unless F() (the C++ function
    you want to wrap) takes more double* or float* arguments than the default
    value of maximum_arity.  That's not going to happen very often.  But if it 
    does, you will know it from seeing compiler errors on your screen.

    Optional arg save_cpp, when 1, makes sure that the generated C++
    file (which contains your file plus extra Python things that, all
    together, are compiled and linked into a Python module) is not
    deleted (as it and other temporary files are by default).  Set it to 1 if 
    you need to debug.  (Unfortunately, with some older Python/gcc combinations,
    those temp files don't get cleaned up however you set save_cpp.  Download
    and build the and-everything package if you want the cleaning-up to work.)

    Optional arg debug_compilation, when 1, compiles with -g.  Otherwise we
    compile with -O2 -DNDEBUG.

    Examples:
      1. The obligatory "Hello world":
        $ cat hello.cpp
        #include <vector>
        #include <cstdio>
        int F( std::vector<int> vi ) {
            printf("Hello world.\\n");
            return 0;
        }
        $ chombodata -i
        >>> from module_maker import *
        >>> m = makeModule( '/home/jdoe/hello.cpp' )
        >>> m.F(())
        Hello world.
        0
        >>> ^d
        $

      2. More relevant to ChomboVis, with F() a standalone user function:
        $ chombodata -i
        >>> from visualizable_dataset import *
        >>> from module_maker import *
        >>> v = VisualizableDataset( hdf5filename = 'stuff.hdf5' )
        >>> bld = v.getBoxLayoutData(1,2)
        >>> mymodule = makeModule( '/home/jdoe/myfunc.cpp' )
        >>> mymodule.F( (), bld.getFArray(3) )
        (That passed to F() the FArray of level 1, component 2, box 3.  The
        vector<int> argument was left empty -- which is perfectly legal -- but
        see the next example for how it can be stuffed with probably essential
        dimensions data.)
        
      3. F() a user function that needs to link to a library:
        $ chombodata -i
        >>> from visualizable_dataset import *
        >>> from module_maker import *
        >>> v = VisualizableDataset( hdf5filename = 'stuff.hdf5' )
        >>> cf = '-I../Chombo/lib/src/AMRElliptic'
        >>> lf = '-L../Chombo/lib/ -lamrelliptic3d.Linux.g++.g77'
        >>> yourmodule = makeModule( '/home/jdoe/yourfunc.cpp', extra_CFLAGS=cf, extra_LDFLAGS=lf )
        >>> bld = v.getBoxLayoutData(1,2)
        >>> layout = bld.getBoxLayout()[3][0] + bld.getBoxLayout()[3][1]
        >>> yourmodule.F( layout, bld.getFArray(3) )

    For another example, complete with a file containing the F() function, and
    a few small libraries to link to, see examples/module_maker/Readme.

    This feature is known to work on Linux, FreeBSD and MacOSX Darwin.
    """
    anag_utils.funcTrace()


    #
    # Generate temporary file names.
    #
    (module_cpp,module_cpp_handle) = anag_utils.g_temp_file_mgr.makeTempFile(
        'module', 'cpp', create=1, save=save_cpp )
    (module_o,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
        'module', 'o', create=0 )
    (module_so,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
        'module', 'so', create=0 )


    #
    # Generate a module-defining .cpp file.
    #
    module_root = module_so.split('/')[-1:][0][:-3]
    module_cpp_handle.write(
                  '#include "data_access/PointerHandleMap.h"' + '\n'
                + '#include "' + func_filename + '"\n'
                + generateDynomodule( maximum_arity=maximum_arity,
                                      module_name = module_root ) )
    module_cpp_handle.close()


    #
    # Compile
    #

    # These environment variables we use, they're set in the chombovis script.
    cflags = '-c -Wall -ftemplate-depth-30 -fPIC '\
           + ' -I' + os.getenv( 'PYTHON_INCLUDE_DIR' ) \
           + ' -I' + os.getenv( 'CHOMBOVIS_HOME' ) + '/include/ChomboVis' \
           + ' -I' + os.getenv( 'HDF5_INCLUDE_DIR' ) + ' ' \
           + extra_CFLAGS
    if debug_compilation == 1:
        cflags += ' -g '
    else:
        cflags += ' -O2 -DNDEBUG '
    if os.sys.platform.find('darwin') != -1:
        cflags += ' -Wno-long-double '
    gpp = os.getenv('GPP')
    compile_string =  gpp + ' ' + cflags + ' ' + module_cpp + ' -o ' \
                   + module_o
    print compile_string
    retval = os.system( compile_string )
    if retval != 0:
        anag_utils.error( "Compilation return value was", retval )
        return None                                     # Early return


    #
    # Link
    #
    platform = os.sys.platform
    if   (platform.find('linux') != -1) or (platform.find('freebsd') != -1):
        link_magicwords = '-shared -Wl,-soname,' + module_so + ' -o ' \
                        + module_so
    elif platform.find('darwin') != -1:
        link_magicwords = '-Wl,-bind_at_load -flat_namespace ' \
                        + '-undefined suppress -o ' + module_so + ' -bundle '
    else:
        anag_utils.fatal( 'Untested platform:', os.sys.platform )

    linklibs = '-L' + os.getenv('CHOMBOVIS_HOME') + '/lib/ChomboVis ' \
             + '-L' + os.getenv( 'HDF5_LIBRARY_DIR' ) + ' ' \
             + module_o \
             + ' -lchombovis_data -lchombovis_utils -lhdf5'
    link_string = gpp + ' ' + link_magicwords + ' ' + linklibs + ' ' \
                + extra_LDFLAGS
    retval = os.system( link_string )
    if retval != 0:
        anag_utils.error( "Linker return value was", retval )
        return None                                     # Early return

    #
    # Import the new module (adding /tmp/chombovis_$USER to PYTHONPATH).
    # 
    sys.path.append( anag_utils.g_temp_file_mgr.getTmpDir() )
    exec( 'import ' + module_root )
    return sys.modules[ module_root ]

#Cut from here

"""
Stuff from here down generates the C++ code that gets compiled and linked into
a Python module.
"""

s4="    "

#-----------------------------------------------------------------------
g_dynomodule_part1 =\
"""
// This is material that gets #included in .cpp files we create on the fly,
// which end up linked into new Python-callable modules.

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE  // Avoids gcc3.3.2 warning
#endif
#include "data_access/python/module_templates.cpp"
#include "utils/Misc.h"
#include <iostream>
#include <map>
#include <vector>

/** Used to identify arity and float/double of user-supplied function F(). */
enum Precision {Float=1, Double=2};

struct FuncType 
{
    FuncType( std::pair<Precision, int> q ) : precision(q.first),
                                              arity(    q.second) { }
    Precision precision;
    int arity;  // Number of float* or double* arguments.
};

// These are the signatures of the user-defined functions we are able to wrap:
"""
#-----------------------------------------------------------------------

g_dynomodule_part2 =\
"""
template<typename REAL_T> static PyObject *
WrappedF( PyObject * args, int arity )
{
    int retval(-1);
    ParseTraits<std::vector<int> > parseTraits;
    assert( PyTuple_Size( args ) == arity+1 );
    std::vector<int> vectArg(
        parseTraits.Convert( PyTuple_GetItem( args, 0 ) ) );
    int intArgs[arity];
    boost::shared_array<REAL_T> arrayArgs[arity];
    for( int i=0;i<arity;++i )
    {
        intArgs[i] = PyInt_AsLong( PyTuple_GetItem( args, i+1 ) );
        arrayArgs[i] = SharedArrayHandleMap<REAL_T>::GetPointer( intArgs[i] );
    }
"""
#-----------------------------------------------------------------------

g_dynomodule_part3 =\
"""
    return PyInt_FromLong( retval );
    // That return value, retval, is the return value from the user-defined
    // function F(), and can be used for anything the user wishes (maybe a
    // status code?)
}


static PyObject *
WrappedF( PyObject * self, PyObject * args )
{
    FuncType ft( FindFuncType( F ) );
        // F is the user-supplied function, defined in another file.

    assert( (ft.precision==Float) || (ft.precision==Double) );
    if( ft.precision == Float )
    {
        return WrappedF<float>( args, ft.arity );
    } else
    {
        return WrappedF<double>( args, ft.arity );
    }
}


static PyMethodDef DynoMethods[] =
{
    {"F", WrappedF, METH_VARARGS,
        "Calls user-supplied function of arbitrary arity"},
    {0,0,0,0}
};


extern "C"
void
initYOUR_LIB_HERE()
{
    Py_InitModule( "YOUR_LIB_HERE", DynoMethods );
}
"""
#-----------------------------------------------------------------------


def _SR( n ):
    """
    Return a string with struct SR, which defines the Fn function typedefs.
    """
    result = ''
    result += "template<typename REAL_T> struct SR\n"
    result += "{\n"
    for i in range(0,n+1):
        args = 'std::vector<int>'
        for j in range(0,i):
            args += ",REAL_T*"
        result += s4 + "typedef int F" + str(i) + "(" + args + ");\n"
    result += "};\n"
    return result


def _FuncTypes( n ):
    """
    Return a string with the FindFuncType definitions.
    """
    result = ''
    for i in range(0,n+1):
        result += "FuncType FindFuncType( SR<float>::F" + str(i) + ") "\
            + "{ return std::make_pair( Float," + str(i) + " ); }\n"
        if i > 0:
            result += "FuncType FindFuncType( SR<double>::F" + str(i) + ") "\
                + "{ return std::make_pair( Double," + str(i) + " ); }\n"
    return result


def _switchArity( n ):
    """
    Generates the code under "switch( arity )".  Arg n is the highest case
    (i.e. arity) to go to.
    """
    result = ''
    result += s4 + "switch( arity )\n"
    result += s4 + "{\n"
    for i in range(0,n+1):
        result += s4 + "    case " + str(i) + ": {\n"
        result += s4 + "        retval = reinterpret_cast<typename SR<REAL_T>::F"\
               + str(i) + "*>(F)(\n" \
               + s4+s4+s4+s4+ "vectArg\n"

        for j in range(0,i):
            result += s4 + "           ,arrayArgs[" + str(j) + "].get()\n"
        result += s4 + "           );\n" 
        result += s4 + "        break;\n"
        result += s4 + s4 + "}\n"
    result += s4 + s4 + "default: assert(0);//Compilation should have failed!\n"
    result += s4 + "}\n"
    return result


def generateDynomodule( maximum_arity, module_name ):
    result = ''
    result += g_dynomodule_part1
    result += _SR(maximum_arity)
    result += _FuncTypes(maximum_arity)
    result += g_dynomodule_part2
    result += _switchArity(maximum_arity)
    'digraph_foo' # See digraph_foo in digraph.py
    result += anag_utils.sed( g_dynomodule_part3, 'YOUR_LIB_HERE', module_name )
    'digraph_foo'
    return result

'digraph_foo'
