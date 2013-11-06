/* Keep track of connect time to the aqc program.  Use file locking to make this
   work even if a player is logged on several times at once.
 Yet to implement:
   1. peak-load pricing
   2. We're counting time too slowly, sometimes.  And sometimes we're counting it just right.  So
      it's going to be hard to find.
   3. With two processes going at the same time, we do seem to count time about twice as fast.  Though
      it's hard to say exactly, since disk-writes are only every ten invocations of update_connect_time().
*/

/* INCLUDED FILES */
#include <sys/file.h>
#include <sys/types.h>  /* for stat(), from which we get file length */
#include <sys/stat.h>   /* for stat() */
#include <sys/dir.h> /* for DIR and opendir() */
#include <fcntl.h>   /* for open() and close() */

#include <unistd.h>  /* for flock() */
#define LOCK_SH 1    /* unistd.h should define this, but in some systems it doesn't */
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "aqc.h"

/* DEFINITIONS  */
#define CNCT_PARFILE "cncttime.par"
#define TIMEWARNINTVAL 300   /* in seconds */

/* GLOBAL VARIABLES */
char   Kname[NAMELEN+1]; /* so we can call functions from SIGALRM, without knowing player's name. */
time_t Disk_time,    /* Total connect time when we last read from disk. */
       Since_diskread, /* time since last disk read */
       Max_allowed;  /* total allowed (available in cncttime.par) */


/* LOCAL FUNCTION DECLARATIONS */
/*void dump_cncttime( void ); */
static time_t get_max_allowed( char *parfilename );
static time_t get_disk_time( char *knickname );
static void time_warning( void );  

/* FUNCTIONS VISIBLE EXTERNALLY 
void disp_connect_time( void );
void init_cncttime_Kname( char *kname );
void update_connect_time( int dump_now );

*/

/* FUNCTION DEFINITIONS */
void disp_connect_time( void ) {  
  time_t total_time;

  update_connect_time(NO);
  total_time = Disk_time + Since_diskread;

  printf( "Connect time used = %7.2f minutes.  Connect time remaining = %7.2f minutes.\n",
           (float)total_time/60, (float)(Max_allowed-total_time)/60 );
} /* end of disp_connect_time() */
/*----------------------------------------------------------*/

void init_cncttime_Kname( char *kname ) {
  /* initializes Kname, so other functions in this file don't
     need to have it passed to them.
   */
  strcpy( Kname, kname );
} /* end of init_cncttime_Kname() */

/*----------------------------------------------------------*/

void update_connect_time( int dump_now ) {
  /* Keeps track of connect time in static variables. */
  /* Assumes Kname has been initialized. */

  static int times_called;  /* number of times this function has been called. */
  static time_t last_time;
  static int disk_read = YES;
  time_t now;
  DIR *dp;

  /* If this is the first call to this function, initialize.  Initialize also if 
   *   the cncttime data has just been dumped to disk (that sets disk_read to YES).
   *   We have to do the latter in case the user is logged on more than once.
  */
  if( disk_read == YES ) {  /* static variables are initialized only once. */
      disk_read = NO;
      Max_allowed = get_max_allowed( CNCT_PARFILE ); 
      Disk_time = get_disk_time( Kname );
      Since_diskread = 0;
      time( &last_time );
      if( (dp=opendir("cncttime")) == NULL ) 
          error( "Directory cncttime does not exist.");
      if( Kname[0]==0 ) error( "cncttime.c: Kname not initialized." );
  }

  /* update */
  time( &now );
  Since_diskread += now - last_time;  
  last_time = now;                    

  /* Issue a warning if time is running out.  Shut off if time *has* run out. */
  if( Max_allowed - (Disk_time + Since_diskread) < 30*60 )
     time_warning();  /* time_warning makes sure we don't issue warnings too often. */
  if( Max_allowed - (Disk_time + Since_diskread) < 0 ) {
      dump_cncttime();
      error( "You have exceeded your time budget." );
  }

  /* Write to file periodically, or if dump_now==YES.  */
  if( ( (++times_called)%10 == 0) || (dump_now==YES) ) {
      dump_cncttime();
      disk_read = YES;
  }

} /* end of update_connect_time() */
/*----------------------------------------------------------*/

