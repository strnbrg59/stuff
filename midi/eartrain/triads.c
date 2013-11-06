/* Plays NUMPROBLEMS triads and waits for correct repetition from user.
     At the end of NUMPROBLEMS triads, plays a low key, and then repeats
     with another NUMPROBLEMS triads.  Quits upon the press of a PC key.
 */

/**** INCLUDE FILES ****/
#include <conio.h> /* for kbhit */
#include <dos.h> /* for delay */
#include <stdlib.h> /* for random */
#include <stdio.h> /* to print mistakes */
#include <time.h> /* for randomize */

/**** DEFINES ****/
#define UART	 0x3F
#define SYSRESET 0xFF
#define DATAPORT 0x330

#define NUMPROBLEMS  10      /* num. of problems, i.e. test problems */
#define NOTE_ON  0x90
#define AGAIN    0x6c        /* to hear the triad again */

#define MINOR3RD 3
#define MAJOR3RD 4
#define FOURTH   5
#define FIFTH    7
#define SEVENTH 11
#define OCTAVE  12

#define C 0x30
#define Df 0x31
#define D 0x32
#define Ef 0x33
#define E 0x34
#define F 0x35
#define Gf 0x36
#define G 0x37
#define Af 0x38
#define A 0x39
#define Bf 0x3a
#define B 0x3b

/**** GLOBAL VARIABLES ****/
int Degree[4] = { -FOURTH, 0, FIFTH, OCTAVE } ;
   /* sets possible translations from tonic, to play triads */
int Mistakes; /* counts mistakes */

/**** TYPEDEF ****/
typedef struct TRIAD {
  char component[3];
} TRIAD;

/**** FUNCTION DECLARATIONS ****/
void clapa(int freq, int loudness);  /* plays a note */
void playtriad( TRIAD *triad, int randinversion, int randdegree);
int listen( TRIAD *triad, int randinversion, int randdegree);
TRIAD *initialize( void );
void read_param_file( int *key, int *mode );
void clearport(void);
int GETDATA(void);         /*  these were     */
int PUTDATA(int databyte); /*  supplied with  */
int PUTCMD(int cmdbyte);   /*  Conger's book. */

/**** FUNCTION DEFINITIONS ****/
main() {
  int randinversion, randdegree, i, quit=0, mistakes;
  TRIAD *triad; /* allocated in initialize(), holds 3 inversions */

  randomize();
  triad = initialize(); /* reads from triads.par */

  /* initialize the MIDI port */
  PUTCMD(UART);
  clearport();
  delay(10); /* need this, else machine hangs up when re-running main() */

  /* loop over repetitions of NUMPROBLEMS triads */
  while(1) { /* to quit, hit any computer key; listen() senses it */
    Mistakes = 0;

    for (i=1;i<=NUMPROBLEMS;i++) {
      randinversion = random(3);
      randdegree = random(4);
      playtriad( triad, randinversion, randdegree);
      quit = listen( triad, randinversion, randdegree);
      if( quit == 1 )
          return 0;
      delay(500);
    } /* for i */

    printf("\n%d Mistakes\n",Mistakes);
    clapa(0x20,0x30);delay(1000);clapa(0x20,0x00); /* end signal: deep sound */
    delay(2000);

  } /* while 1 */
  clearport();
  PUTCMD(SYSRESET);
return 0;
} /* end of main() */
/*------------------------------------------------------------------------*/

TRIAD *initialize(void){
  int key,   /* e.g. C major, Eflat minor, etc. */
      mode;  /* minor=0, major=1 */
  TRIAD *triad;

  triad = (TRIAD *)malloc(3*sizeof(TRIAD));

  read_param_file( &key, &mode );

  if( mode==1 ) /* major */ {
      triad[0].component[0] = key;
      triad[0].component[1] = triad[1].component[0] = key + MAJOR3RD;
      triad[0].component[2] = triad[1].component[1] = triad[2].component[0]
          = triad[0].component[1] + MINOR3RD;
      triad[1].component[2] = triad[2].component[1] = key + OCTAVE;
  }
  else /* mode=0 (minor) */ {
      triad[0].component[0] = key;
      triad[0].component[1] = triad[1].component[0] = key + MINOR3RD;
      triad[0].component[2] = triad[1].component[1] = triad[2].component[0]
          = triad[0].component[1] + MAJOR3RD;
      triad[1].component[2] = triad[2].component[1] = key + OCTAVE;
  }
  triad[2].component[2] = triad[0].component[1] + OCTAVE;

  return triad;
} /* end of initialize() */
/*------------------------------------------------------------------------*/

