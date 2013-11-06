// Nice neat columns for players.dat
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main() {
  char wholeline[241], *tok;
  FILE *infile, *outfile;

  infile = fopen( "players.dat", "r" );
  if(!infile) exit(0);
  outfile = stdout;

  while(!feof(infile)) {
      fgets( wholeline, 240, infile );

      tok = strtok( wholeline, " \t\n" );
      fprintf(outfile, "%-3s", tok );
      tok = strtok( NULL, " \t\n" );
      fprintf(outfile, "%-26s", tok );
      tok = strtok( NULL, " \t\n" );
      fprintf(outfile, "%-10s", tok );
      tok = strtok( NULL, " \t\n" );
      fprintf(outfile, "%-10s", tok );
      fprintf(outfile, "\n" );
  }

  fclose(infile);
return 0;
}
