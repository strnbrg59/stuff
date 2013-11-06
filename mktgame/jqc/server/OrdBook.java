package jqc.server;

import java.io.*;
import java.util.*;

class OrdBook implements Serializable {

    static final long serialVersionUID = 314159L;

	private String assetName = new String("");
    BidOrdStack bid;
    AskOrdStack ask;
    static final int k_maxDepth = 10;

    OrdBook( String assetName ) {
		this.assetName = assetName;
        bid = new BidOrdStack();
        ask = new AskOrdStack();
    }

    String insert( BidOrder b ) {
		String confirmation = new String("");

        if( (ask.head != null ) && (b.price >= ask.head.price) )
            confirmation = clear( b );
        else
            confirmation = bid.insert(b);

		return confirmation;
    }

    String insert( AskOrder a ) 
	{
		String confirmation = new String("");

        if( (bid.head != null ) && (a.price <= bid.head.price) )
            confirmation = clear( a );
        else
            confirmation = ask.insert(a);

		return confirmation;
    }

	String cancel( Order ord )
	{
		String confirmation = new String("");

		if((bid.head != null) && (ord.price <= bid.head.price))
			confirmation = bid.cancel(ord);
		if((ask.head != null) && (ord.price >= ask.head.price))
			confirmation = ask.cancel(ord);

		return confirmation;
	}
    //---------------------

    //---------------------
    // clear(BidOrder) and clear(AskOrder).  These would be great
    // candidates for a C++ template, sigh.
    //
    String clear( BidOrder b ) {
    // recursive
		String confirmation = new String("");
        int unitsSold;

        if( (ask.head != null) && (b.price >= ask.head.price) )
        {
            if( b.units < ask.head.units )
            {
                unitsSold = b.units;
                confirmation = Trade.genConfirmation( b.trader, ask.head.trader, assetName,
								                      unitsSold, ask.head.price );
                ask.head.units -= unitsSold;
            }
            else
            {
                unitsSold = ask.head.units;
                confirmation = Trade.genConfirmation( b.trader, ask.head.trader, assetName, 
                    								  unitsSold, ask.head.price );
                b.units -= unitsSold;
                ask.head = ask.head.next;

                if( b.units > 0 )
                    confirmation += "\n" + clear( b );
            }
        }
        else
            confirmation = bid.insert(b);

		return confirmation;
    } // OrdBook.clear(BidOrder)
    //---------------------

    String clear( AskOrder a ) {
    // recursive
		String confirmation = new String("");
        int unitsSold;        

        if( (bid.head != null) && (a.price <= bid.head.price) )
        {
            if( a.units < bid.head.units )
            {
                unitsSold = a.units;
                confirmation = Trade.genConfirmation( bid.head.trader, a.trader, assetName,
													  unitsSold, bid.head.price );

                bid.head.units -= unitsSold;
            }
            else
            {
                unitsSold = bid.head.units;
                confirmation = Trade.genConfirmation( bid.head.trader, a.trader, assetName,
													  unitsSold, bid.head.price );

                a.units -= unitsSold;
                bid.head = bid.head.next;

                if( a.units > 0 )
                    confirmation += "\n" + clear( a );
            }

        }
        else
            confirmation = ask.insert(a);

		return confirmation;
    } // OrdBook.clear(AskOrder)
    //-----------------------------

    public String toString() {
        String rep;
        rep  = ask.toString();
        rep += bid.toString();
        return rep;
    }
    //-----------------------------

	// dump(): Serialize and save OrdBook in data/ordbook.<assetName>.
	void dump()
	{
		try
		{
			ObjectOutputStream out = new ObjectOutputStream( new FileOutputStream
				( "data/ordbook." + assetName ));
			out.writeObject( this );
			out.flush();
			out.close();
		}
		catch (IOException e) 
		{ 
			System.err.println( e ); 
			e.printStackTrace();
		}
	} // dump()
	//-----------------------------

