#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>

void dump( char const * const outfileName,
           unsigned char * * matrix, int nRows, int nCols );

main( int argc, char* argv[] )
{
    unsigned char * * matrix;
    int nRows, nCols;
    int i, j;

    /* Initialize data we want to make a jpeg of. */
    nRows = 75;
    nCols = 150;
    matrix = (unsigned char * *)malloc( nRows * sizeof(unsigned char *) );
    for( i=0;i<nRows;i++ )
    {
        matrix[i] = (unsigned char *)malloc( nCols * sizeof(unsigned char) );
    }

    for( i=0;i<nRows;i++ )
    {
        for( j=0;j<nCols;j++ )
        {
            matrix[i][j] = i + j;
            assert( matrix[i][j] < 255 );
        }
    }

    //
    // Dump file.
    //
    dump( "manipulated.out", matrix, nRows, nCols );
}

void dump( char const * const outfileName,
           unsigned char * * matrix, int nRows, int nCols )
{
    JSAMPROW row_pointer[1];
    unsigned char * image_buffer; 
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;
    int i,j;
    int quality;

    jpeg_create_compress( &cinfo );

    image_buffer = (unsigned char *)malloc( sizeof(unsigned char) * nRows );

    if ((outfile = fopen(outfileName, "wb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", outfileName);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = nCols;
    cinfo.image_height = nRows;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    jpeg_set_defaults(&cinfo);
    quality = 2;
    jpeg_set_quality(&cinfo, quality, TRUE );
    jpeg_start_compress(&cinfo, TRUE);

    for( i=0;i<cinfo.image_height;i++ )
    {
        for( j=0;j<cinfo.image_width;j++ )
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


