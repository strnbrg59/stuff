#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <iostream>
#include <stl.h>

void load( char const * const infile, vector< vector<unsigned char> > * matrix );
void dump( char const * const outfile, vector< vector<unsigned char> > const & matrix );
void testDump();

main( int argc, char* argv[] )
{
//    testDump();

    //
    // Load file.
    //
    char* infile;
    if( argc == 2 )
    {
        infile = argv[1];
    } else
    {
        infile = "strn.jpg";
    }

    vector< vector<unsigned char> > jm;
    load( infile, &jm );
/*
    for( int i=0;i<jm.size();i++ )
    {
        for( int j=0;j<jm[0].size();j++ )
        {
            cout << jm[i][j] << endl;
        }
    }
*/
    //
    // Manipulate file.
    //
/*
    //
    // Dump file (and core :-( )
    //
    for( int i=0;i<30;i++ )
    {
        vector<unsigned char> one_line;
        for( int j=0;j<50;j++ )
        {
            one_line.push_back( (unsigned char)(i+j) );
        }
        jm.push_back( one_line );
    }
    dump( "manipulated.out", jm );
*/
}

void testDump()
{
    JSAMPROW row_pointer[1];
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress( &cinfo );
    FILE * outfile = fopen( "manipulated.out", "wb" );
    if( !outfile ) exit(1);
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = 10;
    cinfo.image_height = 20;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;   //GRAYSCALE;
//  jpeg_set_quality(&cinfo, 1, TRUE );
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    fclose(outfile);
}    

void dump( char const * const outfile_name,
           vector< vector<unsigned char> > const & matrix )
{
    JSAMPROW row_pointer[1];
    unsigned char * image_buffer = new unsigned char[matrix[0].size()];
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jpeg_create_compress( &cinfo );
    FILE * outfile;
    if ((outfile = fopen(outfile_name, "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", outfile_name);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = matrix[0].size();
    cinfo.image_height = matrix.size();
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    jpeg_set_defaults(&cinfo);
    //jpeg_set_quality(&cinfo, quality, TRUE );
    jpeg_start_compress(&cinfo, TRUE);

    for( int i=0;i<cinfo.image_height;i++ )
    {
        for( int j=0;j<cinfo.image_width;j++ )
        {
            image_buffer[j] = matrix[i][j];
        }
        row_pointer[0] = (JSAMPLE*)(& image_buffer);
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
  
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
}


void load( char const * const infile_name, vector< vector<unsigned char> > * matrix )
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    FILE * infile;
    if ((infile = fopen(infile_name, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", infile_name);
        exit(1);
    }
    jpeg_stdio_src(&cinfo, infile);

    jpeg_read_header( &cinfo, TRUE );
    jpeg_start_decompress( &cinfo );

    cout << "width=" << cinfo.image_width << ", height=" << cinfo.image_height
         << endl;
    cout << "width=" << cinfo.output_width << ", height=" << cinfo.output_height
         << "n_colors=" << cinfo.actual_number_of_colors << ",components="
         << cinfo.output_components << endl;

    int row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY output_buffer = (*cinfo.mem->alloc_sarray)
        ( (j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1 );
    while (cinfo.output_scanline < cinfo.output_height)
    {
        vector<unsigned char> one_line;
        (void) jpeg_read_scanlines(&cinfo, output_buffer, 1);
        for( int col=0;col<row_stride;col++ )
        {
            one_line.push_back( output_buffer[0][col] );
        }
        matrix->push_back( one_line );
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
}
