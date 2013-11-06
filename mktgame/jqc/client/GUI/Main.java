package jqc.client.GUI;

import java.rmi.*;

public class Main
{
    private static String url;

	public static void main(String[] args)
	{
        //System.setSecurityManager(new RMISecurityManager());

        Main main = new Main();
        main.connectToServer();

		LoginFrame login = new LoginFrame();
	}

    private static void connectToServer()
    {
        try 
        {
            url    = System.getProperty("marketname", "rmi://hepburn.lbl.gov:19594/BerkeleyStockExchange");
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
