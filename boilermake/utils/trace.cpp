// This is not a standard include guard.  It's intended to *exclude* everything
// in this file *except* the templatized part of the implementation of class
// TraceStream.  Notice the matching #endif for this occurs just before we begin
// with implementation of the templatized part of TraceStream.
#ifndef INCLUDED_TRACE_HPP

#include "cmdline_base.hpp"
#include "trace.hpp"
#include <cassert>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
using std::cout;
using std::cerr;
using std::endl;

Trace::Trace( char const * funcname )
  : m_funcname(funcname),
    m_trace_stream( funcname )
{
    if( CmdlineBase::DebugLevel() < 5 ) return;

    indent();
    s_nesting_depth ++;
    cout << "Entering " << funcname << endl;
}

Trace::~Trace()
{
    if( CmdlineBase::DebugLevel() < 5 ) return;

    s_nesting_depth --;
    indent();
    cout << "Exiting " << m_funcname << endl;
}

void
Trace::indent()
{
    for( int i=0;i<s_nesting_depth;i++ ) cout << "  ";
}        

void
Trace::SetOstream( char const * filename )
{
    m_trace_stream.SetOstream( filename );
}

void
Trace::FatalSystemError( char const * format, ... )
{
    if( CmdlineBase::DebugLevel() < 1 ) return;

    indent();
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
    exit(1);
}

void
Trace::FatalError( char const * format, ... )
{
    if( CmdlineBase::DebugLevel() < 1 ) return;

    indent();
    cerr << "Fatal Error (" << m_funcname << "): ";

    va_list ap;
    va_start( ap, format );
    
    vfprintf( stderr, format, ap );
    fprintf( stderr, "\n" );
    exit(1);
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


/*static*/ void
TraceStream::SetOstream( char const * filename )
{
    // Making this static will guarantee that we'll be able to use TraceStream
    // at least until exit().  But if destructors for (other) static objects
    // try to use TraceStream, nothing's guaranteed.
    static std::ofstream outfile( filename );
    assert( outfile );

    // No calling this function more than once; we don't want to deal with the
    // complications.
    assert( m_ost == &std::cerr );

    m_ost = &outfile;
}


TraceStream & Trace::Info()
{
    indent();
    m_trace_stream.SetDebugLevel( 4 );
    m_trace_stream << "Info: " << m_trace_stream.FunctionName() << " ";
    return m_trace_stream;
}
TraceStream & Trace::Warning()
{
    indent();
    m_trace_stream.SetDebugLevel( 3 );
    return m_trace_stream;
}
TraceStream & Trace::Error()
{
    indent();
    m_trace_stream.SetDebugLevel( 2 );
    return m_trace_stream;
}
//No FatalError(), because we don't have a good way to call exit().  Use the
//C-style FatalError( char *, ...) version instead.


int Trace::s_nesting_depth = 0;

//
//---------------------------------------------------------
//

TraceStream::TraceStream( char const * function_name )
  : m_function_name( function_name )
{
}

/*static*/ std::ostream * TraceStream::m_ost = &std::cerr;

#else // INCLUDED_TRACE_HPP
#include "cmdline_base.hpp"

template< typename T> TraceStream &
operator<<( TraceStream & tstr, T const & t )
{
    if( CmdlineBase::DebugLevel() >= tstr.m_debug_level )
    {
        (*tstr.m_ost) << t;
    }
    return tstr;
}

#endif // INCLUDED_TRACE_HPP
