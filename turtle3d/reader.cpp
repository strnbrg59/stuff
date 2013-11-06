// Author: Ted Sternberg, strnbrg@trhj.homeunix.net

#include "reader.hpp"

#include <unistd.h>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <string>
#include <iostream>

using std::string;
using std::vector;
using std::cerr;
using std::cout;
using std::cin;
using std::endl;

static std::vector<std::string> Chunk( char * buf, int * leftover );

static Reader * s_reader(0);
Reader * TheReader()
{
    if( ! s_reader )
    {
        s_reader = new Reader;
    }
    return s_reader;
}

vector<string>
Reader::ReadCommands()
{
    vector<string> result;
    int bytesRead;
    struct timeval timeout;
    timeout.tv_sec = timeout.tv_usec = 0;

    fd_set readfds;
    FD_ZERO( &readfds );
    FD_SET( STDIN_FILENO, &readfds );    

    int ret = select( STDIN_FILENO + 1, &readfds, 0, 0, &timeout );
    if( ret == -1 )
    {
        cerr << "Error: select()" << endl;
        return result;
    } else
    if( ret == 0 )  // timed out
    {
        return result;
    } else
    {
        assert( FD_ISSET( STDIN_FILENO, &readfds ) );
        bytesRead = read( STDIN_FILENO, m_buf+m_leftover,
                                        m_maxBytes-m_leftover-1 );
        if( bytesRead < 0 )
        {
            cerr << "Error: read()" << endl;
        } else
        if( bytesRead == 0 )
        {
            return result;
        } else
        if( bytesRead > 0 )
        {
            m_buf[bytesRead+m_leftover] = 0;
            result =  Chunk( m_buf, &m_leftover);
            return result;
        }
    }
    return result;
}


/** Whatever is left that doesn't end in a '\n', copy to the front of buf
 *  and return the length of that leftover piece.
 *
 *  Returns a vector, each of whose elements was a '\n'-delimited piece of
 *  arg buf.
 *  Sets arg leftover to the length of the string, at the end of buf, that
 *  didn't end in '\n' (and which got copied to the front of buf).
*/
vector<string> Chunk( char * buf, int * leftover )
{
    vector<string> result;

    char * slash_n;
    int pos = 0;
    int n( strlen(buf) );
    while( (pos<n) && (slash_n = strpbrk( buf+pos, "\n" )) != 0 )
    {
        *slash_n = 0;
        string cmd( buf+pos );
        result.push_back( cmd );
        pos += 1 + slash_n - (buf+pos);
    }

    if( pos == 0 )
    {
        *leftover = n;
    } else
    if( pos < n )
    {
        for( int i=0;i<n-pos;++i )
        {
            buf[i] = buf[pos+i];
        }
        *leftover = n-pos;
    } else
    if( pos == n )
    {
        *leftover = 0;
    } else
    {
        fprintf( stderr, "Error: pos(%d)>n(%d)\n", pos, n );
        assert(0);
    }

    return result;
}
