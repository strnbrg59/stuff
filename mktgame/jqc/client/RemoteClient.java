// Interface for server-to-client callback.

package jqc.client;

import java.rmi.*;

public interface RemoteClient extends Remote {
    void notification() throws RemoteException;
    void notification( String confirmation ) throws RemoteException;
    String getOID() throws RemoteException;
}
