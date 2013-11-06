package jqc.client.GUI;

import java.awt.*;
import java.awt.event.*;
import java.rmi.*;

public class LoginFrame extends Frame
{
    private String trader = new String("");
    private String pwd    = new String("");

    private TextField traderField = new TextField();
    private TextField pwdField = new TextField();
	private Button    bOK = new Button();

    LoginFrame()
    {
        super();

 		setTitle( "Berkeley Stock Exchange" );
 		setLayout( new BorderLayout() );
 		setSize(400,200);
 		setBackground(new Color(200,200,255));
 		setResizable(false);
 		addWindowListener(new WindowAdapter() {
 			public void windowClosing(WindowEvent e){ System.exit(0); }
 		});

 		//--------------------
 		// Place the name & pwd entry area in the NW corner.
 		Panel pW = new Panel();
 		pW.setLayout( new BorderLayout() );
 		add( pW,"West" );
 		Panel pNW = new Panel();
 		pNW.setLayout(new GridLayout(4,1));
 		pNW.setFont( G.labelFont );
 		pW.add( pNW,"North" );
 
 		pNW.add( new Label("Name"));
 		traderField = new TextField(9);
		traderField.requestFocus();
 		pNW.add( traderField );
 
 		pNW.add( new Label("Password"));
 		pwdField = new TextField(9);
 		pwdField.setEchoChar('x');
 		pNW.add( pwdField );
 		//--------------------

 		// OK button
 		bOK = new Button("OK");
 		bOK.setBackground( G.darkGreen );
 		bOK.setForeground( Color.white );
 		bOK.setFont( G.labelFont );
 		bOK.addActionListener(new ButtonActionListener(this));
 
 		Panel pS = new Panel();
 		pS.add( bOK );
 		add( pS, "South" );
 		show();
		//---------------------

    } // LoginFrame ctor
//----------------------------------

// Inner class...(needs to use private fields of class LoginFrame.
class ButtonActionListener implements ActionListener
{

	private LoginFrame lf;

	ButtonActionListener( LoginFrame lf )
	{
		super();
		this.lf = lf;
	}

    public void actionPerformed( ActionEvent e ) 
    {
	  try
	  {
		boolean correctLogin = 	G.remoteMarket.signIn( lf.traderField.getText(), 
									                   lf.pwdField.getText());
		if( correctLogin == true ) 
		{
			TopMenuFrame top = new TopMenuFrame( lf.traderField.getText() );
    	    lf.dispose();
		}
		else
		{
			// Flashing red button.
            Button nogo = new Button( "Intruder alert" );
            nogo.setFont( G.labelFont );
            nogo.setBackground( Color.red );
			nogo.setForeground( Color.white );
            lf.add(nogo);
            lf.show();
	
			for( int i=0;i<8;i++ )
			{
	            try { java.lang.Thread.sleep(400); }
    	        catch (InterruptedException ie) { System.err.println( ie );}
				nogo.setBackground( Color.white );
				nogo.setForeground( Color.red );
				nogo.repaint();
	
	            try { java.lang.Thread.sleep(200); }
    	        catch (InterruptedException ie) { System.err.println( ie );}
				nogo.setBackground( Color.red );
				nogo.setForeground( Color.white );
				nogo.repaint();
			}
	
			lf.remove(nogo);
			lf.show();
        }
	  }
	  catch (RemoteException re ) { System.err.println(re); }

    } // ButtonActionListener.actionPerformed()
    //--------------------
}
} // class LoginFrame


