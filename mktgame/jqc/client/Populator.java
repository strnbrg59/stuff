package jqc.client;

import java.rmi.*;

public class Populator {

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

		try {
			market.placeOrder( "Ed",     "bid", (float)7.1, 10, "stock" );
			market.placeOrder( "Ted",    "bid", (float)7.2, 20, "stock" );
			market.placeOrder( "Fred",   "ask", (float)8.1, 30, "stock" );
			market.placeOrder( "Ned",    "ask", (float)8.2, 40, "stock" );

			market.placeOrder( "Ed",     "bid", (float)9.5, 11, "bond" );
			market.placeOrder( "Ted",    "bid", (float)9.6, 21, "bond" );
			market.placeOrder( "Fred",   "ask", (float)9.7, 31, "bond" );
			market.placeOrder( "Ned",    "ask", (float)9.8, 41, "bond" );
		}
        catch (RemoteException e)         { System.err.println(e); }
    } // main()
} // class Populator



