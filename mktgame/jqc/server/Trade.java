package jqc.server;

import java.io.*;
import java.util.*;

// Trade: a class for producing canonical confirmations, and for parsing
// those confirmations.
public class Trade
{
	String buyer;
	String seller;
	String assetName;
	int    units;
	float  price;	

	Trade( String buyer, String seller, String assetName, int units, float price )
	{
		this.buyer 		= new String( buyer );
		this.seller 	= new String( seller );
		this.assetName 	= new String( assetName );
		this.units		= units;
		this.price		= price;
	}

	Trade( String confirmation )
	{
		StringTokenizer tokenizer = new StringTokenizer( confirmation );

		buyer = new String( tokenizer.nextToken() ); tokenizer.nextToken(); // discards "bought"
		units = Integer.parseInt( tokenizer.nextToken() );
		assetName = new String( tokenizer.nextToken() ); tokenizer.nextToken(); // discards "from"
		seller = new String( tokenizer.nextToken() ); tokenizer.nextToken(); // discards "at"		
		price = Float.valueOf( tokenizer.nextToken() ).floatValue();
	}

	/** Produce canonical confirmation string **/
	static String genConfirmation( String buyer, String seller, String assetName,
			        			   int units, float price )
	{
        return new String( buyer + " bought " + units + " " + assetName +
            " from " + seller + " at " + price );
	}

	public String toString()
	{
		return new String( genConfirmation( buyer, seller, assetName,
											units, price ));
	}

	/** Return true if argument is a trade confirmation. **/
	static public boolean isTrade( String confirmation )
	{
        if( ( confirmation.indexOf(" bought ") != -1 )
        &&  ( confirmation.indexOf(" from "  ) != -1 )
        &&  ( confirmation.indexOf(" at "    ) != -1 ))
			return true;
		else
			return false;
	}

    /** Record the trade, if any, to disk. Argument is a confirmation, which
      * is not necessarily a trade.  We identify trades by looking for substrings
      * "bought" and "from".  Also, update the traderAccounts in memory.
     **/
    static void record( String confirmation, Accounts traderAccounts )
    {
        // Break confirmation apart at the \n characters; not all
        // lines represent trades (the last one may say "Inserted into
        // order book").

        StringTokenizer tokenizer = new StringTokenizer(confirmation, "\n", false );
        while( tokenizer.hasMoreTokens() )
        {
            String line = new String( tokenizer.nextToken() );
			if( Trade.isTrade( line ) )
            {
              try {   
                PrintWriter writer = new PrintWriter( 
                    new FileOutputStream( G.tickerFile, true ), true );
                writer.println( line );

				traderAccounts.update( new Trade(line) );		
              }
              catch (IOException ioe ) 
			  { 
				System.err.println( ioe ); 
				ioe.printStackTrace();
			  }
            }
        }
    } // record()
}
