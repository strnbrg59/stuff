#ifndef INCLUDED_CMDLINE_BASE_HPP
#define INCLUDED_CMDLINE_BASE_HPP

#include "argmap.h"
#include <map>
#include <string>

 
//----------------------------------------------------------------- 
// 
// class CmdlineBase 
// 
// This is intended as a base class for a class Cmdline that generate_cmdline.py
// generates in each project's src directory.  generate_cmdline.py is called
// from each project's Makefile.  We'd like to call it config except that would
// get confused with the config.h that autoconf produces.
//
// Cmdline variables can come from three places:
// 1. The variable definition file, which is in each project's src directory.
//    This file is used by generate_cmdline.py to generate the local
//    cmdline.{c,h}pp and set default values.
// 2. The cmdline file (typically ~/.<project-name>rc).
// 3. The command line.
// 
// The cmdline file is optional, and even when it's there it need not mention
// all the variables.  Values in the cmdline file override defaults from the
// variable definition file.  Values set on the command line override the
// cmdline file as well as the variable definition file.  But when (and if) the
// project calls CmdlineBase::Reload(), we look at the cmdline file and
// anything that's changed there takes effect, so run-time changes to the
// cmdline file override everything else.
//  
//----------------------------------------------------------------- 
 
class CmdlineBase 
{ 
  public: 
     
    CmdlineBase();
    virtual ~CmdlineBase();

    void Reload();
    void PrintCmdlineParams( std::ostream & ) const;

    static int DebugLevel();
    static void DebugLevel( int value);

    // --------------------------------

  protected:
    std::map<std::string,SetType*> m_argmap;

    void LoadDefaultsFromFile();
    void ParseCommandLine( int argc, char *argv[] );
    void SaveCmdlineFileTimestamp();
    void CheckForCmdlineFile( std::string );

    static int s_debug_level;  // 0=Fatal, 1=Error, 2=Warning, 3=Info, 4=Trace.

    // ---- end of configurable parameters -----

    std::string m_cmdlineFilename;   // Full list of params with initial values.
    bool   m_cmdlineFileExists;
    time_t m_lastTouch; // last time cmdline file was touched.

    std::string ArgName( std::string );
    std::string ArgVal( std::string );

  private:
    enum Misc {k_max_line_length=120};
}; 
 
#endif // INCLUDED_CMDLINE_BASE_HPP
