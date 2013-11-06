/* all those char * typecasts don't seem to hurt anything.  If you change
them to void *, what happens is you raise the "overwrite" warning in
del_ptr() because its FILLCHAR is some char, and it won't match what your
void pointer points to.  So just leave this alone. */
/*---------------------------------------------------------------
   xmem.c -- Extended Dynamic Memory Control Module.
---------------------------------------------------------------*/
/* ********************** INCLUDE FILES ********************** */
#include <stdio.h>
#include <stdlib.h>   /* includes malloc() and calloc() */
#ifdef __TURBOC__
#   include <mem.h>
#else
#  include <string.h>  /* for memset() */
#endif

/* ********************* GLOBAL FUNCTIONS ******************** */
char *x_malloc(unsigned int);
char *x_calloc(unsigned int, unsigned int);
void  x_free(char *);
void  x_chkfree(void);

static char *sto_ptr(char *p, unsigned b);
static void del_ptr(char *p);

/* ********************* GLOBAL VARIABLES ******************** */
/* memtrace usage:
      = 0 => simple calls to malloc & free
      = 1 => tracking of all allocations using hash table
      = 2 => checking for changes to previously freed blocks.
      Warning: setting to 2 makes x_mem make copies of everything, so
      your dynamic memory usage will be excessive.  Also, calls to
      free() will seem not to work.
*/
int  memtrace   = 0;     /* memory tracing control variable */
long Tot_memory = 0L;    /* total amount of allocated memory */
long Tot_alloc  = 0L;    /* total # of allocations */
long Tot_free   = 0L;    /* total # of calls to x_free */
int  hashsize   = 47;    /* size of hash table */
int  bucketsize = 10;    /* number of entries per hash bucket */

/* ********************* LOCAL VARIABLES ********************* */
/* memory allocation tracking table */

/* amount of extra allocation for overhead */
#define OVHDSIZE 2

/* fill character for overhead gap */
#define FILLCHAR '\377'

/* allocated entry information */
typedef struct alloc_entry {
   int   size;           /* size of allocated area */
   char *ptr;            /* pointer to allocated area */
   char *freed;          /* pointer to copy of allocated area */
} ALLOCATION;

typedef struct bucket {
   struct bucket *next;  /* pointer to next bucket when filled */
   int entries;          /* number of used entries */
   ALLOCATION *alloc;    /* allocated entry array */
} BUCKET;

#define NUL_BUCKET ((BUCKET *)0)

/* dynamic pointer hash table */
static BUCKET **ptrhash = (BUCKET **)0;

/* ==============================================================
   Store pointer in hash table
*/
static char *sto_ptr(char *p, unsigned b)
/*char *p;      pointer to be stored */
/*unsigned b;   size of area */
{
   register BUCKET *bp, *bq;    /* bucket pointers */
   register int bno;            /* bucket/entry number */

   if ( ! ptrhash ) {
      /* allocate pointer hash table */
      ptrhash = (BUCKET **)calloc(hashsize, sizeof(BUCKET *));
      if ( ! ptrhash )
         return(NULL);
      Tot_memory = hashsize * sizeof(BUCKET *);
   }
   /* compute hash table index */
   bno = (int)((unsigned long)p % hashsize);

   /* find first bucket with available entries */
   for (bq = bp = ptrhash[bno]; bp && bp->entries == bucketsize;
      bp = bp->next)
      bq = bp;

   /* allocate new bucket if necessary */
   if ( bp == NUL_BUCKET ) {
      bp = (BUCKET *)malloc(sizeof(BUCKET));
      if ( !bp )
         return(NULL);
      bp->next = NUL_BUCKET;
      bp->entries = 0;
      if ( bq )
         /* connect to end of bucket chain */
         bq->next = bp;
      else
         /* initial bucket for this hash entry */
         ptrhash[bno] = bp;

      /* allocate bucket's allocation entry array */
      bp->alloc = (ALLOCATION *)calloc(bucketsize,
                     sizeof(ALLOCATION));

      /* memory total includes space used by hash table */
      Tot_memory += sizeof(BUCKET) +
                       bucketsize*sizeof(ALLOCATION);
   }
   /* store pointer to allocated block */
   bno = bp->entries++;
   bp->alloc[bno].ptr   = p;
   bp->alloc[bno].freed = NULL;
   bp->alloc[bno].size  = b;
   /* update total allocation */
   Tot_memory += b;

   /* increment total number of allocations */
   ++Tot_alloc;

   return(p);
}

