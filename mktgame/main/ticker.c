/* print the trading history in rpn-readable format */
#include <stdio.h>
#include <string.h>
#include "aqc.h"

/**** GLOBAL VARIABLES ****/
SECURITY **Securitydata;
int N_sec;
int N_players;
NAMES *Names;

/**** FUNCTION DEFINITIONS ****/
main() {
        char infilename[ACTNAMELEN+1], outfilename[ACTNAMELEN+1],
	     security[NAMELEN+1];
	ORDER order;
	TRADE trade;
	HOLDINGS holding;
        SECURITY *sec;
	FILE *infile, *outfile;

    /* load data about the securities */
    Securitydata = load_securities();
    /* load players' names */
    N_players = load_players( &Names );

	/* prompt user for security name */
	printf( "Enter security name > " );
	scanf( "%s", security );
	sec = find_sec( security, Securitydata );

	/* open files */
	strcpy( infilename, security );
	strcat( infilename, "act/diary.out" );
	strcpy( outfilename, "ticker." );
	strcat( outfilename, security );
	infile = fopen( infilename, "rb" );
	outfile = fopen( outfilename, "w" );

	while (!feof(infile) ) {
		fread( &order, 1, sizeof(ORDER), infile );
		fread( &trade, 1, sizeof(TRADE), infile );
		fread( &holding, 1, sizeof(HOLDINGS), infile );

            if( ismoney(sec) == NO )
    		fprintf( outfile, "%5d %8.3f %ld\n",
	    	    (int)(trade.amount), (float)(trade.price), trade.zman );
            else if( trade.name[0]!=0 ) /* ==0 indicates this currency-security was being used as a currency, so we don't want to display this trade */
    		fprintf( outfile, "%8.2f %8.3f %ld\n",
                    (float)(trade.amount), (float)(trade.price), trade.zman );
	}

	fclose( infile );
	fclose( outfile );
    return 0;
}
