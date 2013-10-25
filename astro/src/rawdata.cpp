#include "astro.hpp"
#include "myregexp.hpp"
#include "trace.hpp"
#include "timer.hpp"

#include <algorithm>
#include <climits>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <iostream>
using std::endl;

/** Construct an "empty" one, all pixels set to _default_color. */
RawData::RawData( int row_min, int row_max, int col_min, int col_max,
                  PixelValue default_color )
{
    Trace t("RawData::RawData()");

    // Are row_min and col_min ever other than zero?  Yes.  See
    // RawData::Transform().

    _Resize( row_min, row_max, col_min, col_max );
    for( int i=row_min;i<=row_max;i++ )
    {
        for( int j=col_min;j<=col_max;j++ )
        {
            PixelAt( default_color, i, j );
        }
    }
}


/** Read a PPM file.  PPM format is as follows:
P6
# optional comment
width height
max_pixel_value
data

For example
P6
#Created with The GIMP
724 496
255
^C^C^B [snip]

We assume here that max_pixel_value is always 255.  Each pixel is represented
by three colors, hence there are three chars for each pixel.

We read the file through a pipe.  If the file name ends in ".gz", we do a
"gzip -dc" on it.
*/
RawData::RawData( string ppm_file_name )
{
    Trace t( "RawData::RawData()" );

    // Assert that the file name ends either in ppm.gz or ppm.
    MyRegexp ppm_gz( "\\.ppm\\.gz$" );
    MyRegexp ppm( "\\.ppm$" );
    assert( ppm_gz.matches(ppm_file_name)
    ||      ppm.matches(ppm_file_name) );

    // Read ppm file through popen.  Not, strictly speaking, portable, but fine
    // on gcc + Linux.
    string popen_command;
    if( (ppm_file_name.size() > 3)
    &&  (!strcmp( ppm_file_name.c_str() + ppm_file_name.size()-3, ".gz" )) )
    {
        popen_command = string( "gzip -dc " ) + ppm_file_name;
    } else
    {
        popen_command = string("cat ") + ppm_file_name;
    }
    FILE * c_infile = popen( popen_command.c_str(), "r" );

    // Read "P6".
    size_t max_header_line_length=4096; // Plenty
    char * aline = new char[max_header_line_length+1];
    size_t n = max_header_line_length;
    fgets( aline, n, c_infile );
    if( aline[strlen(aline)-1] == '\n' ) aline[strlen(aline)-1] = 0;
    assert( string(aline) == string("P6") );

    // Read comment lines, if any.
    int peek_ahead = fgetc(c_infile);
    ungetc( peek_ahead, c_infile );
    while( peek_ahead == '#' )
    {
        n = max_header_line_length;
        fgets( aline, n, c_infile );
        if( aline[strlen(aline)-1] == '\n' ) aline[strlen(aline)-1] = 0;
        t.Info( "Read comment line:|%s|.", aline );

        peek_ahead = fgetc(c_infile);
        ungetc( peek_ahead, c_infile );
    }

    // Read dimensions.
    int width, height;
    n = max_header_line_length;
    fgets( aline, n, c_infile );
    char * tok = strtok( aline, " \t\n" ); assert( tok );
    width = atoi( tok );
    tok = strtok(0, " \t\n"); assert( tok );
    height = atoi( tok );
    t.Info( "Read width=%d, height=%d.", width, height );

    // Read max_pixel_value.  We are set up for 255 only, 3 colors per pixel.
    n = max_header_line_length;
    fgets( aline, n, c_infile );
    assert( atoi( aline ) == 255 );
    delete [] aline;

    //
    // Set member data.
    //
    _Resize( 0, height-1, 0, width-1 ); // Sets _row_min and _row_max.
    for( int i=_row_min;i<=_row_max;i++ )
    {
        for( int j=_col_min;j<=_col_max;j++ )
        {
            unsigned char red   = fgetc(c_infile);
            unsigned char green = fgetc(c_infile);
            unsigned char blue  = fgetc(c_infile);
            assert( !feof(c_infile) );

            PixelAt( red + green + blue, i,j );
            // We're throwing away specific color information, just adding
            // up R + G + B.
        }
    }

    pclose( c_infile );
}

