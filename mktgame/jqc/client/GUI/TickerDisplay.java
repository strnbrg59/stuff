package jqc.client.GUI;

import java.awt.*;
import java.rmi.*;
import java.util.*;

public class TickerDisplay extends NotifiableTextArea
{
    public TickerDisplay( String assetName, int rows, int cols ) 
    {
        super( assetName+"TickerDisplay", rows, cols );
    }

    void update( String confirmation )
    {
        // confirmation may contain several lines, and not all of them will
        // will be trades.  Extract the trade lines and print those out.

        StringTokenizer tokenizer = new StringTokenizer( confirmation, "\n" );
        while( tokenizer.hasMoreTokens() == true )
        {
            String line = new String( tokenizer.nextToken() );
            if( jqc.server.Trade.isTrade( line ) )
            {
                append( line + "\n");
            }
        }
    }

    /** This is the update() that the AccountDisplay and OrdBookDisplay classes
      * use to do their things.  It'll get called when we click inside the
	  * TickerDisplay, but nothing should happen (because the thing gets updated
	  * automatically anyway).
    **/
    void update() {}
} 
