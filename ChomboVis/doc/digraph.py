#####################################################################
# Generate the graphvis input file for generating the dependency graph
# for the ChomboVis project.
#####################################################################

import sys
import re
import string
import os
try:
    import anag_utils
except:
    sys.stderr.write( "You must put ../src_py:../lib on your PYTHONPATH\n"
                      "and ../lib on your LD_LIBRARY_PATH.\n" )
    sys.exit(1)

g_digraph_foo = '\'digraph_foo\''

class FileGroups:
    """
    List of the files whose dependencies we are interested in.
    """
    def getTopLevelFiles(self): return self.toplevel_files
    def getControlFiles(self): return self.control_files
    def getApiFiles(self): return self.api_files
    def getVtkFiles(self): return self.vtk_files
    def getUtilityFiles(self): return self.utility_files
    def getAllFiles(self): return ( self.getTopLevelFiles() +
                                    self.getControlFiles() +
                                    self.getVtkFiles() +
                                    self.getUtilityFiles() +
                                    self.getApiFiles() )

    def __init__( self ):
        self.toplevel_files = (
            'chombovis.py',
            'main.py' )

        self.control_files = (
            'menubar.py',
            'control_annotation.py',
            'control_cameras.py',
            'control_clip.py',
            'control_cmap.py',
            'control_data.py',
            'control_eb.py',
            'control_fab_tables.py',
            'control_grid.py',
            'control_iso.py',
            'control_particles.py',
            'control_print.py',
            'control_slice.py',
            'control_stream.py',
            'control_vol.py' )

        self.vtk_files = (
            'vtk_annotation.py',
            'vtk_axes.py',
            'vtk_cameras.py',
            'vtk_clip.py',
            'vtk_cmap.py',
            'vtk_data.py',
            'vtk_eb.py',
            'vtk_fab_tables.py',
            'vtk_grid.py',
            'vtk_iso.py',
            'vtk_particles.py',
            'vtk_print.py',
            'vtk_slice.py',
            'vtk_stream.py',
            'vtk_vol.py',
            'vtk_vtk.py',
            'vtk_line_plot.py',
            'vtk_line_segment.py',
            'vol_tk_interactor.py',
            'selector.py' )

        self.api_files = (
            'annotation_api.py',
            'clip_api.py',
            'cmap_api.py',
            'eb_api.py',
            'grid_api.py',
            'iso_api.py',
            'misc_api.py',
            'network_api.py',
            'particles_api.py',
            'reader_api.py',
            'slice_api.py',
            'stream_api.py',
            'volume_api.py' )

        self.utility_files = (
            'algorithms.py',
            'anag_megawidgets.py',
            'anag_utils.py',
            'box.py',
            'box_layout.py',
            'box_layout_data.py',
            'cmd_line.py',
            'module_maker.py',
            'dialog_inventory.py',
            'visualizable_dataset.py',
            'notifier.py',
            'saved_states.py',
            'self_control.py' )


