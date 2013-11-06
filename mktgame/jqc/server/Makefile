AUXCLASSES = OrdBook.class Order.class Passwords.class AssetProperties.class G.class Accounts.class Transfer.class
CLIENT_INTERFACE = ../client/RemoteMarket
JFLAGS = 

Server : Server.class
	rmic jqc.server.Server

Server.class : Server.java $(AUXCLASSES) $(CLIENT_INTERFACE).class
	javac $(JFLAGS) Server.java

Order.class : Order.java
	javac $(JFLAGS) Order.java
OrdBook.class : OrdBook.java Order.class
	javac $(JFLAGS) OrdBook.java
Passwords.class : Passwords.java Order.class
	javac $(JFLAGS) Passwords.java
G.class : G.java 
	javac $(JFLAGS) G.java
Trade.class : Trade.java 
	javac $(JFLAGS) Trade.java
Accounts.class : Accounts.java 
	javac $(JFLAGS) Accounts.java
Transfer.class : Transfer.java 
	javac $(JFLAGS) Transfer.java
AssetProperties.class : AssetProperties.java Order.class
	javac $(JFLAGS) AssetProperties.java
$(CLIENT_INTERFACE).class : $(CLIENT_INTERFACE).java
	javac $(JFLAGS) $(CLIENT_INTERFACE).java

clean :
	rm *.class *~ 
