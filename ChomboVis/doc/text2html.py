#!/usr/bin/env python

#######################################################################
# Generates numbered headings and a table of contents.
# You just type <H1>...</H1>, <H2>...</H2> etc, and put
# <TOC></TOC> where you want your table of contents to go.
#######################################################################

import os
import string
import sys
import anag_utils

class CmdLine:
    """
    Copied out a little bit of cmd_line.py.
    For valid command-line options, see the definition of self.usage_message.
    """

    def __init__( self ):
        """ Initialize all legal command-line options. """
        self.debug_level = 2

        # Legal command-line options.  For definitions, see self.usage_message.
        self.max_depth = 1

        self.usage_message = """
        ******************************************************************
        * Generate numbered sections and a hyperlinked table of contents.
        *
        * Usage: text2html.py [option_name=option_value]
        *
        * Options:
        *
        * 1. max_depth
        *    The maximum depth we want the table of contents to go to.
        *    Default: 1
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


cmd_line = CmdLine()

# Get input from stdin
# FIXME: Isn't there a simpler way to read from stdin?
infile = os.fdopen( 0 )

table_of_contents = [] # Text of table of contents.
main_text = [] # All text lines other than table of contents.
chapter_nums = [0,0,0,0,0,0,0,0,0] # produces dotted chapter headings, e.g. 2.3.1
old_depth = 0   # x part (a number in [1-9]) of <Hx>

one_line = infile.readline()
#if( FIXME: How do we check for EOF?  Look into the EOFError exception.
#    print( "Usage: cat <infile> | " + os.argv[0] )

while( one_line != '' ) :

    # Look for "<H[1-9]>...</H".  We require the <H and </H appear on the same
    # line; otherwise it would be pretty hard to build the table of contents.
    open_pos = string.find( string.lower(one_line), "<h" )
    close_pos = string.find( string.lower(one_line), "</h" )

    if( ( open_pos != -1 )
    and ( close_pos != -1 ) 
    and ( len(one_line) >= open_pos + 4 )
    and ( string.find('123456789',one_line[open_pos+2]) != -1 )
    and ( one_line[open_pos+3] == '>' ) ):
        depth = string.atoi(one_line[open_pos+2])

        # Update chapter_nums
        chapter_nums[depth-1] = chapter_nums[depth-1] + 1
        for i in range(depth,9) :  # 9 is highest <H?> legal in HTML
            chapter_nums[i] = 0

        # Compute chapter heading number as a string
        dotted = "%d" % chapter_nums[0]
        i = 1
        while( chapter_nums[i] != 0 ) :
            dotted = "%s.%d" % (dotted,chapter_nums[i])
            i = i + 1

        # Create something along the lines of <A NAME="6.2. Foo"></A> <H3>6.2. Foo</H3>
        new_line = "<A NAME=\"" + dotted + ". " \
                 + string.strip(one_line[open_pos+4:close_pos]) + "\"></A>"\
                 + one_line[:open_pos+4] + " " + dotted + ". " + one_line[open_pos+4:]
        main_text.append( new_line[:-1] )

        # Build up table of contents
        for i in range(depth, min(old_depth, int(cmd_line.max_depth)+1)) :  # </UL>
            table_of_contents.append( "</UL>" )
        for i in range(old_depth, min(depth, int(cmd_line.max_depth)+1)) :  # <UL>
            table_of_contents.append( "<UL>" )
        if depth <= int(cmd_line.max_depth):
            table_of_contents.append( "<LI><A HREF=\"#" + dotted + ". "
                + string.strip(one_line[open_pos+4:close_pos]) + "\">"
                + dotted + ". " + string.strip(one_line[open_pos+4:close_pos]) + "</A>" )

        old_depth = depth
    else :
        if( (open_pos != -1) and (close_pos == -1) 
        and (string.find('123456789',one_line[open_pos+2]) != -1 ) ) :
            print "Error: <H and </H must appear on the same line"
            sys.exit(1)
        main_text.append( one_line[:-1] )

    one_line = infile.readline()

# End of while block

#
# Print
#
for i in main_text:
    if -1 != i.find( '<TOC></TOC>' ):
        for j in table_of_contents:
            print j
        print '</UL>'
    else:
        print i


