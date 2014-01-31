import re
import string
import sys


"""
Generates project-specific cmdline.hpp and cmdline.cpp, from a text file
of the following format:

   long_name single_char_name  type   default   comment

Where:
    long_name is a name in --foo_bar format.  The data member will be called
        m_foo_bar and the accessors will be FooBar().

    single_char_name: e.g. -a, -b.

    type is either "bool", "short", "int", "double" or "string".

    default is the default value.  If it's a string, it must not contain white
        space.

    comment is a string that describes what the variable is for.  It may contain
        white space, and should be surrounded by double quotes.

Each project's Makefile should call this program to generate cmdline.cpp and
cmdline.hpp for it.
"""

# This will be overridden in __main__().
g_cmdline_file='/dev/null'

##
##Define constant parts of the files we are going to generate.
##
g_cmdline_cpp_part1 =\
   """
   #include "cmdline_base.hpp"
   #include "cmdline.hpp"
   
   /*******************************************************************
    * This is a generated file.  Do not hand-edit it.
    *******************************************************************/
   
   Cmdline::Cmdline(int argc, char *argv[])
     :
"""
g_cmdline_cpp_part2 =\
    """
        CheckForCmdlineFile(std::string(
             std::string(getenv("HOME") ? getenv("HOME") : "/dev/null") + std::string("/") + std::string(
    """

g_cmdline_cpp_part3 =\
    """")));
        SaveCmdlineFileTimestamp();
        LoadDefaultsFromFile();
        ParseCommandLine(argc, argv);
    }
    """
g_cmdline_hpp_part1 =\
    """
    #ifndef INCLUDED_CMDLINE_HPP
    #define INCLUDED_CMDLINE_HPP
    #include "cmdline_base.hpp"
    /*******************************************************************
     * This is a generated file.  Do not hand-edit it.
    *******************************************************************/
     
    class Cmdline : public CmdlineBase
    { 
      public: 
         
        Cmdline(int argc, char *argv[]);
        virtual ~Cmdline() {}
    """
g_cmdline_hpp_part2 =\
    """
    }; 
    #endif // INCLUDED_CMDLINE_HPP    
    """


class VariableInfo:
    """
    The information in a line of the variable definition text file (whose format
    is detailed above).
    """
    def __init__(self, textfile_line):
        """
        Construct from a line of the variable definition text file.
        """
        split_line = textfile_line.split()
        self.long_name = split_line[0]
        self.short_name = split_line[1]
        self.type   = split_line[2]
        if self.type == 'string' : self.type = 'std::string'
        self.default = split_line[3]
        self.comment = split_line[4:]


    def makeAccessorDeclarations(self):
        """
        Generate the getter and the setter, and return them as a pair.
        """
        getter = self.type + ' ' + self.foo_bar2FooBar(self.long_name) \
               + '() const { return ' + self.foo_bar2m_foo_bar(self.long_name) \
               + '; }'
        setter = 'void ' + self.foo_bar2FooBar(self.long_name) + '(' + self.type\
               + ' x) {' + self.foo_bar2m_foo_bar(self.long_name) + ' = x; }'
        return (getter, setter)


    def makeMemberDataDeclaration(self):
        return 'Anything ' + self.foo_bar2m_foo_bar(self.long_name) + '; // '\
               + string.join(self.comment)


    def makeArgmapElement(self):
        """
        Assign an element to m_argmap, to correspond to this variable.
        """
        result = 'm_argmap["' + self.long_name + '"] = new SetType'\
               + '(&'\
               + self.foo_bar2m_foo_bar(self.long_name) + ', '\
               + string.join(self.comment) + ');'
        return result

    def buildShort2LongargMap(self):
        result = "m_short2longarg_map['" + self.short_name + "'] = " \
               + '"' + self.long_name + '";'
        return result

    def buildShortOpts(self):
        result = 'm_shortopts += "' + self.short_name + ':";'
        return result

    def buildLongOpts(self):
        result = 'm_longopts.push_back(MakeStructOption("' + self.long_name \
               + '", required_argument, NULL, \'' + self.short_name + '\'));'
        return result

    def foo_bar2m_foo_bar(self, foo_bar):
        """
        Return "m_foo_bar", the standard for naming member data.
        """
        return 'm_' + foo_bar
    
    def foo_bar2FooBar(self, foo_bar):
        """
        Return "FooBar", the standard for naming accessors.
        """
        result = ''
        for piece in foo_bar.split('_'):
            result += piece.capitalize()
        return result
    

    def testme(self):
        """
        Assumes self has been constructed from a cmdline var info file line.
        """
        print "makeAccessorDeclarations returns:",\
              self.makeAccessorDeclarations()
        print "makeMemberDataDeclaration returns:",\
              self.makeMemberDataDeclaration()
        print "makeArgmapElement returns:", self.makeArgmapElement()


    
