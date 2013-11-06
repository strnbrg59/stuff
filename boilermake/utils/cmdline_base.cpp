#include "cmdline_base.hpp"
#include "trace.hpp"

#include <cassert>
#include <fstream>
#include <cstdio> 
#include <cstdlib> 
#include <cstring>
#include <string>
#include <sys/types.h> 
#include <sys/stat.h>
#include <time.h> 
#include <vector>
using std::cerr;
using std::cout;
using std::endl;

//----------------------------------------------------------------- 
// 
// class Cmdline 
// 
// Process configuration data from the command line and/or cmdline file.
//  
//----------------------------------------------------------------- 

/** Initialize the Cmdline object from the master cmdline file.
 *  Then let any command-line override what you got from the file.
 *  An STL map acts as the one-stop reference and interface to all
 *  the parameters.
*/
CmdlineBase::CmdlineBase()
{ 
    m_argmap["debug_level"] = new SetType(&s_debug_level, "");
}


CmdlineBase::~CmdlineBase()
{
    for( std::map<std::string,SetType*>::iterator iter = m_argmap.begin();
         iter != m_argmap.end();
         ++iter )
    {
        delete iter->second;
    }
}


/** Supports feature whereby configuration is sensitive to run-time
 *   modifications to cmdline file.
*/
void
CmdlineBase::SaveCmdlineFileTimestamp()
{
    if( !m_cmdlineFileExists )
    {
        return;
    }

    // Save timestamp of cmdline file.
    struct stat mystat;
    stat( m_cmdlineFilename.c_str(), &mystat );
    m_lastTouch = mystat.st_mtime;
}


/** Cmdline file is assumed to reside in $HOME directory.
 *  New: It's not obligatory to have a cmdline file, and even if you have one
 *       it doesn't have to mention all the cmdline variables.
 */
void
CmdlineBase::CheckForCmdlineFile( std::string filename )
{
    Trace t("CmdlineBase::CheckForCmdlineFile()");
    m_cmdlineFilename = filename;

    // See if it exists.
    struct stat mystat;
    int err = stat( m_cmdlineFilename.c_str(), &mystat );
    if( err != 0 )
    {
        t.Info( "No cmdline file %s.  OK.", m_cmdlineFilename.c_str() );
        m_cmdlineFileExists = false;
    } else
    {
        m_cmdlineFileExists = true;
    }
}


/** Check if m_cmdlineFilename has been touched.  If yes, then reset cmdline
 *  parameters.  Note this will override anything originally set from the
 *  command line (and this is perhaps counterintuitive, because in
 *  Cmdline::Cmdline(), the command line settings override the cmdline file
 *  settings.  So it's probably a good idea to set your params from the cmdline
 *  file, rather than from the command line.
 *
 *  If there's no cmdline file (precisely: if there was no cmdline file at
 *  start-up), then this is a no-op.
*/
void
CmdlineBase::Reload()
{
    if( ! m_cmdlineFileExists )
    {
        return;
    }
    struct stat mystat;
    stat( m_cmdlineFilename.c_str(), &mystat );
    if( mystat.st_mtime > m_lastTouch )
    {
        LoadDefaultsFromFile();
        m_lastTouch = mystat.st_mtime;
    }
}

/** Cmdline file must conform to the following format:
 *  Comments begin with #
 *  Blank lines are ok.
 *  All other lines should indicate the value of a parameter, in name=value
 *    format.  The name should start in the first column of the line, and
 *    there should be no blanks around the = sign.
 *  For string types, it's ok to put nothing after the = sign; this indicates
 *    a default value of "".
 *  
 *  The cmdline file is optional.  When it does exist, it does not need to
 *  mention every variable.
*/
void CmdlineBase::LoadDefaultsFromFile()
{
    Trace t("CmdlineBase::LoadDefaultsFromFile()");
    if( !m_cmdlineFileExists )
    {
        return;
    }

    // Open cmdline file
    std::ifstream infile( m_cmdlineFilename.c_str() );
    if( !infile )
    {
        t.FatalError( "Shouldn't have gotten here." );
    }

    // Read cmdline file and set data members.
    char linebuf[k_max_line_length+1];
    std::vector<std::string> all_param_names;
    while( !infile.eof() )
    {
        infile.getline( linebuf,k_max_line_length+1);
        if( strlen( linebuf ) > k_max_line_length-1 )
        {
            cerr << m_cmdlineFilename << " lines must not exceed "
                 << k_max_line_length << " columns.\n";
            exit(1);
        }

        // If the line starts with a character other than #, then interpret it
        // and set the corresponding data member.
        if( !isalpha(linebuf[0]) ) ;
        else
        {
            if( m_argmap.find(ArgName(linebuf)) != m_argmap.end() )
            {
                m_argmap[ArgName(linebuf)]->set(ArgVal(linebuf));
                if( ArgName(linebuf) == "debug_level" )
                {
                    DebugLevel( atoi(ArgVal(linebuf).c_str()) );
                }
                all_param_names.push_back( ArgName(linebuf) );
            }
        }
    }
}        
    