class CmdLine:
    """
    Copied out a little bit of cmd_line.py.
    For valid command-line options, see the definition of self.usage_message.
    """

    def __init__( self ):
        """ Initialize all legal command-line options. """
        self.debug_level = 2

        # Legal command-line options.  For definitions, see self.usage_message.
        self.show_toplevel = 1
        self.show_control = 1
        self.show_api = 1
        self.show_vtk = 1
        self.show_utility = 1
        self.show_nonchombo = 1
        self.single_module = None
        self.focus = 0

        self.usage_message = """
        ******************************************************************
        * Usage: python digraph.py [option_name=option_value]
        *
        * Options:
        *
        * 1. show_toplevel
        *    If 1, then consider toplevel modules (main and chombovis).
        *    If 0, ignore them.
        *    Default: 1
        *
        * 2. show_control
        *    If 1, then consider GUI control modules (menubar and control_*)
        *    If 0, ignore them.
        *    Default: 1
        *
        * 3. show_api
        *    If 1, then consider *_api modules.
        *    If 0, ignore them.
        *    Default: 1
        *
        * 4. show_vtk
        *    If 1, then consider modules that call into Vtk libraries (vtk_*,
        *        selector, vol_tk_interactor).
        *    If 0, ignore them.
        *    Default: 1
        *
        * 5. show_utility
        *    If 1, then consider utility modules (algorithms, anag_megawidgets,
        *        anag_utils, cmd_line, notifier, saved_states).
        *    If 0, ignore them.
        *    Default: 1
        *
        * 6. show_nonchombo
        *    If 1, then show dependencies on non-ChomboVis modules (e.g. sys,
        *        Tkinter, math, os, re,...).  (But even if 1, we make no
        *        attempt to show what these non-ChomboVis modules themselves
        *        depend on.)
        *    If 0, ignore them.
        *    Default: 1
        *
        * 7. focus
        *    If 1, then show all modules, potentially, but only when they have
        *    connections to those selected by the other options.
        *    Default: 0.
        *
        * 8. single_module
        *    Like show_*, but restricted to one module.
        *    Example: "single_module=control_iso".
        *    Default: None
        ******************************************************************
        """

        self._gatherCmdLineOptions()


    def _gatherCmdLineOptions( self ):
        anag_utils.funcTrace()

        if len(sys.argv) == 1:
            return

        argvee = self._crunch( sys.argv[1:] )

        for item in argvee:
            try:
                key, value = item.split( '=' )
                exec( 'x = self.' + key )
                # That will have thrown an exception, if self.key doesn't
                # exist, i.e. illegal command-line option.
                # So if we're here, things are looking good.  (But we still
                # haven't checked the reasonableness of the value.)
                exec( 'self.' + key + '="' + value + '"' )
            except:
                anag_utils.fatal( self.usage_message )

    def _crunch( self, argvee ):
        """
        Arg argvee is argv[1:]

        Normalize cmd-line, if possible, by removing blanks from around
        '=' signs.
        """
        anag_utils.funcTrace()

        str = ''
        for i in argvee:
            str = str + i + ' '
        str = str[:-1]

        pos = str.find(' =')
        while not pos == -1:
            str = str[:pos] + str[pos+1:]
            pos = str.find(' =')

        pos = str.find('= ')
        while not pos == -1:
            str = str[:pos+1] + str[pos+2:]
            pos = str.find('= ')

        return str.split(' ')


class Colors:
    """
    Defines colors for the nodes and edges, by different categories of module.
    """
    def __init__( self ):

        color_saturation = ".4 "
        color_value = "1.0 "
        self.toplevel  = ".1 " + color_saturation + color_value
        self.control   =  ".3 " + color_saturation + color_value
        self.api       =  ".0 " + color_saturation + color_value
        self.vtk       =  ".5 " + color_saturation + color_value
        self.utility   =  ".7 " + color_saturation + color_value
        self.nonchombo = ".9 " + color_saturation + color_value

        # This is for reverse colors.
        """ 
        color_saturation = "0.95 "
        color_value = "0.4 "
        """    

def findImports( file_lines ):
    """
    Find the names of all modules mentioned in "import module" or
    "from module import" style.

    Arg file_lines is what you'd get from a file.readlines() on the file.

    Returns the imported modules.
    """
    import_re = re.compile( '^ *import' )
    from_import_re = re.compile( '^from [A-z_][A-z_]* import' )
    result = []

    for line in file_lines:
        if import_re.match( line ):
            result.append( line.split()[1] )
        if from_import_re.match( line ):
            result.append( line.split()[1] )

    return result


def listIntersection( list1, list2 ):
    """
    Return the list that is the intersection of list1 and list2.
    """
    result = []
    for k in list1:
        if k in list2:
            result.append(k)
    return result


def listDifference( list1, list2 ):
    """
    Return a list that consists of all the elements of list1 that are not also
    in list2.
    """
    result = []
    for k in list1:
        if not k in list2:
            result.append(k)
    return result


def file2module( file_name ):
    """ Return file_name with the '.py' taken off the end. """
    return file_name[:-3]


