/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/
// Author: Ted Sternberg

#include "cmdline.h"
#include "Trace.h"
#include <iostream>
#include <stdio.h>
#include <cassert>
#include <cstdarg>

using std::cerr;
using std::endl;

Trace::Trace( char const * funcname )
  : m_funcname(funcname),
    m_trace_stream( std::cerr, funcname )
{
    if( CmdlineBase::DebugLevel() < 5 ) return;

    indent();
    s_nesting_depth ++;
    cerr << "Entering " << funcname << endl;
}

Trace::~Trace()
{
    if( CmdlineBase::DebugLevel() < 5 ) return;

    s_nesting_depth --;
    indent();
    cerr << "Exiting " << m_funcname << endl;
}

void
Trace::indent()
{
    for( int i=0;i<s_nesting_depth;i++ ) cerr << "  ";
}        

void
Trace::FatalSystemError( char const * format, ... )
{
    indent();
    if( CmdlineBase::DebugLevel() < 1 ) return;
    fprintf( stderr, "Fatal System Error: " );
    if( CmdlineBase::DebugLevel() < 5 )
    {
        cerr << "(" << m_funcname << "): ";
    }
    perror( "" );

    va_list ap;
    va_start( ap, format );
    
    vfprintf( stderr, format, ap );
    fprintf( stderr, "\n" );

    assert(0); // We want a stack trace.
}

void
Trace::FatalError( char const * format, ... )
{
    indent();
    if( CmdlineBase::DebugLevel() < 1 ) return;
    cerr << "Fatal Error (" << m_funcname << "): ";

    va_list ap;
    va_start( ap, format );
    
    vfprintf( stderr, format, ap );
    fprintf( stderr, "\n" );

    assert(0); // We want a stack trace.
}

void
Trace::SystemError( char const * format, ... )
{
    indent();
    if( CmdlineBase::DebugLevel() < 2 ) return;

    fprintf( stderr, "System Error" );
    if( CmdlineBase::DebugLevel() < 5 )
    {
        cerr << "(" << m_funcname << "): ";
    }
    perror( "" );

    va_list ap;
    va_start( ap, format );
    
    vfprintf( stderr, format, ap );
    fprintf( stderr, "\n" );
}

void
Trace::Error( char const * format, ... )
{
    indent();
    if( CmdlineBase::DebugLevel() < 2 ) return;

    cerr << "Error (" << m_funcname << "): ";
    va_list ap;
    va_start( ap, format );
    
    vfprintf( stderr, format, ap );
    fprintf( stderr, "\n" );
}

void
Trace::Warning( char const * format, ... )
{
    indent();
    if( CmdlineBase::DebugLevel() < 3 ) return;

    cerr << "Warning ";
    if( CmdlineBase::DebugLevel() < 5 )
    {
        cerr << "(" << m_funcname << "): ";
    }

    va_list ap;
    va_start( ap, format );
    
    vfprintf( stderr, format, ap );
    fprintf( stderr, "\n" );
}

void
Trace::Info( char const * format, ... )
{
    indent();
    if( CmdlineBase::DebugLevel() < 4 ) return;

    cerr << "Info ";
    if( CmdlineBase::DebugLevel() < 5 )
    {
        cerr << "(" << m_funcname << "): ";
    }

    va_list ap;
    va_start( ap, format );
    
    vfprintf( stderr, format, ap );
    fprintf( stderr, "\n" );
}


TraceStream & Trace::Info()
{
    indent();
    m_trace_stream.SetDebugLevel( 4 );
    m_trace_stream << "Info: " << m_trace_stream.FunctionName() << ":";
    return m_trace_stream;
}
TraceStream & Trace::Warning()
{
    indent();
    m_trace_stream.SetDebugLevel( 3 );
    m_trace_stream << "Warning: " << m_trace_stream.FunctionName() << ":";
    return m_trace_stream;
}
TraceStream & Trace::Error()
{
    indent();
    m_trace_stream.SetDebugLevel( 2 );
    m_trace_stream << "Error: " << m_trace_stream.FunctionName() << ":";
    return m_trace_stream;
}
//No FatalError(), because we don't have a good way to call exit().  Use the
//C-style FatalError( char *, ...) version instead.


int Trace::s_nesting_depth = 0;

//
//---------------------------------------------------------
//

TraceStream::TraceStream( std::ostream & ost, char const * function_name )
  : _ost( ost ),
    _function_name( function_name )
{
}
