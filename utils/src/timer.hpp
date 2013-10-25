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
#ifndef DEFINED_TIMER_H
#define DEFINED_TIMER_H

#include <sys/time.h>
#include <iostream>

/** Timer object, for timing code execution. */
class Timer
{
  public:
    Timer( std::ostream & = std::cerr );
    void Start();
    void Pause( char const * message = 0 );
    void Resume();
    void Stop( char const * message );

    static void SetSilent( bool );

    Timer & operator-=( Timer const & that );
  private:
    long m_prev_sec;  // seconds
    long m_prev_usec; // microseconds
    float m_accumulated_milliseconds;
    struct timeval m_timeval;

    void AccumulateTime();

    enum State { constructed=0, start=1, pause=2, resume=3, stop=4 };
    State m_state;

    static bool s_silent;
    std::ostream & m_outstream;
};


/**
    If you need the timer to resume and pause in a loop, and you're not sure
    where in the code it's really going to be finished, use this class.

    Uses an extra internal Timer to net out the overhead of Resume()+Pause().
*/
class TimerContext
{
  public:
    TimerContext( char const * id ) : m_id( id )
    {
        m_timer.Start();
        m_timer.Pause();

        m_overheadTimer.Start();
        m_overheadTimer.Pause();
    }

    ~TimerContext()
    {
        m_timer -= m_overheadTimer;
        m_timer.Stop( m_id.c_str() );
    }

    void Resume() { m_timer.Resume(); }
    void Pause() 
    {
        m_timer.Pause();
        m_overheadTimer.Resume();
        m_overheadTimer.Pause();
    }

  private:
    std::string m_id;
    Timer m_timer;
    Timer m_overheadTimer;
};


#ifdef DO_DEMO // for unit test

class Trace // Do-nothing version
{
  public:
    Trace( char const * funcname ) {}

    void FatalError( char const * format, ... ) {}
    void FatalSystemError( char const * format, ... ) {}
    void Error( char const * format, ... ) {}
    void SystemError( char const * format, ... ) {}
    void Warning( char const * format, ... ) {}
    void Info( char const * format, ... ) {}
  private:
    static int s_nesting_depth;
};

#endif // DO_DEMO

#endif // DEFINED_TIMER_H
