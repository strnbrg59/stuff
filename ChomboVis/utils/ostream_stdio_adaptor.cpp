#include "ostream_stdio_adaptor.h"

// Raw write to the file descriptor.
int
OstreamStdioAdaptor::write( void const * buf, size_t nbytes )
{
    return ::write( m_fildes, buf, nbytes );
}


OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, int i )
{
    osa.StreamOutForBasicType( i );
    return osa;
}

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, float x )
{
    osa.StreamOutForBasicType( x );
    return osa;
}

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, double x )
{
    osa.StreamOutForBasicType( x );
    return osa;
}

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, char const * str )
{
    osa.StreamOutForBasicType( str );
    return osa;
}

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, char c )
{
    osa.StreamOutForBasicType( c );
    return osa;
}

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, std::string str )
{
    return operator<<( osa, str.c_str() );
}


OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, ENDL_T endl_t )
{
    osa.StreamOutForBasicType( endl_t );
    return osa;
}


#ifdef DO_DEMO


#include <iostream>

struct MyStruct
{
    int i;
    int j;
};

template<typename STREAM_T> STREAM_T & operator<<( STREAM_T & ost,
                                                   MyStruct const & mys )
{
    ost << "[" << mys.i << "," << mys.j << "]";
    return ost;
    // Non-template implementation -- making the arg an ostream& -- doesn't
    // work because ostream::operator<<() isn't virtual.
}


int main()
{
    OstreamStdioAdaptor out( 2 );
    out << "Hello world" << '\n' << "Hello again\n" ;

    MyStruct mys;
    mys.i = 17; mys.j = 31;
    out << "Adaptor: mys=" << mys << '\n';
    out << std::endl;

    std::cout << "cout: mys=" << mys << "\n";
}

#endif // DO_DEMO