if __name__ == '__main__':
    """
    Usage: python generate_cmdline cmdline_var_info_file cmdline_file

    ...where cmdline_var_info_file is the text file that describes the variables,
    while cmdline_file is the file (must be in $HOME directory!) that sets
    those variables' values.  
    FIXME: The cmdline_file is redundant.  We can dispense with it as soon as
    we modify CmdlineBase::LoadDefaultsFromFile() to read the format of
    cmdline_var_info_file instead of the format of cmdline_file.
    """
    varinfo_file = open(sys.argv[1])
    varinfo_lines = varinfo_file.readlines()
    g_cmdline_file = sys.argv[2]
    comment_regex_pattern = '^ *#|^\s*$'
    comment_regex = re.compile(comment_regex_pattern)

    #
    # Generate cmdline.cpp
    #
    cmdline_cpp = open('cmdline.cpp', 'w')
    cmdline_cpp.write(g_cmdline_cpp_part1)
    for line in varinfo_lines:
        if comment_regex.match(line):
            continue
        varinfo = VariableInfo(line[:-1])
        cmdline_cpp.write('        ' + varinfo.foo_bar2m_foo_bar(varinfo.long_name) \
                         + '(' + varinfo.default + ')')
        if line != varinfo_lines[-1]:
            cmdline_cpp.write(',')
        cmdline_cpp.write('\n')

    cmdline_cpp.write('    {\n')
    for line in varinfo_lines:
        if comment_regex.match(line):
            continue
        varinfo = VariableInfo(line[:-1])
        cmdline_cpp.write('        ' + varinfo.makeArgmapElement() + '\n')
        cmdline_cpp.write('        ' + varinfo.buildShort2LongargMap() + '\n')
        cmdline_cpp.write('        ' + varinfo.buildShortOpts() + '\n')
        cmdline_cpp.write('        ' + varinfo.buildLongOpts() + '\n')

    cmdline_cpp.write(g_cmdline_cpp_part2 + '                "' + g_cmdline_file +
                      g_cmdline_cpp_part3 + '\n')

    #
    # Generate cmdline.hpp
    #
    cmdline_hpp = open('cmdline.hpp', 'w')
    cmdline_hpp.write(g_cmdline_hpp_part1 + '\n')

    #     Generate accessor functions.
    for line in varinfo_lines:
        if comment_regex.match(line):
            continue
        varinfo = VariableInfo(line[:-1])
        accessors = varinfo.makeAccessorDeclarations()
        cmdline_hpp.write('        ' + accessors[0] + '\n'
                         +'        ' + accessors[1] + '\n\n')

    #    Generate member data.
    cmdline_hpp.write('      private:\n')
    for line in varinfo_lines:
        if comment_regex.match(line):
            continue
        varinfo = VariableInfo(line[:-1])
        cmdline_hpp.write('        ' + varinfo.makeMemberDataDeclaration()+'\n')

    cmdline_hpp.write(g_cmdline_hpp_part2 + '\n')