def stripQuotes( file_lines, quote_chars, mirror=None, replace=False ):
    """
    Strips out everything between pairs of quote_chars.  Arg quote_chars is
    typically ', ", or \"\"\".

    Arg file_lines is what you'd get from a file.readlines() on the file.

    If arg mirror==1, then the close-quote is taken as quote_chars.reverse()
    (useful for stripping out /* ... */ pairs).

    Returns the lines of the file -- minus the material between matching quotes
    -- unless optional arg replace is True, in which case everything but '\n'
    is converted to blanks.
    """
    result = ''  # Will be a list of strings -- lines of the file.

    # We'll work on file_lines joined into a single long string, as that
    # eliminates the complications of finding pairs of quotes that span
    # multiple lines.
    # After we're done, we'll break the result back up at the '\n's, so the
    # other filters will work properly.
    joined_lines = string.join( file_lines, '' )

    if mirror == 1:
        reversed_quote_chars = list(quote_chars)
        reversed_quote_chars.reverse()
        reversed_quote_chars = string.join( reversed_quote_chars, '' )

    open_pos = joined_lines.find( quote_chars )
    if open_pos == -1:
        if quote_chars != g_digraph_foo:
            anag_utils.warning(
                "Failed to find even one open_pos.  Returning now." )
        return file_lines
    result = result + joined_lines[:open_pos]

    while 1:
        if mirror == 1:
            close_pos = joined_lines.find( reversed_quote_chars,
                                           open_pos + len(quote_chars))
        else:
            close_pos = joined_lines.find( quote_chars,
                                           open_pos + len(quote_chars))
        if close_pos == -1:
            anag_utils.error( "!!! Failed to find closing quote near character",
                open_pos, ". Context was", joined_lines[open_pos:open_pos+100],
                 ". mirror=", mirror, ", quote_chars=", quote_chars)
            return result

        if replace == True:
            result += blankChars(joined_lines[open_pos:
                                              close_pos+len(quote_chars)])

        open_pos = joined_lines.find( quote_chars, close_pos + len(quote_chars))
        if open_pos == -1:
            result += joined_lines[close_pos+len(quote_chars):]
            break
        else:
            result += joined_lines[close_pos+len(quote_chars):open_pos]

    # Convert back to a list of '\n'-terminated strings.
    result = result.split('\n')
    for i in range(0, len(result)):
        result[i] = result[i] + '\n'
    return result


def blankChars( str ):
    """
    Replace all non-'\n' chars to blanks.  This is useful when what you want to
    do is not strip out comments but just replace them with white space (which
    is what we want to do when we're looking for the "last #include".
    """
    trans_table=list(256*' ')
    trans_table[10] = '\n'
    trans_table=string.join(trans_table,'')
    return string.translate(str,trans_table)


def stripBlankLines( file_lines ):
    """
    Remove blank lines.
    """
    blank_line = re.compile('^ *$')
    result = []
    for line in file_lines:
        if not blank_line.match( line ):
            result.append( line )
    return result


def stripPoundComments( file_lines ):
    """
    Strips out everything past a '#' on a line, except for color specifications
    (which match 
    #[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9]).

    Arg file_lines is what you'd get from a file.readlines() on the file.

    Returns the lines of the file -- minus the material after the '#' signs.
    """
    result = [] # Will be a list of strings -- lines of the file.
    color_re = re.compile(
        '#[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9]' )

    for line in file_lines:
        pos = line.find( '#' )
        if pos == -1:
            result.append( line )
        elif not color_re.match( line[pos:] ):
            result.append( line[:pos] )

    return result


def findModuleReferences( file_lines, file_groups ):
    """
    Finds where modules in this directory are used.  That is, for every element
    of file_groups.getAllFiles(), finds where its name -- minus the .py but
    prepended with "self" -- appears in any of the same files.
    Look also for the file name minus the .py but plus a '.'.

    Arg file_lines is what you'd get from a file.readlines() on a file.
    Make sure you call this after stripping out all the comments.

    Returns a list of the modules used.
    """
    result = [] # Will be a list of strings -- module names.

    for m in map( file2module, file_groups.getAllFiles() ):
        for line in file_lines:
            if( (line.find( 'self.' + m ) != -1)
            or  (line.find( m + '.' ) != -1) ):
                if not m in result: result.append( m )
    result.sort()
    return result


def writeDotFile( depdict, file_groups ):
    """
    Arg depdict stands for "dependency dictionary".  Its keys are modules
    that will have arrows coming out of them, while the values are the
    modules to which those arrows will go.

    Here we convert to the format dot accepts, and write that on stdout.
    """

    print 'digraph chombodigraph {'

    #
    # Node coloration.
    #
    nodes = list(depdict.keys())
    for v in depdict.values():
        nodes = nodes + list(v)
    nodes.sort()
    # Eliminate duplicates.
    r = range( 0, len(nodes) )
    r.reverse()
    for i in r:
        if nodes.index( nodes[i] ) < i:
            del nodes[i]
    # Now nodes is a sorted list of all the nodes we'll see in the drawing.

    def nodots( str ):  # Change, e.g. os.path to os_path.
        pos = str.find( '.' )
        if pos == -1:
            return str
        else:
            return string.join( str.split('.'), '_' )

    colors = Colors()
    colormap = {} # keys=module names, values=colors.
    for n in nodes:
        f = n + '.py'
        if   f in file_groups.getTopLevelFiles():
            colormap[n] = colors.toplevel
        elif f in file_groups.getControlFiles():
            colormap[n] = colors.control
        elif f in file_groups.getApiFiles():
            colormap[n] = colors.api
        elif f in file_groups.getVtkFiles():
            colormap[n] = colors.vtk
        elif f in file_groups.getUtilityFiles():
            colormap[n] = colors.utility
        else:
            colormap[n] = colors.nonchombo
        print nodots(n), '[color="', colormap[n], '", style=filled];'
    

    # Sort the keys, so that the output is predictable (i.e. so we can compare
    # to the canonicals.
    sorted_keys = depdict.keys()[:]
    sorted_keys.sort()

    for k in sorted_keys:
        if len(depdict[k]) > 0:
            sys.stdout.write( nodots(k) + ' -> {' )
            for i in depdict[k]:
                sys.stdout.write( nodots(i) + ' ' )
            sys.stdout.write( '} [color="' + colormap[k] + '"];\n' )
            
    print '}'


