#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main() {
    char wholeline[100], linecopy[100], *tok;
    FILE *infile, *outfile;

    infile = fopen( "test.dat", "r" );
    if(!infile) {printf("Can\'t find test.dat.\n"); exit(0);}
    outfile = stdout; // fopen( "strip.out", "w" );

    while( !feof(infile) ) {
        fgets( wholeline, 99, infile );
        strncpy( linecopy, wholeline, 99 );
        tok = strtok( linecopy, " \t\n"  );
        if( strcmp( tok, "Subject:" ) )
            fputs( wholeline, outfile );
    }

    fclose(infile); fclose(outfile);
}
