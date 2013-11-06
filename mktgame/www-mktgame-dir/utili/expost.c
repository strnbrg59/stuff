/* Account for portfolio performance by creating a time-series of
 * one trader's ex-post portfolio value.
 *
 * Algorithm: You only need open the act file for the currency; it
 *   will tell you what securities were traded, and in what amounts.
 * 
*/
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

        static struct {  /* to keep track of ex-post values */
            int units;
            double expost_value;
	} portf[100];  /* need as many as we have securities. */

    /* load data about the securities */
    Securitydata = load_securities();
    /* load players' names */
    N_players = load_players( &Names );

    /* prompt user for security name */
    printf( "Enter currency name > " );
    scanf( "%s", security );
    sec = find_sec( security, Securitydata );

    /* open files */
    strcpy( infilename, security );
    strcat( infilename, "act/diary.out" );
    strcpy( outfilename, "expost." );
    strcat( outfilename, security );
    infile = fopen( infilename, "rb" );
    outfile = fopen( outfilename, "w" );

    while (!feof(infile) ) {
	fread( &order, 1, sizeof(ORDER), infile );     
	fread( &trade, 1, sizeof(TRADE), infile );  /* only one we'll use */
	fread( &holding, 1, sizeof(HOLDINGS), infile );

        // portf[ sec_num( trade.other_sec ) ].units += trade.amount;
        // sec_num() returns the ordinal number of the security.
        // You'll have to write that function here.

        // portf[ sec_num( trade.other_sec ) ].expost_value 
               += trade.amount*expost_val[sec_num( trade.other_sec ) ]; 
        

	fclose( infile );
	fclose( outfile );
    return 0;
    }
}
