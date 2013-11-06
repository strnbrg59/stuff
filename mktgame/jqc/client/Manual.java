package jqc.client;

import java.rmi.*;
import java.rmi.server.*;
import java.io.*;
import java.lang.*;
import java.text.*;
import java.util.*;

public class Manual implements RemoteClient
{

    String url;
    static RemoteMarket market;

    protected void finalize()
    {
        try{ market.unregisterClient( this ); }
        catch (RemoteException e) { System.err.println(e); }
    }

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
			System.err.println(e); 
			System.exit(1);
		}
        catch (Exception e) 
		{
            System.err.println(e);
			System.exit(1);
        }
    }
    public void notification( String confirmation ){}

    public void notification()
    {
        System.out.println( "Client: Server just sent notification." );
    }

    public String getOID() { return new String("Manual"); } 

    public static void main(String[] args) {

        Manual manual = new Manual();
        manual.connectToServer();

        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		String trader = new String("");

		// Prompt for name and password.
		try {
			System.out.print( "name: " );
			trader = in.readLine();
			System.out.print( "password: " );
			String pwd = in.readLine();

			if(	market.signIn( trader, pwd ) == false )
				System.exit(1);
        }
		catch (RemoteException e)
		{
            System.err.println( e );
            System.exit(1);
        }
		catch( IOException e )
		{
            System.err.println( e );
            System.exit(1);
        }

		//-------------------------------
		// Interpreter loop.
        String cmdLine = new String("");

        for(;;) // Exit on "quit" or "exit" at command line.
        {
            System.out.print("jqc> ");

            try{ cmdLine = in.readLine(); }
            catch ( IOException e )
            {
                System.err.println( e );
                continue;
            }

            CmdVector cmdArgs = new CmdVector(cmdLine);
            try
            {
                execIt( cmdArgs, trader );
            }
            catch (InvalidCommandException e) { System.err.println(e); }
	        catch (RemoteException e)         { System.err.println(e); }
		}
		//-------------------------------

    } // main()
    //------------------

    static void execIt( CmdVector cmdArgs, String trader ) 
        throws InvalidCommandException, RemoteException
    {
        if( cmdArgs.size() == 0 )
            return;
        
        if( ( (cmdArgs.stringAt(0)).compareTo( "exit" ) == 0 )
        ||  ( (cmdArgs.stringAt(0)).compareTo( "quit" ) == 0 )
        ||  ( (cmdArgs.stringAt(0)).compareTo( "bye"  ) == 0 ))
        {
            System.out.println( "bye" );
            System.exit(0);
        }

        else if (((cmdArgs.stringAt(0)).toLowerCase().compareTo( "bid" ) == 0 )
			 ||  ((cmdArgs.stringAt(0)).toLowerCase().compareTo( "ask" ) == 0 ))
        {
			if( cmdArgs.size() != 5 )
				throw new InvalidCommandException( 
					new String("Syntax: <bid|ask> <price> for <units> <assetName>."));

            String for_word = cmdArgs.stringAt(2);
            if( for_word.compareTo( "for" ) != 0 )
				throw new InvalidCommandException( 
					new String("Syntax: <bid|ask> <price> for <units> <assetName>."));

			String side = cmdArgs.stringAt(0);
			float price;		
            String assetName = cmdArgs.stringAt(4);
            int units;
            try {
	            price = cmdArgs.floatAt(1);
                units = cmdArgs.intAt(3);
            }
            catch (NumberFormatException e)
            {
                System.err.println( e );
                return;
            }
			
			String confirmation = new String (
	            market.placeOrder( trader, side, price, units, assetName )
			);
			System.out.println( confirmation );
        } // placeOrder()

		else if( (cmdArgs.stringAt(0)).compareTo( "cancel" ) == 0 )
		{
			if( cmdArgs.size() != 6 )
				throw new InvalidCommandException( new String(
					"Syntax: cancel <bid|ask> <price> for <units> <assetName>."));
			// We don't do anything with <bid|ask>, but require the user to 
			// supply it just for consistency with placeOrder().

            String for_word = cmdArgs.stringAt(3);

			float price;		
            String assetName = cmdArgs.stringAt(5);
            int units;
            try {
	            price = cmdArgs.floatAt(2);
                units = cmdArgs.intAt(4);
            }
            catch (NumberFormatException e)
            {
				System.err.println( 
					"Syntax: cancel <bid|ask> <price> for <units> <assetName>.");
                return;
            }

			String side = new String("");
			if( (cmdArgs.stringAt(1).toLowerCase().compareTo( "bid" ) != 0 )
			&&  (cmdArgs.stringAt(1).toLowerCase().compareTo( "ask" ) != 0 ))
			{
				System.err.println( 
					"Syntax: cancel <bid|ask> <price> for <units> <assetName>.");
                return;
            }
			else
				side = cmdArgs.stringAt(1).toLowerCase();
			
			String confirmation = new String(
				market.cancelOrder( trader, side, price, units, assetName ));
			System.out.println( confirmation );
		} // cancelOrder()

		else if( (cmdArgs.stringAt(0)).compareTo( "exerciseOptions" ) == 0 )
		{
			if( cmdArgs.size() != 3 )
				throw new InvalidCommandException( new String(
					"Syntax: exerciseOptions <units> <optionname>" ));

			int units;
            try 
			{
                units = cmdArgs.intAt(1);
            }
            catch (NumberFormatException e)
            {
				System.err.println( 
					"Syntax: exerciseOptions <units> <optionName>.");
                return;
            }
            String assetName = cmdArgs.stringAt(2);
			
			String confirmation =
				market.exerciseOptions( trader, units, assetName );
			System.out.println( confirmation );
		}

        else if( (cmdArgs.stringAt(0)).compareTo( "showOrderBook" ) == 0 )
        {
			if( cmdArgs.size() != 2 )
				throw new InvalidCommandException( 
					new String("Syntax: showOrderBook <assetName>."));

            String assetName = new String(cmdArgs.stringAt(1));
            System.out.println( market.showOrderBook( assetName ) );
        }

		else if( (cmdArgs.stringAt(0)).compareTo( "showHoldings" ) == 0 )
		{
			if( cmdArgs.size() == 1 )
				System.out.println( market.showHoldings( trader ) );				
			else if( cmdArgs.size() == 2 )
			{
				String traderName = new String( cmdArgs.stringAt(1) );
				System.out.println( market.showHoldings( traderName ) );
			}
			else throw new InvalidCommandException( 
				new String("Syntax: showHoldings [trader]."));
		}

        else if( (cmdArgs.stringAt(0)).equals( "showAssetProperties" ))
        {
            System.out.println( market.getAssetProperties() );
        }
    } // execIt()

} // class Manual
//--------------------

// class CmdVector: Turn the tokens in a String into elements of a
// Vector.
class CmdVector extends Vector
{
    final String delimiter = new String(" \t\r\n");
    
    CmdVector( String cmdLine )
    {
        super( 2 );

        StringTokenizer tokenizer = new StringTokenizer( cmdLine, delimiter, false );

        while( tokenizer.hasMoreTokens() == true )
        {
            String tok = new String( tokenizer.nextToken() );
            this.addElement( tok );
        }

    }

    String stringAt( int index ) // Avoids annoying typecast
    {
        return (String)elementAt( index );
    }
    int intAt( int index )   
    {
        return Integer.parseInt( stringAt(index) );
    }
    float floatAt( int index )   
    {
        return Float.valueOf( stringAt(index) ).floatValue();
    }
} // class CmdVector
//----------------------

class InvalidCommandException extends Exception {
    InvalidCommandException( String e ) { super( e ); }
}
//--------------------


