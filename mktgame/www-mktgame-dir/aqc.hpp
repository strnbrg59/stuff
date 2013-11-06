/* File AQC.H */
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <String.h>
#include "fcommunic.hpp"

// Special definitions for compilation on rahul's machines
#ifdef Rahul // if we're using the file-based "sockets" functions.
			 // More rahul definitions below.
#	define SEEK_SET 0
#	define SEEK_CUR 1
#	define SEEK_END 2
#endif

#ifdef Rahul
#	define CGIBINDIR "/cgi-bin/strnbrg/"
#	define LOCALHOST "www.rahul.net"
#else
#	define CGIBINDIR "/teds-cgi-bin/"
#	define LOCALHOST "localhost"
#endif

/**** DEFINITIONS ****/
//#define HTML "Content-type: text/html\nPragma: no-cache\n\n"
#define HTML "Content-type: text/html\n\n"

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
#ifdef Rahul
#	define WWWMKTGAMEDIR "/wg/home/strnbrg/www-mktgame-dir/"
#	define PLAYERFILE    "/wg/home/strnbrg/www-mktgame-dir/players.dat"
#	define ORDBOOKNAME   "ordbook.dat"  
#	define DIARYFILE     "diary.out"
#	define SecurityFILE  "/wg/home/strnbrg/www-mktgame-dir/security.par"
#	define ANNOUNCEMENT  "/wg/home/strnbrg/www-mktgame-dir/announce.doc"
#else
#	define WWWMKTGAMEDIR "/users/ted/src/mktgame/www-mktgame-dir/"
#	define PLAYERFILE    "/users/ted/src/mktgame/www-mktgame-dir/players.dat"
#	define ORDBOOKNAME   "ordbook.dat"  
#	define DIARYFILE     "diary.out"
#	define SecurityFILE  "/users/ted/src/mktgame/www-mktgame-dir/security.par"
#endif


/* string lengths */
#define NAMELEN 9
#define ACTNAMELEN 250  /* length of full path to .act files--plenty */

/* misc */
#define MAXDEPTH 10  /* for displaying the order books in summary form */
#define SCRNLINES 6  /* number of lines on a monitor screen */
#define TOOHIGH 120 /* reject prices greater than this as ridiculous */
#define TOOMANY 50  /* reject orders for more units than this */

/**** TYPEDEFS ****/
struct Order {
    char name[NAMELEN+1];   /* player's name (8 chars + 0x0) */
    int mkt_or_lim;    		/* MKT|LIM */
    int buy_or_sell;   		/* BUY|SELL */
    char comment;           /* 'C'=cancel, 'F'=forced, 'E'=opt.exercise */
    double amount;     		/* unsigned */
    double price;
    time_t zman;            /* as returned by time_t() function */
    struct Order *next; 	/* link (needed to maintain order book) */
    struct Order *prev;     /* back-link */
};

struct Orderbook {
    double price;
    double amount;
};

struct Trade {
    char name[NAMELEN+1];		/* player's name (8 chars + 0x0) */
    char other_sec[NAMELEN+1];  /* security (or currency) traded for */
    int  as_money;       /* YES if trade was initiated in other_sec's market */
    double amount;	       /* signed */
    double price;	       /* average price for transaction */
    time_t zman;               /* as returned by time_t() function */
};

struct Holdings {
    char name[NAMELEN+1];
    double amount;
    time_t zman;
};

struct Names {
    char name[NAMELEN+1];   /* name for market game */
    char pwd[NAMELEN+1];    /* password */
    char realname[10*NAMELEN+1]; /* player's real name */
    int  section;           /* section of the course */
};

struct Security {
    char   name[NAMELEN+1];
    char   forcemkt[NAMELEN+1];  /* keep length of forcemkt and currency */
    char   currency[NAMELEN+1];  /* fields the same--see load_sec().     */
    char   menu_fullname[4*NAMELEN]; /* name in menu display */
    char   menu_choice[2];          /* one-letter code for selection from menu */
    char   underlying[NAMELEN+1]; /* underlying asset, if this is an option */
    char   call_or_put[2];        /* "c" or "p" */
    double strikeprice;
    int    maxorders;            /* maximum orders a person may place */
};

struct Pair {    /* complex number--needed for permute() */
    int index;  /* 0,1,2,... */
    int mikri;  /* random number */
};

struct Actheader { /* header in .act file */
    int numorders;
};

struct Vect { /* for mean() and variance()  (trimmed moments) */
    int n;
    double *data;
};

struct Chosen_ones {
    char name[NAMELEN+1];   /* player's name */
    int  units;             /* units (short) that are going to be exercised against him */
};

/**** FUNCTION DECLARATIONS ****/
/* in orders.c */
int 	  count_orders( Order *best );
void 	  dump_ordbook( Order *best_limbid, Order *best_limask,
                        char *security );
