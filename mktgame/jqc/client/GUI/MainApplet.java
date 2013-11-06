package jqc.client.GUI;

import java.rmi.*;
import java.applet.*;

public class Main extends Applet
{
    private static String url;

//	public static void main(String[] args)
    public void
	{
        System.setSecurityManager(new RMISecurityManager());

        Main main = new Main();
        main.connectToServer();

		LoginFrame login = new LoginFrame();
	}

    private static void connectToServer()
    {
        try 
        {
            url    = System.getProperty("market", "rmi:///BerkeleyStockExchange");
            G.remoteMarket = (jqc.client.RemoteMarket) Naming.lookup(url);
        }
        catch (RemoteException e) 
		{ 
			System.err.println(e); 
			System.exit(1);
		}
        catch (Exception e) 
		{
            System.err.println(e);
			System.exit(1);
        }
    } // Main.connectToServer()
	//---------------------------

} // class Main
//----------------------------------