/** Command-line options all take the form of name=value, e.g.
 *  "lambda=0.3", "logfilelist=logs.dat".
 *
 *  The only other valid thing for the command line is "-h" or "--help".
*/
void CmdlineBase::ParseCommandLine( int argc, char *argv[] )
{
    Trace t("ParseCommandLine()");
    // Print help, if argv is -h or --help.
    if( argc==2
    && ( std::string(argv[1])=="-h" || std::string(argv[1])=="--help" ))
    {
        for( std::map<std::string,SetType*>::iterator i=m_argmap.begin();
                                            i!=m_argmap.end();
                                            i++ )
        {
            cerr << i->first << "       # " << i->second->getDescription() 
                 << endl;
        }
        cout << "**********************************************\n";
        exit(0);
    }

    // Go through all of argv[].
    for( int i=1;i<argc;i++ )
    {
        if( m_argmap.find( ArgName(argv[i]) ) == m_argmap.end() )
        {
            cerr << "***Error: " << ArgName(argv[i]) << " is not a legal "
                 << "command-line argument.  " << endl
                 << "Use -h option for help." << endl;
            exit(1);
        }
        m_argmap[ArgName(argv[i])]->set(ArgVal(argv[i]));
        t.Info() << "cmd-line-option " << ArgName(argv[i]) << "=" 
                 << ArgVal(argv[i]) << '\n';
        if( ArgName(argv[i]) == "debug_level" )
        {
            DebugLevel( atoi(ArgVal(argv[i]).c_str()) );
        }
    } 
} 
 
/** Arguments -- both at the command line and in the cmdline file --
 *  have the name=value format.
 *  This function returns the first part -- the name.
*/
std::string CmdlineBase::ArgName( std::string str )
{
    char* buf = new char[ str.length()+1 ];
    strcpy( buf, str.c_str() );
    std::string result( strtok( buf, "=" ) );
    delete[] buf;
    return result;
}

/** Arguments -- both at the command line and in the cmdline file --
 *  have the name=value format (no spaces allowed around the '=' sign!)
 *  This function returns the second part -- the value.  If it's a string, put
 *  double quotes around it; we'll strip them off.
*/
std::string CmdlineBase::ArgVal( std::string str )
{
    std::string result;
    char * buf = new char[ str.length()+1 ];
    strcpy( buf, str.c_str() );
    char * tok = strtok( buf, "=" );  assert( tok );
    tok = strtok( NULL, " \t\0\n#" ); assert( tok );

    // Strip off surrounding '"', if any.
    if( (tok[0]=='"') && (tok[strlen(tok)-1]=='"') )
    {
        tok[strlen(tok)-1] = 0;
        result = std::string( tok+1 );
    } else
    {
        result = std::string( tok );
    }

    delete[] buf;
    return result;
}


/*static*/ void
CmdlineBase::DebugLevel( int value )
{
    Trace t("CmdlineBase::DebugLevel()");
    s_debug_level = value;
} 

/*static*/int
CmdlineBase::DebugLevel()
{
    return s_debug_level;
}


void
CmdlineBase::PrintCmdlineParams( std::ostream & ost ) const
{
    Trace t("CmdlineBase::");
    for( std::map<std::string, SetType*>::const_iterator
         iter = m_argmap.begin();
         iter != m_argmap.end();
         ++iter )
    {
        t.Info() << "  " << iter->first << "=" << iter->second->toString()
                 << '\n';
    }
}

Anything CmdlineBase::s_debug_level = 3;


void
CmdlineFactory::Init(Cmdline const& cmdline)
{
    m_rep = &cmdline;
}


Cmdline const&
CmdlineFactory::TheCmdline()
{
    return *m_rep;
}

Cmdline const* CmdlineFactory::m_rep;
