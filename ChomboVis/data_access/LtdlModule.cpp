#include "LtdlModule.h"

#include <iostream>
using std::cout; using std::cerr;
#include <cstdlib>
#include <cassert>
#include <cstring>

#include <ltdl.h>

#ifndef MODULE_PATH_ENV
#  define MODULE_PATH_ENV    "MODULE_PATH"
#endif


/**
    Might fail for various reasons.  Before actually using a LtdlModule
    object, test if it's any good by calling its IsGood() member.
*/
LtdlModule::LtdlModule( char const * moduleName )
  : m_rep(0),
    m_isgood(false)
{
    // Initialise libltdl.  Destructor will "lt_dlclose" it.
    lt_dlinit();
    if( Save_dlerror() )
    {
        cerr << "Error: lt_dlinit() failed: " << m_errormsg << '\n';
        return;
    }
    
    // Set the module search path.
    const char *path = getenv(MODULE_PATH_ENV);
    if( path )
    {
        lt_dlsetsearchpath(path);
        if( Save_dlerror() )
        {
            cerr << "Error: lt_dlsetsearchpath() failed: " << m_errormsg <<'\n';
            return;
        }
    }
  
    // Load the module.
    m_rep = lt_dlopenext( moduleName );
    if( !m_rep )
    {
        Save_dlerror();
        cerr << "Error: lt_dlopenext() failed: " << m_errormsg << '\n';
        return;
    }

    m_isgood = true;
}


/**
    Returns true if this module contains a certain symbol (which could be a
    function or data).
*/
bool
LtdlModule::HasSymbol( char const * symbolName ) const
{
    return lt_dlsym( m_rep, symbolName );
}


/** Returns true if the LtdlModule object's construction succeeded.
*/
bool
LtdlModule::IsGood() const
{
    return m_isgood;
}


/** Calls a function.  Our convention is that all functions have the same
    signature -- defined in the typedef LtdlModule::FuncSignature.

    Before calling this function, it's a good idea to verify that arg funcName
    is actually defined in the module: call this->HasSymbol() for that.

    Returns 0 if OK.
*/
int
LtdlModule::Apply( char const * funcName, void * funcArg ) const
{
    assert( m_isgood );

    FuncSignature * entrypoint = (FuncSignature *)( lt_dlsym(m_rep, funcName) );
    if( Save_dlerror() )
    {
        cerr << "Error, LtdlModule::Apply(): " << m_errormsg << '\n';
        return 1;
    }

    assert( entrypoint );
    (*entrypoint)( funcArg );
    
    return 0;
}


/** Unload the module and shut down libltdl.
*/
LtdlModule::~LtdlModule()
{
    lt_dlclose(m_rep);
    if( Save_dlerror() )
    {
        cerr << "lt_dlclose(): " << m_errormsg << '\n';
    }

    lt_dlexit();
    if( Save_dlerror() )
    {
        cerr << "lt_dlexit: " << m_errormsg << '\n';
    }
}


/** Calls lt_dlerror and saves its message, if any, in m_errormsg.
    Returns true if there was an error, false otherwise.
    (Gotta save lt_dlerror's return value right away, as it's a static char *
    and might therefore get overwritten by the printf or cout we hope will
    print it.)
*/
bool
LtdlModule::Save_dlerror() const
{
    char *error = (char *)lt_dlerror();
    if( error )
    {
        m_errormsg = std::string( error );
        return true;
    } else
    {
        return false;
    }
}
