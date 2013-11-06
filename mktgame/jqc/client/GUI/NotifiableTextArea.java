/** Abstract class for TextAreas that go dark when the server notifies them,
  * and which update themselves when you click in them.
  * Subclassed by classes OrdBookDisplay, AccountDisplay and TickerDisplay.
**/

package jqc.client.GUI;

import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.rmi.server.*;

abstract public class 
NotifiableTextArea extends TextArea implements jqc.client.RemoteClient
{

    public final String OID;
    // OID is set to asset name by OrdBookDisplay, to trader names (buyer+seller)
    // by AccountDisplay.  Server.java looks at it to determine which objects on
    // its RemoteClient Vector to notify.

    public NotifiableTextArea( String OID, int rows, int cols ) 
    {
        super( new String(""), rows, cols );
        this.OID = OID;

        setBackground( G.TextAreaColor );
        setFont( G.labelFont );

        enableEvents(AWTEvent.MOUSE_EVENT_MASK);

        try
        {
            UnicastRemoteObject.exportObject(this);        
            G.remoteMarket.registerClient( this );
        }
        catch (RemoteException e) { 
            System.out.println( "Exception thrown when calling registerNotifiableTextArea()");
            System.err.println(e); return; 
        }

        update();
    }

    public String getOID() { return new String(OID); }

    abstract void update();
    abstract void update( String confirmation );

    public void notification()
    {
        //System.out.println( "Client: Server just sent notification." );
        setBackground( G.TextAreaColor.darker().darker() );
        repaint();
    }

    public void notification( String confirmation )
    {
        update( confirmation );
    }

    // If user clicks inside, then update the display.
    public void processMouseEvent(MouseEvent e)
    {
        if( e.getID() == MouseEvent.MOUSE_PRESSED)
        {
            update();
        }
    }

    protected void finalize()
    {
        try{ G.remoteMarket.unregisterClient( this ); }
        catch (RemoteException e) { System.err.println(e); }
    }

} // class NotifiableTextArea
//----------------------------------
