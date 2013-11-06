#include "exceptions.hpp"
#include "trace.hpp"
#include <fstream>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#define min(a,b) a < b ? a : b

//
// Exceptions
//

//
// Electric Fence note: you might need to "export EF_ALLOW_MALLOC_0=1"
// to avoid a bogus "illegal instruction" error.
//

Exception::Exception()
{
    memset( m_msg, 0, m_max_msg_length );
}

Exception::Exception( char const * format, ... )
{
    memset( m_msg, 0, m_max_msg_length+1 );

    va_list ap;
    va_start( ap, format );
    int nc = vsnprintf( m_msg, m_max_msg_length, format, ap );
    m_msg[ min( m_max_msg_length, nc ) ] = 0;
}

EofException::EofException( char const * format, ... )
  : Exception()
{
    // Because of the varargs, we have no choice but to cut and paste
    // in the implementation of each Exception subclass.
    va_list ap;
    va_start( ap, format );
    int nc = vsnprintf( m_msg, m_max_msg_length, format, ap );
    m_msg[ min( m_max_msg_length, nc ) ] = 0;
}

ReadErrorException::ReadErrorException( char const * format, ... )
  : Exception()
{
    va_list ap;
    va_start( ap, format );
    int nc = vsnprintf( m_msg, m_max_msg_length, format, ap );
    m_msg[ min( m_max_msg_length, nc ) ] = 0;
}

TimeoutException::TimeoutException( char const * format, ... )
  : Exception()
{
    va_list ap;
    va_start( ap, format );
    int nc = vsnprintf( m_msg, m_max_msg_length, format, ap );
    m_msg[ min( m_max_msg_length, nc ) ] = 0;
}

MiscException::MiscException( char const * format, ... )
  : Exception()
{
    va_list ap;
    va_start( ap, format );
    int nc = vsnprintf( m_msg, m_max_msg_length, format, ap );
    m_msg[ min( m_max_msg_length, nc ) ] = 0;
}

UnexpectedServerBehaviorException::UnexpectedServerBehaviorException( 
  char const * format, ... )
  : Exception()
{
    va_list ap;
    va_start( ap, format );
    int nc = vsnprintf( m_msg, m_max_msg_length, format, ap );
    m_msg[ min( m_max_msg_length, nc ) ] = 0;
}


std::ostream&
operator<<( std::ostream& o, Exception e )
{
    o << e.m_msg;
    return o;
}

#undef min
