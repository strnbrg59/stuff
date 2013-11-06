/**** TYPEDEFS for old-style structures ****/
typedef struct OLDORDER {
    char name[NAMELEN+1];   /* player's name (8 chars + 0x0) */
    int mkt_or_lim;    		/* MKT|LIM */
    int buy_or_sell;   		/* BUY|SELL */
    int cancel;                 /* YES|NO */
    int shares;        		/* unsigned */
    double price;
    time_t zman;            /* as returned by time_t() function */
    struct OLDORDER *next; 	/* link */
    struct OLDORDER *prev;     /* back-link */
} OLDORDER;

typedef struct OLDTRADE {
	char name[NAMELEN+1];		/* player's name (8 chars + 0x0) */
    int shares;					/* signed */
    double price;				/* average price for transaction */
    time_t zman;               /* as returned by time_t() function */
} OLDTRADE;

typedef struct OLDPORTFOLIO {
	char name[NAMELEN+1];
    double money;
    int shares;
    time_t zman;
} OLDPORTFOLIO;
/*---- end of old-style structure typedefs ----*/