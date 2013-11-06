package jqc.client;

import java.rmi.*;
import java.util.Random;

public class Poisson {

    static String url;
    static RemoteMarket market;
    static {
        try 
        {
            url    = System.getProperty("marketname", "rmi://:19594/BerkeleyStockExchange");
            market = (RemoteMarket) Naming.lookup(url);
        }
        catch (RemoteException e) 
		{ 
			System.err.println(e); 
			System.exit(1);
		}
        catch (Exception e) 
		{
            System.err.println(e);
			System.exit(1);
        }
    }

    public static void main(String[] args) {

		if( args.length != 1 )
		{
			System.err.println( "Usage: Poisson assetName" );
		}

		String assetName = args[0];

		Random rand = new Random();

		for(;;)
		{
			float price = -1;
			int units = Math.abs(rand.nextInt()%20);
			int iside = rand.nextInt();
			String side = new String("");
			if( iside < 0 )
				side = "bid";
			else
				side = "ask";

			try 
			{
				jqc.server.Order[] insideQuote = market.showInsideQuotes( assetName );
				jqc.server.Order bestBid = insideQuote[0];
				jqc.server.Order bestAsk = insideQuote[1];
				if( side.equals( "bid" ) &&	( bestAsk != null ) )
				{
					price = bestAsk.price();
					units = units <= bestAsk.units() ? units : bestAsk.units();
				}
				if( side.equals( "ask" ) &&	( bestBid != null ) )
				{
					price = bestBid.price();
					units = units <= bestBid.units() ? units : bestBid.units();
				}
			
				System.out.println( "Poisson ordering " + units + " units." );

				if( price != -1 )
					market.placeOrder( "Poisson", side, price, units, assetName );
			}
        	catch (RemoteException e) { System.err.println(e); }

			try { java.lang.Thread.sleep(20000); }
			catch (InterruptedException ie) {}
		}
    } // main()
} // class Poisson


