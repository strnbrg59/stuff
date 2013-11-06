package jqc.rmi.server;

public class StandAloneDriver {
    //---------------------------------
    // constants

    static final int nIter = 1000;

    static final int nTraders = 6;
    static final String[] traders = {"Fed", "Ada", "Bob", "Eve", "Jeb", "Tom" };

    // Parameters for generating random orders.
    static final double bidBot = 6;
    static final double bidRange = 2.0;
    static final double askBot = 7;
    static final double askRange = 2.0;
    //---------------------------------

    public static void main( String[] args ) {

//		try{
    	    RemoteMarketServer market = new RemoteMarketServer();
//		}
//	  	catch (RemoteException e) { System.err.println(e); }
//		catch (Exception e)       { System.err.println(e); }

        for( int i=0;i<nIter;i++ ) {

            if( randGen.nextInt() < 0 )
            {
                double bidPrice = bidBot + 
                    Math.round(10*bidRange*randGen.nextDouble())/10.0;
                int traderNum = Math.abs(randGen.nextInt()) % nTraders;
                int bidUnits = (Math.abs(randGen.nextInt()) % 100) + 1;

                System.out.println( "newBid: " + traders[traderNum] + " " + bidPrice
                    + " " + bidUnits );
                market.placeOrder( traders[traderNum], "bid", bidPrice, bidUnits );
            }
            else
            {
                double askPrice = askBot 
                    + Math.round(10*askRange*randGen.nextDouble())/10.0;
                int traderNum = Math.abs(randGen.nextInt()) % nTraders;
                int askUnits = (Math.abs(randGen.nextInt()) % 100) + 1;

                System.out.println( "newAsk: " + traders[traderNum] + " " + askPrice
                    + " " + askUnits );
                market.placeOrder( traders[traderNum], "ask", askPrice, askUnits );
            }

        }
    }

// Random number generation
    private static java.util.Random randGen = new java.util.Random();

} // class StandAloneDriver
