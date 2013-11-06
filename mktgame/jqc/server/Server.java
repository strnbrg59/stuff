package jqc.server;

import java.io.*;
import java.rmi.*;
import java.rmi.server.*;
import java.text.*;
import java.util.*;
import jqc.client.RemoteMarket;
import jqc.client.RemoteClient;
import jqc.client.GUI.OrdBookDisplay;

public class Server extends UnicastRemoteObject 
implements RemoteMarket 
{
    private AssetProperties assetProperties; 
    private Hashtable ordBooks;
	private Accounts traderAccounts;
    private Vector clients; // Registered for notification after placeOrder().
    private static Integer mutex = new Integer(1);


    public Server() throws RemoteException{ 
        super(); 

        assetProperties = new AssetProperties(G.assetInfoFile);

        // Create the ticker File object, and create the file itself if
        // it doesn't already exist.
        try
        {
            File tickerFile = new File( G.tickerFile );
            if( tickerFile.exists() == false )
            {
                FileOutputStream ticker = new FileOutputStream(G.tickerFile);
                ticker.close();
            }
        }
        catch (IOException ioe ) { ioe.printStackTrace(); }

        ordBooks = OrdBook.loadOrdbooks( assetProperties );
        clients = new Vector();

		traderAccounts = new Accounts();
    }

    public Properties getAssetProperties() throws RemoteException
    {
        return (Properties)assetProperties;
    }

    public synchronized boolean signIn( String trader, String pwd )
	throws RemoteException
    {
		return Passwords.verifyTrader( trader, pwd );
    }

	public String showHoldings( String trader )
	{
		return traderAccounts.show( trader );
	}

    public String showTicker( String assetName )
    {
        return new String("showTicker() is under construction");
    }

    public String placeOrder( String trader, String side,
        float price, int units, String asset ) throws RemoteException
    {
        Order.sanityCheck( price, units ); // throws RemoteException

		String confirmation = new String("");

        synchronized( mutex )
        {
            if( side.equals( "bid" ) ) {
                BidOrder bidOrd = new BidOrder( trader, price, units );
                confirmation  =
    				((OrdBook)(ordBooks.get(asset))).insert(bidOrd );
            }
            else {
                AskOrder askOrd = new AskOrder( trader, price, units );
    			confirmation = 
    	            ((OrdBook)(ordBooks.get(asset))).insert(askOrd );
            }
    
    		((OrdBook)(ordBooks.get(asset))).dump();
            Trade.record( confirmation, traderAccounts );
        }

        notifyClients( clients, asset, confirmation );

		return confirmation;
    } // placeOrder()

    // notifyClients(): not part of the RemoteMarket interface.
    // This gets called as the last thing placeOrder() and exerciseOptions() do.
    void notifyClients( Vector clients, String asset, String confirmation )
    {
        //System.out.println( "Server: clients.size()=" + clients.size() );

        // If this caused a trade, come up with the name of everybody affected --
        // trader, as well as the one or more traders on the other side.
        Vector partiesToTrade = new Vector();
        boolean tradeHappened = false;
        StringTokenizer tokenizer = new StringTokenizer( confirmation, "\n" );
        while( tokenizer.hasMoreTokens() )
        {
            String line = new String( tokenizer.nextToken() );
            if( Trade.isTrade( line ) )
            {
                tradeHappened = true;
                Trade trade = new Trade( line );
                partiesToTrade.addElement( trade.seller );
                partiesToTrade.addElement( trade.buyer );
            }
        }

        for( int i=0;i<clients.size();i++ )
        {
            try
            {
                RemoteClient rc = (RemoteClient)(clients.elementAt(i));
				//System.out.println("rc.getOID()=" + rc.getOID());
				System.out.flush();

                // Notify OrdBookDisplay objects.
                if( rc.getOID().equals( asset )  // notifies order book window
                ||  rc.getOID().equals( "Manual"    )) // notifies Manual client
                {
                    rc.notification();
                }

                // Notify AccountDisplay objects.
                for( int j=0;j<partiesToTrade.size();j++ )
                {
                    //System.out.println( "partiesToTrade.elementAt("+j+")=" +
                    //    partiesToTrade.elementAt(j) );
                    if(rc.getOID().equals((String)(partiesToTrade.elementAt(j))))
                        rc.notification();
                }

                // Notify TickerDisplay objects.  (They'll print out those lines of
                // confirmation that report trades.)
                if( rc.getOID().equals( asset + "TickerDisplay" ) )
                {
                    rc.notification( confirmation );
                }

                // Notify market makers.
                if( rc.getOID().equals( asset + "MktMaker" ) 
                &&  ( tradeHappened == true ) )
                {
                    rc.notification( confirmation );
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
                System.err.println( "\nFailed to reach client " + i +
                    ", removing him now." );
                clients.removeElement(clients.elementAt(i));
                i--; // because Vector has shifted.
            }
        }
    }

    public synchronized String cancelOrder( String trader, String side,
        float price, int units, String asset ) throws RemoteException
    {
	// A little kludgy.  Regardless of which side (bid/ask) the order we want
	// to cancel is on, we create a new BidOrder and go comparing it to
	// what's on either side of the order book.  Saves code.

		String confirmation = new String("");

		OrdBook book = (OrdBook)(ordBooks.get(asset));
		if( book == null )
			throw new RemoteException( "Invalid asset name" );
		else
		{
			BidOrder bidOrd = new BidOrder( trader, price, units );
			confirmation = book.cancel( bidOrd );
		}

        // Notify clients (same way as for a new order -- by dimming the order book
		// display).
        for( int i=0;i<clients.size();i++ )
        {
            try
            {
                RemoteClient rc = (RemoteClient)(clients.elementAt(i));

                if( rc.getOID().equals( asset )
                ||  rc.getOID().equals( ""    ))   // case of Manual client
                {
                    rc.notification();
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
                System.err.println( "Failed to reach client " + i +
                    ", removing him now." );
                clients.removeElement(clients.elementAt(i));
                i--; // because Vector has shifted.
            }
        }

		return confirmation;
    } // cancelOrder()

	public synchronized String exerciseOptions(String trader, int units, String asset)
		throws RemoteException
	{
		// Verify asset is really an option.
		String underlyingAsset = assetProperties.getUnderlying(asset);
		if( underlyingAsset.equals( "null" ) )
		{
			return( "This asset is not an option." );
		}
		// Verify trader owns at least as many units as he wants to exercise.
		else if( units >
		((Integer)((Hashtable)traderAccounts.get(trader)).get(asset)).intValue() )
		{
			return( "You do not own that many units." );
		}
		else if( units <= 0 )
		{
			return( "Cannot exercise nonpositive option units." );
		}

		System.out.println( "Called exerciseOptions " + units + " " + asset );
		int unitsLeft = units; // decrement every "trade" with counterparties.
		// FIXME: We'll need to randomize our choice of counterparties.
		Enumeration traderNames = traderAccounts.keys();
		System.out.println( "Obtained traderAccounts.keys()." );
		while( traderNames.hasMoreElements() && (unitsLeft>0) )
		{
			String counterPartyName = (String)(traderNames.nextElement());
			System.out.println( "counterPartyName=" + counterPartyName );
			Hashtable account = 
				(Hashtable)traderAccounts.get(counterPartyName);
			int counterPartyUnits = 
				((Integer)( account.get( asset ))).intValue();
			System.out.println( "counterPartyUnits=" + counterPartyUnits );

			if( counterPartyUnits < 0 )
			{
				int unitsToCross = Math.min( -counterPartyUnits, unitsLeft );
				System.out.println( "unitsToCross=" + unitsToCross );
				String buyer, seller;  // Of the underlying, as it were.
				if( assetProperties.isaCall( asset ) )
				{
					buyer = trader;
					seller = counterPartyName;
				}
				else
				{
					buyer = counterPartyName;
					seller = trader;
				}
				float strikePrice = assetProperties.getStrikePrice( asset );
				System.out.println( "buyer="+buyer+", seller="+seller 
					+ ",strikePrice="+strikePrice );
				unitsLeft -= unitsToCross;

				String confirmation = Trade.genConfirmation(
					buyer, seller, underlyingAsset, unitsToCross, strikePrice);
				Trade.record( confirmation, traderAccounts);
		        notifyClients( clients, underlyingAsset, confirmation );
				confirmation = Trade.genConfirmation( counterPartyName, trader, 
					asset, unitsToCross,(new Float(0.0)).floatValue());
				Trade.record( confirmation,	traderAccounts);
		        notifyClients( clients, asset, confirmation );
			}
		}

		return( "exerciseOptions() completed normally." );
	}

    public String showOrderBook( String assetName ) throws RemoteException
    {
        String result = ((OrdBook)(ordBooks.get(assetName))).toString() ;
        return result;
    }

	public Order[] showInsideQuotes( String assetName )
	{
		OrdBook ob = (OrdBook)(ordBooks.get(assetName));
		Order[] result = new Order[2];

		if( ob.bid.head != null )
			result[0] = new BidOrder(ob.bid.head);
		if( ob.ask.head != null )
			result[1] = new AskOrder(ob.ask.head);

		return result;
	}

    public void registerClient( RemoteClient rc ) throws RemoteException
    {
        synchronized(clients) 
        {
            clients.addElement( rc );
        }
    }

    public void unregisterClient( RemoteClient rc ) throws RemoteException
    {
        synchronized(clients) 
        {
            System.out.println( "unregistering client." );
            if( clients.removeElement( rc ) == false )
                throw new RemoteException( 
                    "Failed to unregister client; couldn\'t find it." );
        }
    }

    public static void main( String[] args )
    {
        try {
            Server market = new Server();
            String name = 
                System.getProperty("marketname", "rmi://:19594/BerkeleyStockExchange");
            Naming.rebind(name, market);
            System.out.println( "\n" + name + " is open and ready for business." );
        }
        catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }

        /*
         * Here's a neat demonstration of how we can run client-like code
         * on the server:
        try 
        {
            String url = System.getProperty("market", "rmi:///BerkeleyStockExchange");
            RemoteMarket smarket = (RemoteMarket) Naming.lookup(url);
            String ob = smarket.showOrderBook( "bond" );
            System.out.println( "smarket.showOrderBook() returned:\n" + ob );
        }
        catch (Exception ee ) { System.err.println( ee ); }
        */

    }

} // class Server