static time_t get_max_allowed( char *parfilename ) {
  FILE *infile;
  long int result;

  infile = fopen( parfilename, "r" );
  if( infile == NULL ) error( "No CNCT_PARFILE." );

  fscanf( infile, "%ld", &result );
  fclose( infile );
  if( result < 0 ) error( "Maximum time allowed < 0 !?" );
  return (time_t)result;
} /* end of get_max_allowed()  */
/*----------------------------------------------------------*/

static time_t get_disk_time( char *knickname ) {
  /* Look at player's connect time .act file, and see how much time he's logged. */

  char infilename[ACTNAMELEN+1];
  long int result;
  FILE *infile, 
       *errfile;

  strcpy( infilename, "cncttime/" );
  strcat( infilename, knickname );
  strcat( infilename, ".act" ); 

  infile = fopen( infilename, "r" );
  if( infile==NULL ) {
         result = 0;  /* Hopefully, happens only when player is logging in for first time. */
                      /* Otherwise, we return 0 and reset the time; that may be our bug. */
         errfile = fopen( "cncttime.error2", "a" );
         fprintf( errfile, "infile==NULL in get_disk_time(%s)\n", knickname );
         fclose(errfile);
  }
  else {
      fscanf( infile, "%ld", &result );
      fclose( infile );
  }

  return (time_t)result;
} /* end of get_disk_time() */
/*----------------------------------------------------------*/

static void time_warning(void) {
  /* Make sure we don't issue warnings too often. */
  static time_t last_warning_time; 
  time_t now, total_time;

  time( &now );
  if( now - last_warning_time > TIMEWARNINTVAL ) {
      total_time = Disk_time + Since_diskread;
      printf( "Warning: you have only %5.2f minutes connect time left.\n",
              (float)(Max_allowed - total_time)/60 );
      last_warning_time = now;
  }
} /* end of time_warning() */
/*--------------------------------------------------*/      

void dump_cncttime( void ) {
  /* Use file locking, in case this player has another aqc process
   *   going at the same time.  Read Disk_time from disk, in case
   *   it's been updated by the player's other process.
  */
  char infilename[ACTNAMELEN+1],
       charint[30];
  int fd, errorcode = -1;
  time_t total_time;
  struct stat sbuf;
  FILE *errorfile;

  /* generate the infile's name */
  strcpy( infilename, "cncttime/" );
  strcat( infilename, Kname );
  strcat( infilename, ".act" ); 

  fd = open( infilename, O_RDONLY ); /* only so we have something to close below... */
  while( errorcode < 0 ) {
      close(fd); /* matches open() just below... */
      fd = open( infilename, O_RDWR|O_CREAT, 0600 );
      errorcode = flock( fd, LOCK_EX | LOCK_NB );
  }

  /* need to find if infile has meaningful info in it.  If it was just created, it doesn't. */
  /* The only way I know to check if it was just created is to see if its length is zero.*/
  stat( infilename, &sbuf );
  if( sbuf.st_size == 0 )
      Disk_time = 0;
  else {
      Disk_time = get_disk_time( Kname );  /* notice we do this while file is locked! */
      if( Disk_time == 0 ) {
          errorfile = fopen( "cncttime.error", "a" );
          fprintf( errorfile, "Disk_time set to zero by %s\n", Kname );
          fclose( errorfile );
      }
  }
  total_time = Disk_time + Since_diskread; 
  sprintf( charint, "%ld", (long int)total_time );
  write( fd, charint, strlen(charint)+1 );

  errorcode = flock( fd, LOCK_UN );
  close( fd );
  if( errorcode < 0 ) error( "Error in dump_cncttime." );

} /* end of dump_cncttime() */
/*----------------------------------------------------------*/