/** Resulting *this object has just enough rows and columns to hold all the
 *  transformed coordinates.
*/
void
RawData::Transform( StretchParams const & transforms )
{
    Trace t( "RawData::Transform()" );

//Timer timer1, timer2;
//timer1.Start();

    //
    // Transform all the points, copying their new coordinates into a temporary
    // vector.
    //
    vector<Point<double> > new_points;
    int n( (1+_row_max-_row_min)*(1+_col_max-_col_min) );
    new_points.reserve( n ); // Won't be this many; we only keep the nonzeros.
    Point<double> new_point(0,0);
    for( int i=_row_min;i<=_row_max;i++ )
    {
        for( int j=_col_min;j<=_col_max;j++ )
        {
            if( PixelAt(i,j) > 0 )
            {
                new_point.x(i);
                new_point.y(j);
                new_point.Transform( transforms );
                new_points.push_back( new_point );
            }
        }
    }

//timer1.Stop( "timer1" );
//timer2.Start();

    // Establish size of transformed RawData.
    vector<int> i_coords, j_coords;
    i_coords.reserve(4);
    j_coords.reserve(4);
    for( int i=_row_min; i<=_row_max; i+=(_row_max-_row_min) )
    {
        for( int j=_col_min; j<=_col_max; j+=(_col_max-_col_min) )
        {
            Point<double> corner_point(i,j);
            corner_point.Transform( transforms );
            i_coords.push_back( int(corner_point.x() + 0.5) );
            j_coords.push_back( int(corner_point.y() + 0.5) );
        }
    }

    // Create an appropriately resized RawData.  We'll put our new points into
    // it and, in the end, copy it to *this.
    RawData resized_this( *min_element( i_coords.begin(), i_coords.end()),
                          *max_element( i_coords.begin(), i_coords.end()),
                          *min_element( j_coords.begin(), j_coords.end()),
                          *max_element( j_coords.begin(), j_coords.end()) );

    int elem(0);
    for( int i=_row_min;i<=_row_max;i++ )
    {
        for( int j=_col_min;j<=_col_max;j++ )
        {
            if( PixelAt(i,j) > 0 )
            {
                Point<double> newpoint( new_points[elem] );
                resized_this.PixelAt(
                    PixelAt(i,j),
                    int( newpoint.x() + 0.5 ),
                    int( newpoint.y() + 0.5 ) );
                    // FIXME: you're losing information when you round this way.
                    // Instead distribute the energy over a few adjacent pixels.
                ++elem;
            }
        }
    }
    assert( unsigned(elem) == new_points.size() );

    Copy( resized_this );
//timer2.Stop("timer2");
}

/** Add pixel values over all (i,j) coordinates that may appear in either
 *  RawData object.
 *
 *  If, in summing, we obtain pixel values over 255, we store the sum as is.
 *  We rely on Rescale() to take care of the final product.
*/
void
RawData::operator+=( RawData const & that )
{
    // Allocate a RawData that's as large as the union of *this and that.
    int min_i = std::min( RowMin(), that.RowMin() );
    int max_i = std::max( RowMax(), that.RowMax() );
    int min_j = std::min( ColMin(), that.ColMin() );
    int max_j = std::max( ColMax(), that.ColMax() );
    RawData union_data( min_i, max_i, min_j, max_j );

    // Add
    for( int i=min_i; i<=max_i; i++ )
    {
        for( int j=min_j; j<=max_j; j++ )
        {
            int sum = 0;
            if( i >= RowMin() && i <= RowMax()
            &&  j >= ColMin() && j <= ColMax() )
            {
                sum += PixelAt( i,j );
            }
            if( i >= that.RowMin() && i <= that.RowMax()
            &&  j >= that.ColMin() && j <= that.ColMax() )
            {
                sum += that.PixelAt( i,j );
            }

            union_data.PixelAt( sum, i, j );
        }
    }

    // Copy
    Copy( union_data );        
}

/** For subtracting dark frames, mostly.  Unlike operator+=, assumes the two
 *  RawData objects are the same size.
 *  If any subtractions result in negative values, leave them at zero.  But
 *  report the fact you got negative values (shouldn't happen).
*/
void
RawData::operator-=( RawData const & that )
{
    int n_negative_results;
    assert(this->SameSize(that));

    for( int i=RowMin(); i<RowMax(); ++i )
    {
        for( int j=ColMin(); j<ColMax(); ++j )
        {
            int diff = PixelAt( i,j ) - that.PixelAt( i,j );
            if( diff < 0 )
            {
                ++n_negative_results;
                PixelAt( 0, i,j );
            } else
            {
                PixelAt( diff, i,j );
            }
        }
    }
}        


