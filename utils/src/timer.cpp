/**
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
/** Timer object, for timing code execution.
 *  
 *  Legal usage patterns:
 *    Start(), Stop().
 *    Start(), [Pause(), Resume()]*, Stop().
 *    Start(), Pause(), Stop() is OK too.
 */

#include "timer.hpp"
#include <cassert>
#include <sys/time.h>
#include <iostream>
#include <cmath>

#ifndef DO_DEMO
    #include "trace.hpp"
#endif

using std::cerr;
using std::endl;

bool Timer::s_silent( false );

Timer::Timer( std::ostream & outstream )
  : m_outstream( outstream )
{
    m_state = constructed;
}

/** Set to true to suppress printing. */
void
Timer::SetSilent( bool b )
{
    s_silent = b;
}

/** Resets clock. */
void
Timer::Start()
{
    Trace t("Timer::Start()");
    if( m_state != constructed && m_state != stop )
    {
        t.FatalError( "Entered in illegal state: %d.", m_state );
    }

    m_state = start;

    m_accumulated_milliseconds = 0;

    gettimeofday( &m_timeval, 0 );
    m_prev_sec = m_timeval.tv_sec;
    m_prev_usec = m_timeval.tv_usec;
}

/** Prints time, in milliseconds, elapsed since last call to Start().  In
 *  front of the time, prints arg message.
 *  Should be followed by, if anything, a call to Start().
*/
void
Timer::Stop( char const * message )
{
    if( m_state != pause )
    {
        AccumulateTime();
    }

    Trace t("Timer::Stop()");
    if( m_state != start && m_state != resume && m_state != pause )
    {
        t.FatalError( "Entered in illegal state: %d.", m_state );
    }

    m_state = stop;

    if( s_silent == false )
    {
        m_outstream << message << ": " << m_accumulated_milliseconds << endl;
    }
}

/** Pauses the clock.  Doesn't reset it.  Should be followed by a call to
 *  Resume().
 */
void
Timer::Pause( char const * message )
{
    Trace t("Timer::Pause()");

    if( m_state != pause )
    {
        AccumulateTime();
    }

    if( m_state != start && m_state != resume && m_state != pause )
    {
        t.FatalError( "Entered in illegal state: %d.", m_state );
    }

    if( (s_silent == false) && (message != 0) )
    {
        m_outstream << message << ": " << m_accumulated_milliseconds << endl;
    }

    m_state = pause;
}

/** Private member.  Convenience function called from Stop() and Pause(). */
void
Timer::AccumulateTime()
{
    gettimeofday( &m_timeval, 0 );
    long sec_now = m_timeval.tv_sec;
    long usec_now = m_timeval.tv_usec;

    m_accumulated_milliseconds +=
        1000 *  (sec_now - m_prev_sec )
        + 0.001 * (usec_now - m_prev_usec );
}

/** Restarts the clock, from its position at the last Pause(). */
void
Timer::Resume()
{
    Trace t("Timer::Resume()");
    if( m_state != pause )
    {
        t.FatalError( "Entered in illegal state: %d.", m_state );
    }

    m_state = resume;

    gettimeofday( &m_timeval, 0 );
    m_prev_sec = m_timeval.tv_sec;
    m_prev_usec = m_timeval.tv_usec;
}


/** Subtract accumulated time. */
Timer &
Timer::operator-=( Timer const & that )
{
    m_accumulated_milliseconds -= that.m_accumulated_milliseconds;
    return *this;
}  


#ifdef DO_DEMO
#include <fstream>

// Build this with "g++ -DDO_DEMO Timer.cpp
int main()
{
    // TimerContext does its best to net out overhead anyway, so this
    // "second-order" overhead timer may well report negative time.
    static TimerContext s_overheadTimerContext( "overhead" );

    static TimerContext s_timerContext1( "timer1" );
    static Timer        s_timer;

    int const n = 2000;

    s_timer.Start();

    double x;
    for( int i=0;i<n;++i )
    {
        s_overheadTimerContext.Resume();
        s_overheadTimerContext.Pause();

        s_timerContext1.Resume();
        for( int j=0;j<n;++j )
        {
            if( (i*j)%11791 == 0 )
            {
                x = pow(2.1, (i*j)%28);
                if( x == 17 ) cerr << "Very unlikely event." << endl;
            }
        }
        s_timerContext1.Pause();
    }

    s_timer.Stop( "total time" );

    return 0;
}
#endif // DO_DEMO
