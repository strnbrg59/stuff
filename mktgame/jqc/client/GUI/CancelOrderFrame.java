package jqc.client.GUI;

import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.util.*;

public class CancelOrderFrame extends Frame
{
	private String trader;
	private String assetName;
    private Color specialAssetColor;
    private java.awt.List orderList;
    private Panel pN;

	CancelOrderFrame( String trader, String assetName, Color specialAssetColor )
	{
		this.trader = trader;
		this.assetName = assetName;
        this.specialAssetColor = specialAssetColor;
		
		setBackground( specialAssetColor );
		setSize(400,200);
        setLayout( new GridLayout(2,1) );

		setTitle( assetName + ": cancel order" );
		setFont( G.labelFont );
 		addWindowListener(new WindowAdapter() {
 			public void windowClosing(WindowEvent e){ dispose(); }
 		});

        displayList();
    }

    void displayList()
    {
        // Display a clickable list of outstanding orders.
		Vector ownOrders = getOwnOrders( trader );
		orderList = new java.awt.List( ownOrders.size(), true );
		orderList.setFont( G.labelFont );
        orderList.setBackground( Color.white );
		for( int i=0;i<ownOrders.size();i++ )
			orderList.addItem( (String)(ownOrders.elementAt(i) ));
        OrderItemListener oil = new OrderItemListener(this);
        orderList.addItemListener( oil );
        add(orderList);

        pN = new Panel();
        pN.setBackground( specialAssetColor );
		add( pN );

		show();
	}
    void clearList()
    {
        remove(pN);
        remove(orderList);
    }

	Vector getOwnOrders( String trader ) 
	{
		String allOrders = new String("");
		try{ allOrders = new String( G.remoteMarket.showOrderBook(assetName) ); }
		catch (RemoteException re ) { System.err.println(re); }

        StringTokenizer tokenizer = new StringTokenizer( allOrders, "\n", false );
		Vector result = new Vector();

		while( tokenizer.hasMoreTokens() == true )
		{
			// Grab just trader's own.
			// Trader's name is either the first (bids) or last (asks) token
			// on the line.
			String nextOrder = new String(tokenizer.nextToken());
			StringTokenizer lineTokenizer = new StringTokenizer( nextOrder );
			
			String possibleTraderName = lineTokenizer.nextToken();
			if( possibleTraderName.compareTo( trader ) == 0 )
				result.addElement( nextOrder );
			else
			{
				while( lineTokenizer.hasMoreTokens() == true )
					possibleTraderName = lineTokenizer.nextToken();
				if( possibleTraderName.compareTo( trader ) == 0 )
					result.addElement( nextOrder );
			}

		}

		return result;
	}

class OrderItemListener implements ItemListener
{
    CancelOrderFrame cof;

    OrderItemListener( CancelOrderFrame cof ) 
    { 
        super(); 
        this.cof = cof;
    }

    public void itemStateChanged( ItemEvent e )
    {
		String choice = ((java.awt.List)e.getItemSelectable()).getSelectedItem();

        // Obtain side, price and units.  String choice looks just like
        // an entry in the order book -- trader-units-price if it's a bid,
        // price-units-trader if it's an ask.
        String side = new String("");
        float price;
        int units;
        
        StringTokenizer orderTokenizer = new StringTokenizer(choice);
        String tok1 = new String( orderTokenizer.nextToken() );
        if( tok1.compareTo( cof.trader ) == 0 )
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

        try
        {
            G.remoteMarket.cancelOrder( cof.trader, side, price, units, cof.assetName );
            cof.clearList();
            cof.displayList();
        }
        catch (RemoteException re ) { System.err.println( re ); }
    }
}

}
