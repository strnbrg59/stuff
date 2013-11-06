/* File AQC.H */
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

/**** DEFINITIONS ****/
#ifndef __TURBOC__
#define __32BITBYTES__
#endif

#ifdef __WATCOMC__
#define __TURBOC__
#endif

#define MAX(a,b) ( a < b ? b : a )
#define MIN(a,b) ( a > b ? b : a )

#define MKT 0
#define LIM 1
#define BUY 0
#define SELL 1
#define YES 1
#define NO 0
#define SHUTOFF 1

/* file names */
#define PLAYERFILE    "players.dat"
#define ORDBOOKNAME   "ordbook.dat"  /* dumped order book--for safety */
#define DIARYFILE     "diary.out"
#define SECURITYFILE  "security.par"  /* data on the securities to be traded */
#define ANNOUNCEMENT  "announce.doc"

/* string lengths */
#ifdef __TURBOC__ 
#    define NAMELEN 8	/* length of player name in ORDER & TRADE */
#else                   /* shorter in DOS because of 8-char limit on file names */
#    define NAMELEN 9
#endif
#define ACTNAMELEN 250  /* length of full path to .act files--plenty */

/* misc */
#define MAXDEPTH 10  /* for displaying the order books in summary form */
#define SCRNLINES 6  /* number of lines on a monitor screen */
#define TOOHIGH 120 /* reject prices greater than this as ridiculous */
#define TOOMANY 50 /* reject mkt orders for more units than this */

/**** TYPEDEFS ****/
typedef struct ORDER {
    char name[NAMELEN+1];   /* player's name (8 chars + 0x0) */
    int mkt_or_lim;    		/* MKT|LIM */
    int buy_or_sell;   		/* BUY|SELL */
    char comment;           /* 'C'=cancel, 'F'=forced, 'E'=opt.exercise */
    double amount;     		/* unsigned */
    double price;
    time_t zman;            /* as returned by time_t() function */
    struct ORDER *next; 	/* link (needed to maintain order book) */
    struct ORDER *prev;     /* back-link */
} ORDER;

typedef struct ORDERBOOK {
    double price;
    double amount;
} ORDERBOOK;

typedef struct TRADE {
    char name[NAMELEN+1];		/* player's name (8 chars + 0x0) */
    char other_sec[NAMELEN+1];  /* security (or currency) traded for */
    int  as_money;       /* YES if trade was initiated in other_sec's market */
    double amount;	       /* signed */
    double price;	       /* average price for transaction */
    time_t zman;               /* as returned by time_t() function */
} TRADE;

typedef struct HOLDINGS {
    char name[NAMELEN+1];
    double amount;
    time_t zman;
} HOLDINGS;

typedef struct NAMES {
    char name[NAMELEN+1];   /* name for market game */
    char pwd[NAMELEN+1];    /* password */
    char realname[10*NAMELEN+1]; /* player's real name */
    int  section;           /* section of the course */
} NAMES;

typedef struct SECURITY {
    char   name[NAMELEN+1];
    char   forcemkt[NAMELEN+1];  /* keep length of forcemkt and currency */
    char   currency[NAMELEN+1];  /* fields the same--see load_sec().     */
    char   menu_fullname[4*NAMELEN]; /* name in menu display */
    char   menu_choice[2];          /* one-letter code for selection from menu */
    char   underlying[NAMELEN+1]; /* underlying asset, if this is an option */
    char   call_or_put[2];        /* "c" or "p" */
    double strikeprice;
    int    maxorders;            /* maximum orders a person may place */
} SECURITY;

typedef struct PAIR {    /* complex number--needed for permute() */
    int index;  /* 0,1,2,... */
    int mikri;  /* random number */
} PAIR;

typedef struct ACTHEADER { /* header in .act file */
    int numorders;
} ACTHEADER;

typedef struct VECT { /* for mean() and variance()  (trimmed moments) */
    int n;
    double *data;
} VECT; 

/**** FUNCTION DECLARATIONS ****/
/* in orders.c */
int 	  count_orders( ORDER *best );
void 	  dump_ordbook( ORDER *best_limbid, ORDER *best_limask,
                        char *security );
void      dump_ord( ORDER *latestord, HOLDINGS *holding, char *security );
ORDER    *find_just_better( ORDER *bestord, ORDER *latestord );
void      free_ordbook( ORDER *best_limbid, ORDER *best_limask );
void      handle_order( ORDER *latestord, char *security, int call_server );
void 	  load_ordbook( ORDER **best_limbid, ORDER **best_limask,
                        char *security );