/** Deal with pixel values greater than 255 (which may result from additions
 *  of RawData objects).
 *  Arg mode governs what we do:
 *    mode=trim => truncate at 255.
 *    mode=bleed => displace excess (>255) values onto neighboring pixels,
 *        thus obtaining larger star images.  (Not implemented yet.)
 *    mode=normalize => divide all pixel values by the maximum, take
 *        a power less than 1, multiply by 255.
 *  Arg n_layers is relevant only for normalize mode.
*/
void
RawData::Rescale( RescaleMode mode, int n_layers=0 )
{
    switch( mode )
    {
        case trim:
        {
            for( int i=_row_min; i<=_row_max; i++ )
            {
                for( int j=_col_min; j<=_col_max; j++ )
                {
                    PixelAt( std::min(PixelAt(i,j),(PixelValue)255), i,j );
                }
            }
            break;
        }
        case bleed: break;
        case normalize:
        {
            assert( n_layers );
            // This scaling_power ensures that a pixel that had a value of 1
            // in just one layer, and 0 in all the other layers, will be output
            // as 1.
            double scaling_power = 1/(1 + log(n_layers)/log(255));

            int max_val = INT_MIN;
            for( int i=_row_min; i<=_row_max; i++ )
            {
                for( int j=_col_min; j<=_col_max; j++ )
                {
                    max_val = std::max( max_val, PixelAt(i,j) );
                }
            }
            for( int i=_row_min; i<=_row_max; i++ )
            {
                for( int j=_col_min; j<=_col_max; j++ )
                {
                    int new_val = 
                        int( pow( PixelAt(i,j)/(max_val+0.0), scaling_power ) *
                             255 + 0.5 );
                    PixelAt( new_val, i,j );
                }
            }
            break;
        }
    }
}

/** Replace all the data with those of that.  Resize if necessary. */
void
RawData::Copy( RawData const & that )
{
    Trace t("RawData::Copy");
    int min_i = that.RowMin();
    int max_i = that.RowMax();
    int min_j = that.ColMin();
    int max_j = that.ColMax();

    _Resize( min_i, max_i, min_j, max_j  );

    for( int i=_row_min;i<=_row_max;i++ )
    {
        for( int j=_col_min;j<=_col_max;j++ )
        {
            PixelAt( that.PixelAt(i,j), i,j );
        }
    }
    t.Info( "Read width=%d, height=%d.", 1+max_j-min_j, 1+max_i-min_i);
}

/** Getter */
PixelValue
RawData::PixelAt( int i, int j ) const
{
    assert( i>=RowMin() && i<=RowMax() && j>=ColMin() && j<=ColMax() );
    return _rep[ i - _row_min ][ j - _col_min ];
}

/** Setter */
void
RawData::PixelAt( PixelValue color, int i, int j )
{
#ifndef NDEBUG
    Trace t("RawData::PixelAt()");
    if( !( i>=RowMin() && i<=RowMax() && j>=ColMin() && j<=ColMax() ) )
    {
        t.FatalError("(i,j)=(%d,%d), (row_min,row_max,col_min,col_max)="
                     "(%d,%d,%d,%d)", i,j,RowMin(),RowMax(),ColMin(),ColMax());
    }
#endif
    _rep[ i - _row_min ][ j - _col_min ] = color;
}

void
RawData::_Resize( int row_min, int row_max, int col_min, int col_max )
{
    _rep.resize( row_max - row_min + 1 );
    for( int i=0; i <= row_max - row_min; i++ )
    {
        _rep[i].resize( col_max - col_min + 1 );
    }
    _row_min = row_min;
    _row_max = row_max;
    _col_min = col_min;
    _col_max = col_max;
}

/** Dump data in PPM and view with xv. */
void
RawData::Display() const
{
    std::ofstream outfile( "/tmp/astro_new.ppm" );
    outfile << *this;

    int pid = fork();
    if( pid == 0 )
    {
        system( "xv /tmp/astro_new.ppm" );
        exit(0);
    }
}


/** Dump the data in the format that RawData's ctor understands. */
// See http://netpbm.sourceforge.net/doc/ppm.html, for the ppm format spec.
// FIXME: We're changing to PPM format.  RawData's ctor doesn't understand that.
std::ostream&
operator<<( std::ostream & out, RawData const & raw_data )
{
    Trace t( "operator<<" );

    out << "P3" << endl;  // magic number for "plain" ppm format.
    out << 1 + raw_data.ColMax() - raw_data.ColMin() << " "
        << 1 + raw_data.RowMax() - raw_data.RowMin() << endl;
    out << 255 << endl;

    int row_width = 0;
    for( int i=raw_data.RowMin();i<=raw_data.RowMax();i++ )
    {
        for( int j=raw_data.ColMin();j<=raw_data.ColMax();j++ )
        {
            out << int(raw_data.PixelAt( i,j )) << " "
                << int(raw_data.PixelAt( i,j )) << " "
                << int(raw_data.PixelAt( i,j )) << " ";
            row_width += 3 * (1 + 3);
            if( row_width > 40 )
            {
                out << endl;
                row_width = 0;
            }
        }
        out << endl;
    }

    return out;
}


/** Returns true iff *this and that have same {row,col}{min,max}. */
bool
RawData::SameSize( RawData const& that) const
{
    return ( RowMin() == that.RowMin() ) &&
           ( RowMax() == that.RowMax() ) &&
           ( ColMin() == that.ColMin() ) &&
           ( ColMax() == that.ColMax() );
}

