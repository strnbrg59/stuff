#ifndef INCLUDED_TRACE_HPP
#define INCLUDED_TRACE_HPP

/***********************************************************************
CmdlineBase::DebugLevel() determines what prints.
  DebugLevel() = 0 => nothing
                 1 => only FatalError or FatalSystemError
                 2 => Error or SystemError
                 3 => Warning
                 4 => Info
                 5 => Indented trace of function entries and exits.
************************************************************************/

#include <iostream>
#include <fstream>

class TraceStream
{
  public:
    TraceStream( char const * function_name );
    void SetDebugLevel( int d ) { m_debug_level = d; }
    char const * FunctionName() const { return m_function_name; }
    static void SetOstream( char const * filename );
    
  private:
    static std::ostream * m_ost;
    char const * m_function_name;
    int m_debug_level;

  template<typename T> friend TraceStream & operator<<( TraceStream &,
                                                        T const & );
};
template<typename T> TraceStream & operator<<( TraceStream &, T const & );


class Trace
{
  public:
    Trace( char const * funcname );
    ~Trace();

    TraceStream & Info();
    TraceStream & Notice();
    TraceStream & Warning();
    TraceStream & Error();

    void SetOstream( char const * filename );

    void FatalError( char const * format, ... );
    void Error( char const * format, ... );
    void Warning( char const * format, ... );
    void Notice( char const * format, ... );
    void Info( char const * format, ... );

    void noop() {}

  private:
    void indent();

    char const * m_funcname;
    TraceStream m_trace_stream;

    static int s_nesting_depth;
};

#include "trace.cpp"  // Will take in only templatized part of TraceStream impl.

#endif // INCLUDED_TRACE_HPP

