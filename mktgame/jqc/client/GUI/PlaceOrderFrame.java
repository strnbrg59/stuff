package jqc.client.GUI;

import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.rmi.server.*;

public class PlaceOrderFrame extends Frame
{
    private TextField priceBox;
    private TextField unitsBox;
    private String trader;
    private String assetName;
    private Color assetColor;
    private CheckboxGroup cbg;
    private Panel pN, pS;

    PlaceOrderFrame( String trader, String assetName, Color assetColor )
    {
        super();
        this.trader = trader;
        this.assetName = assetName;
        this.assetColor = assetColor;

        // Properties of *this.
        setTitle( assetName + ": new order" );
        setBackground( assetColor );
        setSize(400,150);
        setLayout( new BorderLayout());
        setFont( G.labelFont );
 		addWindowListener(new WindowAdapter() {
 			public void windowClosing(WindowEvent e){ dispose(); }
 		});

        // North panel, for bid/ask and price/units.
        pN = new Panel( new BorderLayout() );
        add(pN,"North");

        // bid/ask checkbox, in NW corner.
        cbg = new CheckboxGroup();
        Checkbox bidBox = new Checkbox( "bid", true, cbg );
        Checkbox askBox = new Checkbox( "ask", false, cbg );
        Panel cp = new Panel();
        cp.setLayout( new GridLayout(1,2,2,2));
        cp.setBackground( Color.pink );
        cp.add(bidBox);
        cp.add(askBox);
        pN.add(cp,"West");

        // price and units TextFields.
        Panel pup = new Panel();
        pup.setLayout( new GridLayout(2,1,2,2));
        pup.setBackground(Color.white);
        pup.add( new Label("price"));
        priceBox = new TextField(5);
        pup.add(priceBox);
        pup.add( new Label("units"));
        unitsBox = new TextField(5);
        pup.add(unitsBox);
        pN.add(pup,"East");

        // South panel, for Submit button.
        pS = new Panel( new BorderLayout() );
        add(pS,"South");

        // Submit button
 		Button bSubmit = new Button("Submit");
 		bSubmit.setBackground( G.darkGreen );
 		bSubmit.setForeground( Color.white );
 		bSubmit.setFont( G.labelFont );
		OrderActionListener oal = new OrderActionListener( this );
 		bSubmit.addActionListener( oal );
        pS.add( bSubmit, "Center" );

        show();
        
    } // PlaceOrderFrame ctor
//-----------------------

// Respond to "Submit" button; send order to server, print server's confirmation.
// Inner class.
class OrderActionListener implements ActionListener
{
	PlaceOrderFrame pof;

	OrderActionListener( PlaceOrderFrame pof )
	{
		this.pof = pof;
	}

	public void actionPerformed(ActionEvent e) {
	    String side = pof.cbg.getSelectedCheckbox().getLabel();
        float price = Float.valueOf(pof.priceBox.getText()).floatValue();
        int units = Integer.parseInt(pof.unitsBox.getText());

        try
        { 
    	    String confirmation = new String( G.remoteMarket.placeOrder( 
                pof.trader, side, price, units, assetName ));
            pof.show();
         }
         catch (RemoteException rme) {}
	}
} // class OrderActionListener

} // class PlaceOrderFrame