void      dump_ord( Order *latestord, Holdings *holding, char *security );
Order    *find_just_better( Order *bestord, Order *latestord );
void      free_ordbook( Order *best_limbid, Order *best_limask );
void      handle_order( Order *latestord, char *security, int call_server );
void 	  load_ordbook( Order **best_limbid, Order **best_limask,
                        char *security );
int       nameok( Names *user, Names *names, int n );
int 	  namescomp( const void *names1, const void *names2 );

/* in dispordr.c */
void      disp_orders( char *security );
void      disp_order_summary( char *security );

/* in bldordr.c */
int at_orderlimit( char *name, char *security );
void	build_order( char *name, String pwd, char *security );
Order *create_order( char name[NAMELEN+1], int mkt_or_lim, int buy_or_sell,
                     double amount, double price );
void disp_order( Order *order, FILE *outfile, char *security );
int front_of_firm( Order *order, char *security );
int verify_order( Order *order, char *security );

/* in trades.c */
Trade   *cross( Order *latestord, Order **best_lim, char *security );
void    disp_trade( FILE *outfile, Trade *trade, char *security );
void    dump_trade( Trade *trade, Holdings *holding, char *security );
void 	notify( Trade *newtrade, char *security );
void    tradecpy( Trade *dest, Trade *source );

/* in util.c */
void       announcement(void);
void       actfile_name( char *name, char *security, char *actfilename );
char      *capname( char *name );
String crypt_pwd( const char* plaintxt, char *master_pwd, int serial );
char      *currency_id( Security *sec );
void       do_nothing( void );
void       error( char *message );
Security *find_sec( char *sec_name, Security **securities );
void       client_warning( char *message );
char       get_disp_cmd( void );
int        ismoney( Security *sec );
double mean( Vect *vect, double trimfactor );
int        paircmp( const void *, const void *);
void print_file( String filename );
Order     *order_alloc( void );
void       ordbook_name( char *security, char *ordbookname );
#ifndef __TURBOC__
void       strlwr( char *str );
#endif
Trade     *trade_alloc( void );
Holdings *holding_alloc( void );
void       order_free( Order *order );
void       trade_free( Trade *trade );
int       *permute( int sup );
void       holding_free( Holdings *holding );
void       prospectus( char *security );
double     round( double x, double y );
char      *shasctime( struct tm *zman );
int        string_length( char *str );
void       stringlwr( char *str );
time_t     unix_seconds(int yy,int mm,int dd, int hh,int min,int ss);
double variance( Vect *vect, double trimfactor );

/* in holdings.c */
void       finger();
void       dump_players( Names *names, int n );
Holdings *find_holding( char *name, Holdings *holdings );
void       free_names( Names *names );
void       free_holdings( Holdings *holdings );
Names     *get_usrnamepwd( void );
void       holding_list( char *name );
int        load_players( Names **names );
Holdings *load_holdings( Names *names, char *security );
Holdings *load_holding( char *name, char *security );
void       update_money( Trade *trade, Holdings *holding );
void       update_holding( Trade *trade, Holdings *holding );
double value_portf( char *name );

/* in cancel.c */
int        cancel_order( char *name, String pwd, char *security );
void 	   remove_order( char *name, char *security,
    Order **best_limbuy, Order **best_limsell, Holdings *holding );
void 	   mark_for_deletion( char *name,
    Order **best_limbuy, Order **best_limsell, int delnum );
void       remove_all_orders( char *security );
void       splice_out( Order **best_lim, Order *delorder );

/* in  disptrad.c */
void      get_act_ts( char *name, char *security );
void      get_money_ts( char *name, char *currency );
void      read_three( Order *currorder, Trade *currtrade,
              Holdings *currholding, long int *filepos, FILE *infile );
void      backup_three( FILE *actfile, long int *filepos );

/* in menu.c */
void disp_choices1( char* username, char* pwd );
void disp_choices2( int sec_num, String username, String pwd );
void      menu1( char *name );
int money_only( Security *sec );

/* in password.c */
int       install_pwd( Names *elem, Names *names );
void      prompt_for_pwd( char *pwd );
void	  verify_user( String username, String pwd );

/* communic/client.c */
int       server_avail( String username );
void      server_ready( int handle );
void      server_goodbye( int handle );

/* in diary.c */
void      update_diary( Trade *trade, Holdings *holding, char *security );
void      disp_diary( char *security );

/* in forcesal.c */
int       check_neg_money( char *name, char *currency );
double    latest_money( char *name, char *currency );
void      forced_sale( void );

/* in options.c */
void      exercise_option( 	char *name, String pwd,
							char *option, char *underlying,
							double x, char call_or_put );
Chosen_ones *choose_shorts( char *option, int units );

/* in loadsec.c */
Security **load_securities( void );
int is_sec_choice( String menu_choice );
int n_currencies( void );

/* in cncttime.c */
void dump_cncttime( void );
void disp_connect_time( void );
void init_cncttime_Kname( char *kname );
void update_connect_time( int dump_now );

//------------
struct G { // for holding all the global variables
	static Names *names;
	static int N_players;
	static int N_sec;
	static Security **securitydata;
};