/* ==============================================================
   Delete pointer from hash table
*/
static void del_ptr(char *p)
/*char *p;   pointer to be freed */
{
   int gap;                     /* index into overhead space */
   register BUCKET *bp, *bq;    /* bucket pointers */
   register int bno, i;         /* bucket/entry number */

   /* compute hash table index */
   bno = (int)((unsigned long)p % hashsize);

   /* search bucket(s) for pointer */
   for (bq = NUL_BUCKET, bp = ptrhash[bno]; bp; bp = bp->next) {
      for ( i = 0; i < bp->entries; ++i ) {
         if ( bp->alloc[i].ptr == p ) {
            /* check integrity of gap */
            for (gap=bp->alloc[i].size-OVHDSIZE;
               gap<bp->alloc[i].size; ++gap ) {
               if ( p[gap] != FILLCHAR ) {
                     printf("WARNING overwrite, addr: %lx\n",
                     (long)p);
      /* unfortunately, we never know if a pointer has been overwritten
         unless for some reason we wish to free it. */
                  break;
               }
            }
            if ( memtrace == 1 ) {
               /* remove entry from bucket */
               if ( --bp->entries == 0 ) {
                  /* free this bucket */
                  if ( bq )
                     bq->next = bp->next;
                  else
                     ptrhash[bno] = bp->next;
                  free((char *)bp->alloc);
                  free((char *)bp);
                  Tot_memory -= (sizeof(BUCKET) +
                                  bucketsize*sizeof(ALLOCATION));
               }
               else if ( i < bp->entries ) {
                  /* move last entry into current spot */
                  bp->alloc[i] = bp->alloc[bp->entries];
               }
               free(p);
            }
            else {
               /* memtrace == 2
                  => save copy to check for bad mods */
	       if ( bp->alloc[i].freed ) {
                  printf("WARNING freeing free ptr, addr: %lx\n",
		     (long)p);
                     memtrace=memtrace;
	       }
               else {
                   bp->alloc[i].freed = (char *)malloc(bp->alloc[i].size);
                   if (bp->alloc[i].freed)
                       memcpy(bp->alloc[i].freed, bp->alloc[i].ptr,
                           bp->alloc[i].size);
                   }

            }
            /* update total allocated memory count */
            Tot_memory -= bp->alloc[i].size;

            /* normal return */
            return;
         }
      }
      bq = bp;
   }
   if ( ! bp ) {
      gap++;
      printf("WARNING freeing bad pointer, addr: %lx\n", (long)p);
      gap--;
   }
}

/* ==============================================================
   Allocate b bytes of memory
*/
char *x_malloc(unsigned int b )
/*unsigned int b;  number of bytes to allocate */
{
   register char *mptr;
   
   if ( memtrace ) {
      /* add gap space */
      b += OVHDSIZE;

      /* allocate memory */
      mptr = (char *)malloc(b);
      if ( mptr ) {
         /* fill gap */
         memset(mptr+b-OVHDSIZE, FILLCHAR, OVHDSIZE);

         /* store mptr in ptrhash */
         mptr = sto_ptr(mptr, b);
      }
   }
   else
      mptr = (char *)malloc(b);

   return(mptr);
}

/* ==============================================================
   Allocate and clear i*s bytes of memory
*/
char *x_calloc( unsigned int i, unsigned int s )
/*unsigned int i;  number of blocks to be allocated */
/*unsigned int s;  size (in bytes) of each block */
{
   register unsigned int amt;
   register char *mptr;
   
   /* allocate requested space */
   mptr = x_malloc(amt = i*s);
   if ( mptr ) {
      /* clear requested space */
      memset(mptr, '\0', amt);
   }
   return (mptr);
}

/* ==============================================================
   Free allocated memory
*/
void x_free( char *p )
/*char *p; pointer to block to be freed */
{
   Tot_free++;
   if ( p == NULL )
      printf("WARNING freed a null pointer\n");
   else if ( memtrace )
      del_ptr(p);
   else
      free((char *)p);
}

/* ==============================================================
   Check to ensure all blocks have been freed
*/
void x_chkfree(void)
{
   ALLOCATION *ap;              /* allocation entry pointer */
   register int bno, i;         /* bucket/entry number */
   register BUCKET *bp, *bq;    /* bucket pointers */

   if ( memtrace ) {
      /* check for unfreed variables */
      for ( bno = 0; bno < hashsize; ++bno ) {
         for ( bp = ptrhash[bno]; bp; bp = bq ) {
            for (i = 0; i < bp->entries; ++i) {
               ap = &bp->alloc[i];
               if ( memtrace == 2 && ap->freed ) {
                  /* check for changes to freed blocks */
                  if ( memcmp(ap->ptr, ap->freed, ap->size) )
                     printf("WARNING block chgd after free, addr: %lx\n",
                        (long)ap->ptr);
               }
               /* free unfreed block */
               printf("WARNING freeing unfreed block, addr: %lx\n",
                  (long)ap->ptr);
               free(ap->ptr);
            }
            bq = bp->next;

            /* free bucket */
            free((char *)bp->alloc);
            free((char *)bp);
         }
      }
      /* free pointer hash pointer array */
      free((char *)ptrhash);
      ptrhash = (BUCKET **)0;

      Tot_memory = 0L;
   }
} /* end of x_chkfree() */
