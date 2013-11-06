package jqc.client.GUI;

import java.awt.*;
import java.rmi.*;

public class OrdBookDisplay extends NotifiableTextArea
{

    public OrdBookDisplay( String assetName, int rows, int cols ) 
    {
        super( assetName, rows, cols );
    }

    void update()
    {
        final String assetName = new String(OID);

        String ordBookString = new String("");
        try { ordBookString = G.remoteMarket.showOrderBook(assetName); }
        catch (RemoteException e) { ordBookString = "RemoteException"; }

        setText( ordBookString );
        setBackground( G.TextAreaColor );
        repaint();
    }

    /** update(String) is abstract in superclass.  It's only called on TickerDisplay.
    **/
    void update( String confirmation )
    {
        System.err.println( "OrdBookDisplay.update(String) called: shouldn\'t happen!" );
    }

}
