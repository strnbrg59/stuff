package jqc.server;

import java.io.*;
import java.text.NumberFormat;
import java.rmi.*;

abstract public class Order implements Serializable {
    static final long serialVersionUID = 314163L;

    //------------------------
    // Data members

    // Instance
    String trader;
    float price;
	public float price() { return price; }
    int    units;
	public int units() { return units; }
    Order  next;

    // Class
    static NumberFormat priceFormat;
    static NumberFormat unitsFormat;

    static final int k_nameLen;
    static final int k_maxUnitsDigits;
    static final int k_maxPriceChars;

    static StringBuffer askOrdBlanks;

    static {
        k_maxUnitsDigits=2;
        k_maxPriceChars=5; // <=100, with precision to 0.1
        k_nameLen=8;
        askOrdBlanks = new StringBuffer( k_nameLen + 4 );
        for( int i=0;i<k_nameLen+4;i++ )
            askOrdBlanks.append(' ');

        priceFormat = NumberFormat.getInstance();
        priceFormat.setMaximumFractionDigits( 1 );
        priceFormat.setMinimumFractionDigits( 1 );

    } // static initializer
    //------------------

    // Constructors
    Order( String trader, float price, int units ) {
        this.trader = trader;
        this.price  = ((float)((int)(10.0*price + 0.5)))/(float)10.0;
        this.units  = units;

        this.next = null;
    } // Order.Order

	Order( Order o ) { this( o.trader, o.price, o.units ); }

    //---------------------
    // Display
    public abstract String toString();

    StringBuffer formatUnits( int units )
    { // Pad with blanks in front.
        StringBuffer result = new StringBuffer();
        String unitsRep = new Integer(units).toString();
        
        for( int i=0;i<k_maxUnitsDigits - unitsRep.length();i++ )
            result.append(' ');
        result.append( unitsRep );

        return result;
    } // Order.formatUnits

    StringBuffer formatPrice( float price )
    { // Pad with blanks in front.
        StringBuffer result = new StringBuffer();
        String priceRep = new String( priceFormat.format(price+0.000001) );
		// That .000001 is a kludge; otherwise 3.2, which for some reason
		// is often represented internally as 3.1999999, gets printed as
		// 3.1.
        
        for( int i=0;i<k_maxPriceChars - priceRep.length();i++ )
            result.append(' ');
        result.append( priceRep );

        return result;
    } // Order.formatPrice

    StringBuffer formatTrader( String trader )
    { // Format trader's name to some constant number of chars.

        StringBuffer result = new StringBuffer();
        int i=0;
        while( i<trader.length() && i<k_nameLen ) 
        {
            result.append(trader.charAt(i));
            i++;
        }
        if( i<k_nameLen )
        {
            for( int j=i;j<k_nameLen;j++ )
                result.append(' ');
        }

        return result;
    } // Order.formatTrader
    //------------------------

    static void sanityCheck( float price, int units ) throws RemoteException
    {
      final int k_maxPrice = 100;
      final int k_maxUnits = 100;

      if( (price <= 0) || (price >= k_maxPrice) ) {
        throw new RemoteException( new String( "Invalid price (should be between "
            + 0 + " and " + k_maxPrice + ")." ) );
      }

      if( (units <= 0) || (units >= k_maxUnits) ) {
        throw new RemoteException( new String( "Invalid units (should be between "
            + 0 + " and " + k_maxUnits + ")." ) );
      }
    } // sanityCheck()

} // class Order
//--------------------------

class BidOrder extends Order implements Serializable {
    static final long serialVersionUID = 314164L;
    
    BidOrder( String trader, float price, int units ) {
        super( trader, price, units );
    }

	BidOrder( Order o ){
		super( o );
	}

    public String toString() {
        String rep = formatTrader(trader) + " "
                   + formatUnits( units ) + " "
                   + formatPrice( price ) + " ";
        
        return rep;
    }
}
//---------------------------

class AskOrder extends Order implements Serializable {
    static final long serialVersionUID = 314165L;
    
    AskOrder( String trader, float price, int units ) {
        super( trader, price, units );
    }

	AskOrder( Order o ) {
		super( o );
	}

    public String toString() 
    {
        String rep = formatPrice( price ) + " "
                   + formatUnits( units ) + " "
                   + formatTrader(trader);

        return rep;
    }
}
