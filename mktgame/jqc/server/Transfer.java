package jqc.server;

import java.io.*;
import java.util.*;

// Transfer: money transfer between two traders.  Useful for paying dividends,
// initial endowments, and, in combination with a sale, an asset transfer.
// 
// The String representation is "<giver> gave <amount> money to <receiver>".
public class Transfer
{
	String giver;
	String receiver;
	float  amount;	

	public Transfer( String confirmation )
	{
		StringTokenizer st = new StringTokenizer( confirmation );

		giver = new String( st.nextToken() ); st.nextToken();     // "gave"
		amount = Float.valueOf( st.nextToken() ).floatValue(); 
		st.nextToken(); st.nextToken();  // "money" "to"
		receiver = new String( st.nextToken() );
	}

	/** Return true if argument is a transfer. **/
	static public boolean isTransfer( String confirmation )
	{
        if( ( confirmation.indexOf(" gave ") != -1 )
        &&  ( confirmation.indexOf(" money "  ) != -1 )
        &&  ( confirmation.indexOf(" to "    ) != -1 ))
			return true;
		else
			return false;
	}
}
