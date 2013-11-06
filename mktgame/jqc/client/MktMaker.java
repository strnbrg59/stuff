// MktMaker: maintain a narrow B-A spread, adjusting it in response
// to the order flow.  Don't do anything if either side of the order
// book is empty; that's market break, and requires human intervention.
// Each MktMaker process is asset-specific, so launch it by specifying
// the asset name as the first (and only) command-line argument.

package jqc.client;

import jqc.server.Order;
import java.rmi.*;
import java.rmi.server.*;
import java.util.*;

public class MktMaker implements RemoteClient 
{

    static String url;
    static RemoteMarket market;
    static String asset = null;
    static jqc.server.Order[] oldInsideQuote = null;
    static jqc.server.Order[] newInsideQuote = null;
    static boolean mainReturned = false;

    static final float pctIdealSpread = (new Float(0.05)).floatValue(); //decimal fraction

    void connectToServer()
    {
        try 
        {
            url    = System.getProperty("marketname", "rmi://:19594/BerkeleyStockExchange");
            market = (RemoteMarket) Naming.lookup(url);
            UnicastRemoteObject.exportObject(this);
            market.registerClient( this );
        }
        catch (RemoteException e) 
		{ 
			e.printStackTrace();
			System.exit(1);
		}
        catch (Exception e) 
		{
            e.printStackTrace();
			System.exit(1);
        }
    }


    // main() doesn't have much to do.  
    public static void main(String[] args) 
    {
        MktMaker mktMaker = new MktMaker();
        mktMaker.connectToServer();

		if( args.length != 1 )
		{
			System.err.println( "Usage: MktMaker assetName" );
            System.exit(1);
		}
		asset = args[0];

        try{ oldInsideQuote = market.showInsideQuotes( asset ); }
        catch (RemoteException e ) { e.printStackTrace(); }

        System.out.println( "MktMaker now in action in " + asset + " market." );    

        mainReturned = true;
    } // main()

    public String getOID() { return new String( asset + "MktMaker" ); }

    //--------------------------
    // notification() is where the work really gets done.  The mktMaker
    // doesn't need to react unless there's been a trade.  When a trade does
    // occur, he has to look at the inside spread, and compare it to what
    // it was previously.  If it has narrowed, just update the "old"
    // spread.  If it's widened, then place an order or two, to narrow it and
    // of course to move it in the direction the price has gone.
    public void notification(){}
    public void notification( String confirmation )
    {
/*
		System.out.println( "Inside MktMaker.notification()." );
		System.out.flush();
*/
        // Don't try doing anything unless all the initialization stuff is finished.
        if( mainReturned == false )
            return;

        //-------------------------------
        // Figure out how prices have changed.
        try{ newInsideQuote = market.showInsideQuotes( asset ); }
        catch (RemoteException e ) { e.printStackTrace(); }

        float oldBestBid = -1;
        float oldBestAsk = -1;
        float newBestBid = -1;
        float newBestAsk = -1;

        if( oldInsideQuote[0] != null )
            oldBestBid = oldInsideQuote[0].price();
        if( oldInsideQuote[1] != null )    
            oldBestAsk = oldInsideQuote[1].price();
        if( newInsideQuote[0] != null )
            newBestBid = newInsideQuote[0].price();
        if( newInsideQuote[1] != null )
            newBestAsk = newInsideQuote[1].price();

        System.out.println( "Old inside quotes: " + oldBestBid +
            ", " + oldBestAsk );
        System.out.println( "New inside quotes: " + newBestBid +
            ", " + newBestAsk );
        //-------------------------------

		// Don't do anything if there's a market break.
		if( newBestAsk == -1
		||  newBestBid == -1
		||  oldBestAsk == -1
		||  oldBestBid == -1 )
		{
			System.out.println( "Market break: MktMaker giving up." );
			return;
		}

        //-------------------------------
        // Place orders.
        if ( newBestAsk > oldBestAsk )
        {
            System.out.println( "MktMaker: gonna up the ask." );            
	        cancelOwnOrders( "MktMaker", asset );

            float idealSpread = newBestAsk * pctIdealSpread;

            try{ market.placeOrder( "MktMaker", "ask", newBestAsk, 50, asset);}
            catch (RemoteException e) { e.printStackTrace(); }
            
            try{ market.placeOrder( "MktMaker", "bid", newBestAsk-idealSpread,50,asset);}
            catch (RemoteException e) { e.printStackTrace(); }
        }

        if ( newBestBid < oldBestBid )
        {
            System.out.println( "MktMaker: gonna cut the bid." );            
	        cancelOwnOrders( "MktMaker", asset );

            float idealSpread = newBestAsk * pctIdealSpread;

            try{ market.placeOrder( "MktMaker", "ask", newBestBid+idealSpread,50,asset);}
            catch (RemoteException e) { e.printStackTrace(); }
            
            try{ market.placeOrder( "MktMaker", "bid", newBestBid, 50, asset);}
            catch (RemoteException e) { e.printStackTrace(); }
        }
        //-------------------------------

        oldInsideQuote = newInsideQuote;
    }

    /** Cancel all of MktMaker's orders. **/
	void cancelOwnOrders( String trader, String asset ) 
	{
/*
		System.out.println( "Inside cancelOwnOrders()." );
		System.out.flush();
*/
		String allOrders = null;
		try{ allOrders = new String( market.showOrderBook(asset) ); }
		catch (RemoteException re ) { re.printStackTrace(); }

        StringTokenizer tokenizer = new StringTokenizer( allOrders, "\n", false );
		Vector ownOrders = new Vector();

		while( tokenizer.hasMoreTokens() == true )
		{
			// Grab just trader's own.
			// Trader's name is either the first (bids) or last (asks) token
			// on the line.
			String nextOrder = new String(tokenizer.nextToken());
			StringTokenizer lineTokenizer = new StringTokenizer( nextOrder );
			
			String possibleTraderName = lineTokenizer.nextToken();
			if( possibleTraderName.equals( trader ) )
				ownOrders.addElement( nextOrder );
			else
			{
				while( lineTokenizer.hasMoreTokens() == true )
					possibleTraderName = lineTokenizer.nextToken();
				if( possibleTraderName.equals( trader ))
					ownOrders.addElement( nextOrder );
			}

		}

        System.out.println( "ownOrders.size()=" + ownOrders.size() );

        for( int i=0;i<ownOrders.size();i++ )
        {
            String orderRep = (String)(ownOrders.elementAt(i));
	        StringTokenizer orderTokenizer = new StringTokenizer(orderRep);
    	    String tok1 = new String( orderTokenizer.nextToken() );
			int units;
			float price;
			String side=null;
        	if( tok1.equals( "MktMaker" ))
	        {
    	        side = "bid";
        	    units = Integer.parseInt(orderTokenizer.nextToken());
            	price = Float.valueOf(orderTokenizer.nextToken()).floatValue();
	        }
    	    else
        	{
            	side = "ask";
	            price = Float.valueOf(tok1).floatValue();
    	        units = Integer.parseInt(orderTokenizer.nextToken());
        	}

            try {
                market.cancelOrder( "MktMaker", side, price, units, asset );
            }
            catch (RemoteException e) { e.printStackTrace(); }
            // Doesn't matter whether we call it a bid or an ask. 

/*
            System.out.println( "sleep(5)" ); System.out.flush();
			try { java.lang.Thread.sleep(10000); }
			catch (InterruptedException ie) { ie.printStackTrace(); }
*/
        }
	}

}
