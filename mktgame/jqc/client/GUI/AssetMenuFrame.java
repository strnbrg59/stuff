package jqc.client.GUI;

import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.util.Random;

public class AssetMenuFrame extends Frame
{
    private String assetName = new String("");
    private String trader    = new String("");
    Color  specialAssetColor;
    private Panel ordBookPanel = new Panel();
    private OrdBookDisplay ordBookDisplay;

    public AssetMenuFrame( String assetName, String trader )
    {
        super();
        this.assetName = assetName;
        this.trader    = trader;

		// Color-code all frames pertaining to this asset.
		// Try to choose bright colors.
		Random rand = new Random();
		int[] colors = new int[3];
		colors[0] = colors[1] = colors[2] = 0;
		int rgbChoice = Math.abs(rand.nextInt())%3;
		colors[rgbChoice] = 255 - Math.abs(rand.nextInt())%200;
		rgbChoice = Math.abs(rand.nextInt())%3;
		colors[rgbChoice] = 255 - Math.abs(rand.nextInt())%200;
		specialAssetColor = new Color(colors[0], colors[1], colors[2]);

		// This Frame.
		setTitle( assetName );
		setSize( 600,350 );
		setBackground( specialAssetColor );
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e){ dispose(); }
		});

		//-----------------
		// Buttons
		int nButtons;
		boolean isAnOption = !G.assetProps.getUnderlying(assetName).equals("null");
		if( isAnOption )
			nButtons = 4;
		else
			nButtons = 3;
		Button[] button = new Button[ nButtons ];
		button[0] = new Button("Place order");
		button[1] = new Button("Cancel order");
		button[2] = new Button("Show ticker");
		if( isAnOption )
			button[3] = new Button("Exercise options");

        // place-order button
        button[0].addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                placeOrder();
            }
        });

        // cancel-order button
		CancelButtonListener cbl = new CancelButtonListener(this);
        button[1].addActionListener( cbl );

        // show-ticker button
        TickerButtonListener tbl = new TickerButtonListener(this);
        button[2].addActionListener( tbl );
        
		// exercise-options button
		if( isAnOption )
		{
			ExerciseOptionsButtonListener ebl = new ExerciseOptionsButtonListener(this);
			button[3].addActionListener( ebl );
		}

		Panel pW = new Panel();
        pW.setLayout( new BorderLayout() );
        Panel pNW = new Panel();
		pNW.setLayout( new GridLayout(nButtons,1));

		for( int i=0;i<nButtons;i++ )
		{
			button[i].setBackground( G.buttonColor );
			button[i].setFont( G.labelFont );
			pNW.add( button[i] );
		}
        pW.add(pNW, "North");
		//-----------------

		// Order book window.
        ordBookDisplay = new OrdBookDisplay( assetName, 15,30 );
        ordBookPanel = new Panel();
        ordBookPanel.add( ordBookDisplay );
        add(ordBookPanel, "East");

        add(pW, "West");

		show();

	} // AssetMenuFrame ctor

    // Weird: if you make the following function private, javac hangs while
    // trying to compile this file.  (placeOrder() is called in a button's
    // ActionListener so you can't make placeOrder() private anyway.  But
    // still it's surprising that the error message isn't generated in a
    // reasonable amount of time.)
    void placeOrder()
    {
        PlaceOrderFrame pof 
			= new PlaceOrderFrame( trader, assetName, specialAssetColor );
    }

// Inner class for cancel-order button
class CancelButtonListener implements ActionListener
{
	AssetMenuFrame amf;

	public CancelButtonListener( AssetMenuFrame amf )
	{
		this.amf = amf;
	}

    public void actionPerformed(ActionEvent e) 
	{
		CancelOrderFrame cof = new CancelOrderFrame( amf.trader, amf.assetName,
			amf.specialAssetColor);
	}
}


class ExerciseOptionsButtonListener implements ActionListener
{
	AssetMenuFrame amf;
	
	public ExerciseOptionsButtonListener( AssetMenuFrame amf )
	{		
		this.amf = amf;
	}

	public void actionPerformed( ActionEvent e )
	{
		ExerciseOptionsFrame eof = new ExerciseOptionsFrame( amf.trader,
			amf.assetName, amf.specialAssetColor );
	}
}

class TickerButtonListener implements ActionListener
{
	AssetMenuFrame amf;

	public TickerButtonListener( AssetMenuFrame amf )
	{
		this.amf = amf;
	}

    public void actionPerformed(ActionEvent e) 
	{
		Frame tickerFrame = new Frame();
        tickerFrame.setTitle( assetName + " ticker" );
        tickerFrame.setBackground( amf.specialAssetColor );
		TickerFrameListener tfl = new TickerFrameListener( tickerFrame );
		tickerFrame.addWindowListener( tfl );

        TickerDisplay ticker = new TickerDisplay( amf.assetName, 10,45 );
        ticker.setBackground( amf.specialAssetColor );

        tickerFrame.add( ticker );
        tickerFrame.pack();
        tickerFrame.show();
	}

	// Why can't I extend the WindowAdaptor?!
	class TickerFrameListener implements WindowListener 
	{
		Frame tickerFrame;
		TickerFrameListener( Frame tickerFrame )
		{
			super();
			this.tickerFrame = tickerFrame;
		}

		public void windowClosing( WindowEvent e ) { tickerFrame.dispose(); }
		public void windowOpened( WindowEvent e ) {}
		public void windowClosed( WindowEvent e ) {}
		public void windowIconified( WindowEvent e ) {}
		public void windowDeiconified( WindowEvent e ) {}
		public void windowActivated( WindowEvent e ) {}
		public void windowDeactivated( WindowEvent e ) {}
		
	}

} // class TickerButtonListener

} // class AssetMenuFrame
//----------------------------------