void playtriad(TRIAD *triad, int randinversion, int randdegree){

  /* display triad on screen */
  printf(" ( %X %X %X )",
      triad[randinversion].component[0] + Degree[randdegree],
      triad[randinversion].component[1] + Degree[randdegree],
      triad[randinversion].component[2] + Degree[randdegree]);

  /* play at a loudness of 0x40 */
  clapa(triad[randinversion].component[0] + Degree[randdegree],0x40);
  clapa(triad[randinversion].component[1] + Degree[randdegree],0x40);
  clapa(triad[randinversion].component[2] + Degree[randdegree],0x40);
  delay(1000);

  /* shut off the sound */
  clapa(triad[randinversion].component[0] + Degree[randdegree],0x00);
  clapa(triad[randinversion].component[1] + Degree[randdegree],0x00);
  clapa(triad[randinversion].component[2] + Degree[randdegree],0x00);

} /* end of playtriad() */
/*------------------------------------------------------------------------*/

int listen( TRIAD *triad, int randinversion, int randdegree){
  /* listen for user's input, wait until he correctly repeats
    the current triad, count Mistakes */
  int loudness, i, j;
  char note[3] = {0,0,0}, freq, temp;

  Mistakes--;

  while( (note[0]!=triad[randinversion].component[0] + Degree[randdegree])
  || (note[1]!=triad[randinversion].component[1] + Degree[randdegree])
  || (note[2]!=triad[randinversion].component[2] + Degree[randdegree]) ) {
    Mistakes++;

    while( GETDATA() != NOTE_ON) {
       if( kbhit() )
           return 1;  /* exit main loop & end program */
    };
    freq = GETDATA();
    loudness = GETDATA();
    note[0] = freq;

    while( GETDATA() != NOTE_ON) {};
    freq = GETDATA();
    loudness = GETDATA();
    note[1] = freq;

    while( GETDATA() != NOTE_ON) {};
    freq = GETDATA();
    loudness = GETDATA();
    note[2] = freq;

    /* sort note[]; in the logical test in this block, it's assumed the
      note[]'s are sorted */
    for (i=0;i<=2;i++)
      for (j=i+1;j<=2;j++) {
        if( note[i]>note[j] ) { /* then switch */
          temp = note[i];
          note[i]=note[j];
          note[j]=temp;
        }
      }

    clapa(note[0],0x40); clapa(note[1],0x40); clapa(note[2],0x40);

    while( GETDATA() != NOTE_ON) {};
    freq = GETDATA();
    loudness = GETDATA();

    while( GETDATA() != NOTE_ON) {};
    freq = GETDATA();
    loudness = GETDATA();

    while( GETDATA() != NOTE_ON) {};
    freq = GETDATA();
    loudness = GETDATA();

    delay(500);
    /* now shut off the sound.  The 8088 computer isn't fast enough to
     * enable us to shut off each sound as the signal comes from the
     * piano.
    */
    clapa(note[0],0x00); clapa(note[1],0x00); clapa(note[2],0x00);

    delay(100); sound(2000); delay(10); nosound(); /* computer sound */

    if( note[2]==AGAIN ) {
    /* hit the highest three keys to hear the triad again */
      playtriad(triad, randinversion, randdegree);
      Mistakes--;
    }

  } /* while user-played triad is wrong */

  return 0;
} /* end of listen() */
/*------------------------------------------------------------------------*/

void clearport(void) {
    while( -1 != GETDATA() ) {} /*  need this to keep dataport clear for
      outgoing bytes from putdata(). Breaks out of this loop when the
      dataport is clear, i.e. when getdata() returns -1. */
} /* end of clearport() */
/*------------------------------------------------------------------------*/

void clapa(int freq, int loudness){  /* generates a tone,
  given its frequency and loudness */
  clearport(); /* need to clear the port regularly, or it won't send
    all your data to the piano */
  PUTDATA(0X90);
  PUTDATA(freq);
  PUTDATA(loudness);
} /* end of clapa() */
/*------------------------------------------------------------------------*/

void read_param_file( int *key, int *mode ) {
  char garbage[20], keych[3], modech[6];
  FILE *paramfile;

  paramfile = fopen("triads.par", "r");
  if( paramfile == NULL ) {
      sound(1000); delay(1000); nosound();
      printf("\nFailed to open triads.par");
      exit(0);
  }

  /* sample format of triads.par:
  key: 0x30
  mode: minor
  */

  fscanf( paramfile, "%s %x", garbage, key );
  fscanf( paramfile, "%s %s", garbage, modech );

  /* translate modech to mode */
  if( modech[1] == 'i' )
      *mode = 0; /* minor */
  else
      *mode = 1; /* major */

  fclose( paramfile );

} /* end of read_param_file() */
/*------------------------------------------------------------------------*/