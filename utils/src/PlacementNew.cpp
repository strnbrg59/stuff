#ifndef INCLUDED_PLACEMENT_NEW_CPP
#define INCLUDED_PLACEMENT_NEW_CPP

#include "Trace.h"
#include "PlacementNew.h"
#include <cmath>
#include <new.h>

PlacementNew::PlacementNew( int bufQuantum )
  : m_bufQuantum( bufQuantum ), // change to suit -- no dependencies
    m_lastBufSize(0),
    m_lastBufPosition(0)
{
    Trace t("PlacementNew::PlacementNew()");
}


PlacementNew::~PlacementNew()
{
    for( vector<char *>::iterator i = m_bufs.begin();
         i != m_bufs.end();
         ++i )
    {
        delete [] *i;
    }
}


template<class T> T *
PlacementNew::New( T obj )
{
    Trace t("PlacementNew::New()");

    int objSize = sizeof( T );
    //t.Info( "sizeof(T)=%d", objSize );

    if( m_lastBufPosition + objSize > m_lastBufSize )
    {   // Create a new memory buffer, and push it onto the buffer vector.
        m_lastBufSize =
            int(ceil((objSize+0.0)/m_bufQuantum))
          * m_bufQuantum;
        t.Info( "new buffer size=%d.", m_lastBufSize );
        m_lastBuf = new char[ m_lastBufSize ];
        if( ! m_lastBuf )
        {
            t.FatalError( "Out of memory." );
        }
        m_bufs.push_back( m_lastBuf );
        m_lastBufPosition = 0;
    }

    T * result = new( m_lastBuf + m_lastBufPosition ) T(obj);
    m_lastBufPosition += objSize;
    return result;
}

void
PlacementNew::UnitTest()
{
    Trace t("PlacementNew::UnitTest()");
    t.Info( "Calling New(17)..." );
    New(17);
    t.Info( "Calling New(3.14)..." );
    New(3.14);
    t.Info( "Calling New(PlacementNew(10))..." );
    PlacementNew foobar(10);
    New( foobar );
}

#endif