    // loadOrdBooks().  OrdBooks in memory are kept in a Hashtable.
	// On disk, they're in data/ordbook.<assetName>. 
    static Hashtable loadOrdbooks( Properties assetProperties )
    {
        Hashtable result = new Hashtable();
        Enumeration assetNames = assetProperties.keys();

        for( int i=0;i<assetProperties.size();i++ )
        {
            String key = new String( (String)(assetNames.nextElement()));;

			// Load from disk, if exists.  Otherwise create empty OrdBook.
			OrdBook ob = new OrdBook(key);
			String filename = new String("data/ordbook." + key);
			File obf = new File( filename );
			if( obf.exists() ) try
			{
				ObjectInputStream in = new ObjectInputStream( new FileInputStream
					(filename));
				ob = (OrdBook)in.readObject();
			} 
			catch (IOException e) 
			{
				System.err.println(e);
				e.printStackTrace();
			}
			catch (ClassNotFoundException cnfe) 
			{
				System.err.println(cnfe);
				cnfe.printStackTrace();
			}
            result.put( key, ob );
        }
        
        return result;
    }

} // class OrdBook
//-------------------------------

abstract class OrdStack implements Serializable {

    static final long serialVersionUID = 314160L;

    OrdStack() {
        head = null;
    }

	String cancel( Order ord )
	{
		// Removes all orders matching ord's trader and price exactly.  We won't
		// be a stickler on the units, because these could change while the
		// user is forming this command (as the result of a trade).
		String confirmation = new String("Nothing matched.");
		int i=0; // Number of orders cancelled.
		Order currLink = head;
		Order prevLink = null;
		while( currLink != null )
		{
			if( ( currLink.trader.compareTo( ord.trader ) == 0 )
			&&  ( currLink.price == ord.price ))
			{
				if( currLink == head )
					head = currLink.next;
				else
					prevLink.next = currLink.next;

				confirmation = new String("Cancelled " + ++i );
			}
			else
				prevLink = currLink;

			currLink = currLink.next;
		}
		
		return confirmation;
	} // cancel()

    String insert( Order newOrd ) {
    // Keep order stack sorted, using subclass' sortOrder() methods.
    // Limit the size of the order book to OrdBook.k_maxDepth.

		// confirmation message: gets changed if order is trimmed off
		// the stack, for noncompetive price.
		String confirmation = new String("Inserted into order book.");

        if( head == null )
            head = newOrd;
        else 
        {
            Order prevLink = null;
            Order currLink = head;

            while( ( currLink != null ) 
            &&     ( sortOrder(currLink.price, newOrd.price) ))
            {
                prevLink = currLink;
                currLink = currLink.next;
            }
    
            if( prevLink != null )
            {
                prevLink.next = newOrd;
                newOrd.next = currLink;            
            }
            else
            {
                if( sortOrder(head.price, newOrd.price) )
                    head.next = newOrd;
                else
                {
                    newOrd.next = head;
                    head = newOrd;
                }
            }
        }

        // Trim away anything beyond OrdBook.k_maxDepth.
        Order currLink = head;
        int atDepth = 0;
        while( currLink != null ) {
            if( ++atDepth > OrdBook.k_maxDepth )
			{
				if( currLink.next == newOrd )
					confirmation = new String("Rejected, price not competitive.");
                currLink.next = null;
			}
            currLink = currLink.next;
        }

		return confirmation;
    } // OrdStack.insert()

    public abstract String toString();
    static final int k_maxDisp = 10; // Max orders to display.

    abstract boolean sortOrder( float x, float y );

    Order head;
} // class OrdStack
//---------------------------

class BidOrdStack extends OrdStack implements Serializable {
    static final long serialVersionUID = 314161L;

    boolean sortOrder( float x, float y ) {
        if( x >= y )
            return true;
        else
            return false;
    }

    public String toString() {
        String rep = "";

        Order currlink = head;
        int i=0;
        while( (currlink != null) && ( i++ < k_maxDisp ) ) {
            rep += currlink.toString() + "\n";
            currlink = currlink.next;
        }

        return rep;
    }

} // class BidOrdStack
//---------------------------

class AskOrdStack extends OrdStack implements Serializable {
    static final long serialVersionUID = 314162L;

    boolean sortOrder( float x, float y ) {
        if( x <= y )
            return true;
        else
            return false;
    }

    public String toString() {
    // Display with head at the bottom.  So we have to first accumulate
    // in an array.

        String[] ordArray = new String[k_maxDisp];

        Order currlink = head;
        int i=0;
        while( currlink != null && i < k_maxDisp ) {
            ordArray[i] = Order.askOrdBlanks + currlink.toString();
            currlink = currlink.next;

            i++;
        }

        // Go through array backwards, building up String to display.

        String rep = "";
        for( int j=i-1; j>=0; j-- )
            rep += ordArray[j] + '\n';

        return rep;
    }
} // class AskOrdStack