int       nameok( NAMES *user, NAMES *names, int n );

/* in dispordr.c */
void      disp_orders( char *security );
void      disp_order_summary( char *security );

/* in bldordr.c */
ORDER    *build_order( char *name, char *security );
ORDER *create_order( char name[NAMELEN+1], int mkt_or_lim, int buy_or_sell,
                     double amount, double price );
void disp_order( ORDER *order, FILE *outfile, char *security );


/* in trades.c */
TRADE   *cross( ORDER *latestord, ORDER **best_lim, char *security );
void    disp_trade( FILE *outfile, TRADE *trade, char *security );
void    dump_trade( TRADE *trade, HOLDINGS *holding, char *security );
void 	notify( TRADE *newtrade, char *security );
void    tradecpy( TRADE *dest, TRADE *source );

/* in util.c */
void       announcement(void);
void       actfile_name( char *name, char *security, char *actfilename );
char      *capname( char *name );
char      *currency_id( SECURITY *sec );
/*void       do_nothing( void );*/
void       error( char *message );
SECURITY *find_sec( char *sec_name, SECURITY **securities );
void       client_warning( char *message );
char       get_disp_cmd( void );
int        ismoney( SECURITY *sec );
double mean( VECT *vect, double trimfactor );
int        paircmp( PAIR *, PAIR *);
ORDER     *order_alloc( void );
void       ordbook_name( char *security, char *ordbookname );
#ifndef __TURBOC__
#ifndef GO32
void       strlwr( char *str );
#endif
#endif
TRADE     *trade_alloc( void );
HOLDINGS *holding_alloc( void );
void       order_free( ORDER *order );
void       trade_free( TRADE *trade );
int       *permute( int sup );
void       holding_free( HOLDINGS *holding );
void       prospectus( char *security );
double     round( double x, double y );
char      *shasctime( struct tm *zman );
int        string_length( char *str );
void       stringlwr( char *str );
time_t     unix_seconds(int yy,int mm,int dd, int hh,int min,int ss);
double variance( VECT *vect, double trimfactor );

/* in holdings.c */
void       finger( char *ownname );
void       dump_players( NAMES *names, int n );
HOLDINGS *find_holding( char *name, HOLDINGS *holdings );
void       free_names( NAMES *names );
void       free_holdings( HOLDINGS *holdings );
NAMES     *get_usrnamepwd( void );
void       holding_list( char *name );
int        load_players( NAMES **names );
HOLDINGS *load_holdings( NAMES *names, char *security );
HOLDINGS *load_holding( char *name, char *security );
void       update_money( TRADE *trade, HOLDINGS *holding );
void       update_holding( TRADE *trade, HOLDINGS *holding );
double value_portf( char *name );

/* in cancel.c */
int        cancel_order( char *name, char *security );
void       remove_all_orders( char *security );
void       splice_out( ORDER **best_lim, ORDER *delorder );

/* in  disptrad.c */
void      get_act_ts( char *name, char *security );
void      get_money_ts( char *name, char *currency );
void      read_three( ORDER *currorder, TRADE *currtrade,
              HOLDINGS *currholding, long int *filepos, FILE *infile );
void      backup_three( FILE *actfile, long int *filepos );

/* in menu.c */
void      menu1( char *name );

/* in password.c */
int       install_pwd( NAMES *elem, NAMES *names );
void      prompt_for_pwd( char *pwd );

/* communic/client.c */
int       server_avail( void );
void      server_ready( int handle );
void      server_goodbye( int handle );

/* in diary.c */
void      update_diary( TRADE *trade, HOLDINGS *holding, char *security );
void      disp_diary( char *security );

/* in forcesal.c */
int       check_neg_money( char *name, char *currency );
double    latest_money( char *name, char *currency );
void      forced_sale( void );

/* in options.c */
void      exercise_option( char *name,  char *option, char *underlying,
                           double x, char call_or_put );

/* in loadsec.c */
SECURITY **load_securities( void );
int is_sec_choice( char menu_choice );
int n_currencies( void );

/* in cncttime.c */
void dump_cncttime( void );
void disp_connect_time( void );
void init_cncttime_Kname( char *kname );
void update_connect_time( int dump_now );
