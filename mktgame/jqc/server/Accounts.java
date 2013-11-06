package jqc.server;

import java.io.*;
import java.text.NumberFormat;
import java.util.*;

/** Accounts class.  A Hashtable holding data on all traders' positions
  * in all assets, and money.  The Hashtable is keyed on traders' names.
  * The Hashtable's values are themselves Hashtables -- keyed on asset
  * names, with values equal to the trader's position in that asset.
**/
class Accounts extends Hashtable 
{

	/** ctor: load trader and asset names from passwords.dat and
	  * assetInfo.dat.  Figure out the holdings by scanning the
	  * trade history -- data/ticker.out.
	**/
	Accounts()
	{
		super();

		// Create the Hashtable, with entries for all traders and asset names, but
		// with all holdings set to zero.
		zero();

		// Set the holdings to their correct values, based on the record in the ticker
		// file.
		loadFromTickerFile();
	} // Accounts ctor

	/** Create the Hashtable, with entries for all traders and asset names, but
	 * with all holdings set to zero.
	**/
	private void zero()
	{
		//-----------------------------
		// Load trader names.
        Properties traderHash = new Properties();
        try {
            traderHash.load(new BufferedInputStream(
                new FileInputStream(G.passwordsFile)));
        }
        catch (FileNotFoundException e) {
			System.err.println(e);
            e.printStackTrace();
            System.exit(1);
        }            
        catch (IOException e) {
			System.err.println(e);
            e.printStackTrace();
            System.exit(1);
        }            
		//------------------------------

		//------------------------------
		// Load asset names into a Hashtable that we will clone,
		// to obtain the value that goes with each trader's name
		// in the Hashtable that is *this.
		Hashtable assetHash = new Hashtable();
		AssetProperties assetProperties = new AssetProperties(G.assetInfoFile);
        Enumeration assetNames = assetProperties.keys();

        for( int i=0;i<assetProperties.size();i++ )
        {
            String key = new String( (String)(assetNames.nextElement()));;
            assetHash.put( key, new Integer(0) );
        }

		assetHash.put( "money", new Float(0.0) );
		//------------------------------

		//------------------------------
		// Finally, create *this.
		Enumeration traderNames = traderHash.keys();
		for( int i=0;i<traderHash.size();i++ )
		{
			String key = new String( (String)(traderNames.nextElement()));
			Hashtable value = new Hashtable();
			value = (Hashtable)assetHash.clone();

			put( key, value );
		}

	} // zero()

	/** Set the holdings to their correct values, based on the record in the ticker file.
	  * Ticker file records can be of two types -- trades and transfers.
	**/
	private void loadFromTickerFile()  
	{
		// All you have to do is call update() on each trade!
		try
		{
			BufferedReader infile = new BufferedReader( new FileReader(G.tickerFile));
			while( infile.ready() )
			{
				String line = new String(infile.readLine());
				if( Trade.isTrade(line) )
					update( new Trade(line) );
				if( Transfer.isTransfer(line) )
					update( new Transfer(line) );
			}
			
			infile.close();
		}
		catch (IOException ioe) { System.err.println(ioe); ioe.printStackTrace(); }
	}

	/** Update for a transfer. For simplicity, transfers are always money transfers.
      * If we need to transfer an asset, we can sell it and then transfer money to
	  * offset the price.
	**/
	void update( Transfer transfer )
	{
		Hashtable giverHash = (Hashtable)get( transfer.giver );
		float giverHolding = ((Float)(giverHash.get("money"))).floatValue() -
							 transfer.amount;
		giverHash.remove( "money" );
		giverHash.put( "money", new Float(giverHolding) );

		Hashtable receiverHash = (Hashtable)get( transfer.receiver );
		float receiverHolding = ((Float)(receiverHash.get("money"))).floatValue() +
							 transfer.amount;
		receiverHash.remove( "money" );
		receiverHash.put( "money", new Float(receiverHolding) );
	}

	/** Update for a trade. **/
	void update( Trade trade )
	{
		// Stick it into *this.  
		Hashtable buyerHash = (Hashtable)get( trade.buyer );
		int buyerHolding = 
			((Integer)(buyerHash.get( trade.assetName ))).intValue() + trade.units;
		buyerHash.remove( trade.assetName );
		buyerHash.put( trade.assetName, new Integer( buyerHolding ));
		float buyerMoney =
			((Float)(buyerHash.get( "money" ))).floatValue() - trade.units * trade.price;
		buyerHash.remove( "money" );
		buyerHash.put( "money", new Float(buyerMoney) );
	
		Hashtable sellerHash = (Hashtable)get(trade.seller);
		int sellerHolding = 
			((Integer)(sellerHash.get(trade.assetName))).intValue() - trade.units;
		sellerHash.remove( trade.assetName );
		sellerHash.put( trade.assetName, new Integer( sellerHolding ));
		float sellerMoney =
			((Float)(sellerHash.get( "money" ))).floatValue() + trade.units * trade.price;
		sellerHash.remove( "money" );
		sellerHash.put( "money", new Float(sellerMoney) );

	} // update()

	public String show( String trader )
	{
		String result = new String("");
		Hashtable traderHash = (Hashtable)get(trader);
		Enumeration traderNames = traderHash.keys();

		for( int i=0;i<traderHash.size();i++ )
		{
			String assetName = new String( (String)(traderNames.nextElement()));

			if( assetName.compareTo( "money" ) != 0 )
			{
				Integer holdings = (Integer)(traderHash.get( assetName ));
				result += assetName + ": " + blanks(9-assetName.length()) + 
						blanks(6-(holdings.toString()).length()) + holdings.intValue() 
						+ "\n";
			}
		}
		Float money = (Float)(traderHash.get( "money" ));

		// Format the money amount...alot of work for no more than a "%6.2f".
		NumberFormat priceFormat = NumberFormat.getInstance();
		priceFormat.setMaximumFractionDigits( 1 );
		priceFormat.setMinimumFractionDigits( 1 );
		String strMoney = new String(priceFormat.format( money.floatValue() ));;
		result += "money: " + blanks(9-5) + blanks(6-strMoney.length()) +
				strMoney;

		return result;
	}

	/** Return n blanks. **/
	String blanks( int n )
	{
		String result = new String("");
		for( int i=0;i<n;i++ )
			result += " ";
		return result;
	}
} // class Accounts


