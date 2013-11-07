// A class that works like an ostream but is associated with a file
// descriptor and passes all its stuff right through onto that file descriptor.

#ifndef INCLUDED_OSTREAM_STDIO_ADAPTOR_H
#define INCLUDED_OSTREAM_STDIO_ADAPTOR_H

#include <sstream>
#include <string>
#include <unistd.h>


/**
    Can be used as an ostream, but it's associated with a file descriptor on
    which it writes its output.  So this lets you pipe stream I/O to a child
    process.  This sort of stuff (see VisualizableDataset.hImpl version
    1.19)...
        c_outfile = fdopen( fd[1], "w" );
        FILEBUF ofb( c_outfile, std::ios_base::out );
        std::ostream outfile( & ofb );
    ...stopped working with gcc3.4.2.

    For integral types and char const * we actually define operator<< to do
    something special; it writes to a stringstream which it then writes onto
    the file descriptor.  For all other classes, write operator<< generically,
    so it can be passed either an ordinary ostream, or an OstreamStdioAdaptor.
    E.g. if you have a function
    ostream & operator<<( ostream &, IntVect const & ),
    rewrite it like so:

    template<typename OSTREAM_T> OSTREAM_T &
    operator<<( OSTREAM_T & out, IntVect const & iv )
    {
        out << "[" << iv.i << ',' << iv.j << "]"; // format to taste
        return out;
    }

*/
class OstreamStdioAdaptor
{
  public:
    OstreamStdioAdaptor( int fildes )
      : m_fildes( fildes )
    {
    }

    void precision( int p ) { m_strstream.precision(p); }

    ///
    /** For int, float, double, char const *
    */
    template<typename T> OstreamStdioAdaptor &
        StreamOutForBasicType( T const & );

    int write( void const * buf, size_t nbytes );

  private:
    std::stringstream m_strstream;
    int               m_fildes;
};


OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, int i );

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, float x );

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, double x );

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, char const * str );

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, char c );

OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, std::string str );

typedef std::ostream & (*ENDL_T)(std::ostream&); // endl, ends, flush...
OstreamStdioAdaptor &
operator<<( OstreamStdioAdaptor & osa, ENDL_T endl_t );

template<typename T> OstreamStdioAdaptor &
OstreamStdioAdaptor::StreamOutForBasicType( T const & t )
{
    this->m_strstream << t;
    int size = this->m_strstream.str().size();
    ::write( this->m_fildes, this->m_strstream.str().c_str(), size );
    this->m_strstream.str("");
    return *this;
}

#endif // INCLUDED_OSTREAM_STDIO_ADAPTOR_H
