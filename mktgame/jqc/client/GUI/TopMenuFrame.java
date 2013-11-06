package jqc.client.GUI;

import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.util.*;
import jqc.server.AssetProperties;

public class TopMenuFrame extends Frame
{
    private Panel pW;
    private String trader;

    TopMenuFrame( String trader )
    {
        super();
        this.trader = trader;

        setTitle( "Berkeley Stock Exchange" );
		setSize(400,200);
		setBackground(Color.white);
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e){ System.exit(0); }
		});

		//-----------------
		// List: Choice of assets. (East)
		try
		{ 
			G.assetProps = (AssetProperties)(G.remoteMarket.getAssetProperties());
		}
		catch (RemoteException e ) { System.err.println( e ); }
		java.awt.List assetList = new java.awt.List( G.assetProps.size(),false);
		assetList.setFont( G.labelFont );
        Enumeration keys = G.assetProps.keys();
        while( keys.hasMoreElements() == true )
        {
    		assetList.addItem( (String)(keys.nextElement()) );
	    }

		AssetItemListener il = new AssetItemListener(this);
		assetList.addItemListener( il );

		add( assetList, "East" );
		//-----------------

		//-----------------
        // West panel -- for buttons (NW), and holdings (SW).
		pW = new Panel();
        pW.setLayout( new BorderLayout() );

		//-----------------
		// Quit Button (North-West)
        Panel pNW = new Panel();
		Button quitButton = new Button("Quit");
        quitButton.setBackground( Color.red );
		quitButton.setForeground( Color.white );
		quitButton.setFont( G.labelFont );
        quitButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                System.exit(0);
            }
        });
        pNW.add(quitButton);
        pW.add( pNW, "North" );

        //----------------
        // Holdings window (South-West)
        AccountDisplay accountDisplay = new AccountDisplay( trader, 5,22 );
        Panel pSW = new Panel();
        pSW.add( accountDisplay );
        pW.add( pSW, "South" );
		//-----------------

		add(pW,"West");		
		show();
    }

    void respondToAssetChoice( String assetName )
    {
	    AssetMenuFrame amf = new AssetMenuFrame( assetName, trader );
    }
} // class TopMenuFrame
//----------------------------------

class AssetItemListener implements ItemListener
{
	TopMenuFrame tmf;

	AssetItemListener( TopMenuFrame tmf )
	{
		this.tmf = tmf;
	}

	public void itemStateChanged(ItemEvent e) {
		String choice = ((java.awt.List)e.getItemSelectable()).getSelectedItem();
		tmf.respondToAssetChoice( choice );
	}
} // class AssetItemListener
