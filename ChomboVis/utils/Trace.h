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
#ifndef INCLUDED_TRACE_HPP
#define INCLUDED_TRACE_HPP

/***********************************************************************
Cmdline::DebugLevel() determines what prints.
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
    TraceStream( std::ostream &, char const * function_name );
    void SetDebugLevel( int d ) { _debug_level = d; }
    int  GetDebugLevel() const { return _debug_level; }
    char const * FunctionName() const { return _function_name; }
    
  private:
    std::ostream & _ost;
    char const * _function_name;
    int _debug_level;

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
    TraceStream & Warning();
    TraceStream & Error();

    int  GetDebugLevel() const { return m_trace_stream.GetDebugLevel(); }
    void SetDebugLevel( int d ) { m_trace_stream.SetDebugLevel(d); }

    void FatalError( char const * format, ... );
    void FatalSystemError( char const * format, ... );
    void Error( char const * format, ... );
    void SystemError( char const * format, ... );
    void Warning( char const * format, ... );
    void Info( char const * format, ... );

    void NoOp() { } // To avoid unused-variable compiler warnings.

  private:
    void indent();

    char const * m_funcname;
    TraceStream m_trace_stream;
    static int s_nesting_depth;
};

#include "Trace.hImpl"

#endif // INCLUDED_TRACE_HPP
