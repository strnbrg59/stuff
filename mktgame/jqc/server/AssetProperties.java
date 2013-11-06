package jqc.server;

import java.io.*;
import java.util.*;

// Strategy: main() constructs an AssetProperties object.  That object will
// eventually contain information like the asset's currency, underlying (if
// it's a contingent claim), etc.
// main() then passes that to OrdBook.loadOrdBooks(), which goes to disk, 
// loads the order books (if they exist), and returns a Hashtable keyed 
// by the assets' names.

public class AssetProperties extends Properties
{
    public AssetProperties( String infilename )
    {
        super();

        try 
        {
            load(new BufferedInputStream( new FileInputStream( infilename )));
        }
        catch (FileNotFoundException e) 
        {
            System.err.println(e);
			e.printStackTrace();
            System.exit(1);
        }            
        catch (IOException e) 
        {
            System.err.println(e);
			e.printStackTrace();
            System.exit(1);
        }                
	}

	public String getCurrency( String assetName )
	{
		String assetPropStr = (String)get(assetName);
		StringTokenizer st = new StringTokenizer( assetPropStr );
		return (String)st.nextToken();
	}

	public String getForcedSalesMarket( String assetName )
	{
		String assetPropStr = (String)get(assetName);
		StringTokenizer st = new StringTokenizer( assetPropStr );
		st.nextToken();
		return (String)st.nextToken();
	}

	/** Underlying asset field is actually three things concatenated:
	  * a '+' or '-' to indicate call-type or put-type option, a strike
	  * price, and finally the name of the underlying asset.
	*/
	public String getUnderlying( String assetName )
	{
		String assetPropStr = (String)get(assetName);
		StringTokenizer st = new StringTokenizer( assetPropStr );
		st.nextToken();
		st.nextToken();

		String wholething = st.nextToken();
		int i=0;
		while( ! Character.isLetter(wholething.charAt(i)) )
			i++;
		String underlyingName = wholething.substring(i);
		return underlyingName;
	}

	/** Return true if the option is an option to buy -- indicated by a
	  * '+' prefixing the strike price in the asset info file. 
	*/
	public boolean isaCall( String assetName )
	{
		String assetPropStr = (String)get(assetName);
		StringTokenizer st = new StringTokenizer( assetPropStr );
		st.nextToken();
		st.nextToken();
		String wholething = st.nextToken();

		boolean result;
		if( wholething.charAt(0) == '+' )
			result = true;
		else
			result = false;
		return result;
	}

	/** Strike price of an option -- indicated by a prefix to the "underlying"
	  * field of the AssetProperties object.
	*/
	public float getStrikePrice( String assetName )
	{
		String assetPropStr = (String)get(assetName);
		StringTokenizer st = new StringTokenizer( assetPropStr );
		st.nextToken();
		st.nextToken();
		String wholething = st.nextToken();

		int startOfChars=0;
		while( ! Character.isLetter(wholething.charAt( startOfChars )))
			startOfChars++;
		float result = (new Float(wholething.substring(1,startOfChars))).floatValue();
		return result;
	}

} // class AssetProperties    


/*
		// The values are now just strings.  We want to keep them
		// in a structure.
		Enumeration assetNames = keys();
		while( assetNames.hasMoreElements() )
		{
			String assetName = (String)assetNames.nextElement();
			String[] assetProps = (String[])get(assetName);
			System.out.println( assetProps );
		}
*/
