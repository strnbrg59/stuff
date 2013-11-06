#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

// Break buf up into '\n'-delimited chunks.
// Whatever is left that doesn't end in a '\n', copy to the front of buf
// and return the length of that leftover piece.
int Chunk( char * buf )
{
    char * slash_n;
    int pos = 0;
    int n( strlen(buf) );
    while( (pos<n) && (slash_n = strpbrk( buf+pos, "\n" )) != 0 )
    {
        *slash_n = 0;
        printf( "buf+pos=|%s|\n", buf+pos );
        pos += 1 + slash_n - (buf+pos);
    }

    if( pos == 0 )
    {
        return n;
    } else
    if( pos < n )
    {
        for( int i=0;i<n-pos;++i )
        {
            buf[i] = buf[pos+i];
        }
        return n-pos;
    } else
    if( pos == n )
    {
        return 0;
    } else
    {
        fprintf( stderr, "Error: pos(%d)>n(%d)\n", pos, n ); 
        assert(0);
    }
}

int main( int argc, char * * argv )
{
    char * buf = new char[100];
    int leftover(0);

    for( int i=1; i<argc; ++i )
    {
        strcpy( buf+leftover, argv[i] );
        char * tbuf = buf;
        while( *tbuf )
        {
            if( *tbuf == 'z' ) *tbuf = '\n';
            ++tbuf;
        }
    
        leftover = Chunk( buf );
        printf("------------------\n");
    }

    delete [] buf;
    return 0;
}
