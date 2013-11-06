package jqc.client;

import java.rmi.*;
import java.util.Properties;
import jqc.client.GUI.OrdBookDisplay;

public interface RemoteMarket extends Remote {

    public Properties getAssetProperties() throws RemoteException;

    public boolean signIn( String trader, String pwd )
       throws RemoteException;

    public String placeOrder( String trader, String side, 
        float price, int units, String asset ) throws RemoteException;

	public String cancelOrder( String trader, String side,
		float price, int units, String asset ) throws RemoteException;

	public String exerciseOptions( String trader, int units, String asset )
		throws RemoteException;

    public String showOrderBook( String assetName ) throws RemoteException;
	public jqc.server.Order[]  showInsideQuotes( String assetName ) 
		throws RemoteException;

	public String showHoldings( String trader ) throws RemoteException;
	public String showTicker( String assetName ) throws RemoteException;

    public void registerClient  ( RemoteClient rc ) throws RemoteException;
    public void unregisterClient( RemoteClient rc ) throws RemoteException;
}


