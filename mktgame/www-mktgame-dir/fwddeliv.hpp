/**** TYPEDEFS for old-style structures ****/
typedef struct OLDOrder {
    char name[NAMELEN+1];   /* player's name (8 chars + 0x0) */
    int mkt_or_lim;    		/* MKT|LIM */
    int buy_or_sell;   		/* BUY|SELL */
    int cancel;                 /* YES|NO */
    int shares;        		/* unsigned */
    double price;
    time_t zman;            /* as returned by time_t() function */
    struct OLDOrder *next; 	/* link */
    struct OLDOrder *prev;     /* back-link */
} OLDOrder;

typedef struct OLDTrade {
	char name[NAMELEN+1];		/* player's name (8 chars + 0x0) */
    int shares;					/* signed */
    double price;				/* average price for transaction */
    time_t zman;               /* as returned by time_t() function */
} OLDTrade;

typedef struct OLDPORTFOLIO {
	char name[NAMELEN+1];
    double money;
    int shares;
    time_t zman;
} OLDPORTFOLIO;
/*---- end of old-style structure typedefs ----*/