def removeUnwanted( depdict, cmd_line, file_groups ):
    """
    Arg depdict stands for "dependency dictionary".  Its keys are modules
    that will have arrows coming out of them, while the values are the
    modules to which those arrows will go.

    Look at cmd_line.show_* and cmd_line.focus to determine how to censor
    some of the graph edges.

    This function modifies arg depdict.
    """

    files_to_show = []
    if cmd_line.single_module == None:
        if int(cmd_line.show_toplevel) == 1:
            files_to_show = files_to_show+list(file_groups.getTopLevelFiles())
        if int(cmd_line.show_control) == 1:
            files_to_show = files_to_show +list(file_groups.getControlFiles())
        if int(cmd_line.show_api) == 1:
            files_to_show = files_to_show +list(file_groups.getApiFiles())
        if int(cmd_line.show_vtk) == 1:
            files_to_show = files_to_show +list(file_groups.getVtkFiles())
        if int(cmd_line.show_utility) == 1:
            files_to_show = files_to_show +list(file_groups.getUtilityFiles())
    else:
        files_to_show = (cmd_line.single_module + '.py',)
    modules_to_show = map( file2module, files_to_show )

    for k in depdict.keys():
        if int(cmd_line.focus) == 0:
            if not k in modules_to_show:
                del( depdict[k] )
            else:
                if int(cmd_line.show_nonchombo) == 0:
                    depdict[k] = listIntersection( depdict[k], modules_to_show )
                else:
                    depdict[k] = listDifference(
                        depdict[k],
                        map( file2module,
                            listDifference( file_groups.getAllFiles(),
                                            files_to_show )))

        else: # Show all edges involving files_to_show
            if k in modules_to_show:
                pass # OK, use it.
            else:
                depdict[k] = listIntersection( depdict[k], modules_to_show )

    for k in depdict.keys():
        if depdict[k] == []:
            del depdict[k]


if __name__ == '__main__':

    cmd_line = CmdLine() # Defined here; this is not cmd_line.py of ChomboVis.
    anag_utils.setDebugLevel( int(cmd_line.debug_level) )

    file_groups = FileGroups()

    dependency_dictionary = {}

    for filename in file_groups.getAllFiles():
        f = open( '../src_py/' + filename )
        lines = f.readlines()
        anag_utils.info( 'filename=',filename )

        #
        # Strip out comments and all material within single and double quotes.
        # We strip out material within quotes because some of those strings
        # look like module invocations (and they're bogus of course).
        # This stripping is not perfect.  It gets tripped up by, among other
        # things, a '\#' inside quotes.
        # That's where 'digraph_foo' comes in; use that to surround text that
        # digraph would otherwise choke on.  There's an example of this in
        # module_maker.py
        #
        stripped = stripQuotes( lines, g_digraph_foo )
        stripped = stripPoundComments( stripped )
        stripped = stripQuotes( stripped, '"""' )
        stripped = stripBlankLines( stripped )
        stripped = stripQuotes( stripped, '"' )
        stripped = stripQuotes( stripped, "'" )

        filename_base = filename.split('.')[0]
        (tmp_file,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
            filename_base, 'py' )
        outfile = open( tmp_file, 'w' )
        outfile.writelines( stripped )

        #
        # Find the dependencies.
        #
        # "import" statements.
        imports = findImports( stripped )
        # Usage as "self.<module>" or "<module>."
        module_references = findModuleReferences( stripped, file_groups )
        dependencies = []
        for m in imports + module_references:
            if not m in dependencies:
                dependencies.append( m )
        dependencies.sort()

        dependency_dictionary[ file2module( filename ) ] = dependencies

    removeUnwanted( dependency_dictionary, cmd_line, file_groups )
    writeDotFile( dependency_dictionary, file_groups )

