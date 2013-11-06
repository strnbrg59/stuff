package jqc.client.GUI;

import java.awt.*;
import java.rmi.*;

public class AccountDisplay extends NotifiableTextArea
{
    public AccountDisplay( String trader, int rows, int cols ) 
    {
        super( trader, rows, cols );
    }

    void update()
    {
        final String trader = new String(OID);

        String accountString = new String("");
        try { accountString = G.remoteMarket.showHoldings( trader ); }
        catch (RemoteException e) { accountString = "RemoteException"; }

        setText( accountString );
        setBackground( G.TextAreaColor );
        repaint();
    }

    /** update(String) is abstract in superclass.  It's only called on TickerDisplay.
    **/
    void update( String confirmation )
    {
        System.err.println( "OrdBookDisplay.update(String) called: shouldn\'t happen!" );
    }

} // class AccountDisplay
//----------------------------------



