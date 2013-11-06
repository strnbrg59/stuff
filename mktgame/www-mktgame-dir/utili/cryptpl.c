// Encrypt game passwords, for easy and safe transfers to readers.
// Algorithm: password is a number on [0,93]: this becomes cryptkey_0, our
// initial key.  Subsequent keys are generated from rand() called cryptkey%7
// times.  Letting c_i be the i'th character in the password,
// the encrypted character is mod(c_i+cryptkey_i-33, 94)+33.  I.e. we do
// a transposition transform, using "random" transpositions.
//
// Usage: cryptpl [-e|-d] filename cryptkey
// where -e means "encrypt" and -d means "decrypt".

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINELEN 240

void crypt( char *name, char e_or_d, int *cryptkey );

main( int argc, char *argv[] ) {
  char wholeline[LINELEN+1], linecopy[LINELEN+1],
      *section, *realname, *knickname, *password, *grade,
	      e_or_d; // -e for encrypt, -d for decrypt
  int cryptkey;
  FILE *infile, *outfile;

  //--------------------- load parameters from command line
  if( argc!=4 ) {
      printf(" (Usage: \"cryptplayers -e|-d infile cryptkey\")\n");
      exit(0);
  }

  e_or_d = argv[1][1];
  if( e_or_d!='e' && e_or_d!='d' ) {
      printf(" (Usage: \"cryptplayers -e|-d infile cryptkey\")\n");
      exit(0);
  }

  infile = fopen( argv[2], "r" );
  if(!infile) {
     printf("Failed to open infile.  (Usage: \"cryptplayers -e|-d infile cryptkey\")\n");
     printf("where cryptkey should be an integer on [0,93]\n");
     exit(0);
  }
  outfile = stdout;

  cryptkey = atoi(argv[3]);
  if( cryptkey<0 || cryptkey>93 ) {
      printf("cryptkey has to be on [0,93]\n"); exit(0);
  }
  if( cryptkey==66 ) cryptkey=65;  // inversion doesn't work for 66.
# ifdef __TURBOC__
      srand(cryptkey);
# else
      srandom(cryptkey);
# endif
  //--------------------- done loading parameters

  // Read infile line by line, send knicknames and password for encryption.
  section = (char *)&argc;  // So it's not NULL; we want to dereference it.
  while( section[0]!='%' ) { // while not end of file
      fgets( wholeline, LINELEN, infile );
      strncpy( linecopy, wholeline, LINELEN );
      section = strtok( linecopy, " \t\n" );
      fprintf( outfile, "%-3s", section );
      realname = strtok( NULL, " \t\n" );
      if( realname ) fprintf( outfile, "%-26s", realname ); else break;
      knickname = strtok( NULL, " \t\n" );
      if( knickname ) {
          crypt( knickname, e_or_d, &cryptkey );
          fprintf( outfile, "%-10s", knickname ); 
      }
      else break;

      password = strtok( NULL, " \t\n" );
      if( password ) {
	  crypt( password, e_or_d, &cryptkey );
	  fprintf( outfile, "%-10s", password );
      } // if(password)
      else break;

      // Read in everything else on the line(these are grades), & fputs them.
      grade = strtok( NULL, " \t\n" );
      while(grade) {
	  fprintf( outfile, "%-5s", grade );
	  grade = strtok( NULL, " \t\n" );
      }
      fprintf( outfile, "\n" );
  }
return 0;
} // main()
//------------------------------

void crypt( char *name, char e_or_d, int *cryptkey ) {
  // name: will be encrypted and overwritten.
  // e_or_d: e for "encrypt", d for "decrypt".
  // cryptkey: number on [0,93].
  int i=0,j;
  while( name[i] != 0 ) {
      if( e_or_d=='e' )  // encrypt
	  name[i] = (name[i]+ *cryptkey)%94 + 33;
      else // decrypt
	  if( name[i]- *cryptkey >= 66 )
	      name[i] = name[i]- *cryptkey-33;
	  else
	      if( name[i]- *cryptkey >= -28 )
		  name[i] = name[i]- *cryptkey-33+94;
	      else
		  name[i] = name[i]- *cryptkey-33+188;

#     ifdef __TURBOC__
          for( j=0;j< *cryptkey%7;j++ ) rand();
	   *cryptkey = rand()%94;
#     else
          for( j=0;j< *cryptkey%7;j++ ) random();
	   *cryptkey = random()%94;
#     endif
      if(  *cryptkey == 66 )  *cryptkey=65;
      i++;
  }
} // crypt()


