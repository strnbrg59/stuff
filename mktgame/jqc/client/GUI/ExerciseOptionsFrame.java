package jqc.client.GUI;

import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.rmi.server.*;

public class ExerciseOptionsFrame extends Frame
{
    private TextField unitsBox;
    private String trader;
    private String assetName;
    private Color assetColor;
    private Panel pN, pS;

    ExerciseOptionsFrame( String trader, String assetName, Color assetColor )
    {
        super();
        this.trader = trader;
        this.assetName = assetName;
        this.assetColor = assetColor;

        // Properties of *this.
        setTitle( "exercise " + assetName );
        setBackground( assetColor );
        setSize(200,150);
        setLayout( new BorderLayout());
        setFont( G.labelFont );
 		addWindowListener(new WindowAdapter() {
 			public void windowClosing(WindowEvent e){ dispose(); }
 		});

        // North panel, for "units to exercise:" caption, and units textfield.
        pN = new Panel();
        pN.setBackground(Color.white);
        unitsBox = new TextField(5);
        pN.add(unitsBox);
        pN.add( new Label("units to exercise"));
        add(pN,"North");

        // South panel, for Submit button.
        pS = new Panel( new BorderLayout() );
        add(pS,"South");

        // Submit button
 		Button bSubmit = new Button("Submit");
 		bSubmit.setBackground( G.darkGreen );
 		bSubmit.setForeground( Color.white );
 		bSubmit.setFont( G.labelFont );
		ExerciseActionListener oal = new ExerciseActionListener( this );
 		bSubmit.addActionListener( oal );
        pS.add( bSubmit, "Center" );

        show();
        
    } // ExerciseOptionsFrame ctor
//-----------------------

// Respond to "Submit" button; instruct server to exercise some number of options.
// Inner class.
class ExerciseActionListener implements ActionListener
{
	ExerciseOptionsFrame eof;

	ExerciseActionListener( ExerciseOptionsFrame eof )
	{
		this.eof = eof;
	}

	public void actionPerformed(ActionEvent e) 
	{
        int units = Integer.parseInt(eof.unitsBox.getText());

		try
		{
			String confirmation = 
			G.remoteMarket.exerciseOptions( eof.trader, units, eof.assetName );
		}
		catch( RemoteException re )
		{
			System.err.println( "RemoteException pushing exercise-options button.");
		}
	}
} // class ExerciseActionListener

} // class ExerciseOptionsFrame
