ALLCLIENTS = Manual Poisson Populator MktMaker Gui 
JFLAGS = 

all : $(ALLCLIENTS)
clean :
	rm *.class *~

####
Manual : Manual.class RemoteMarket.class RemoteClient.class
Poisson : Poisson.class RemoteMarket.class
Populator : Populator.class
MktMaker : MktMaker.class
Gui : 
	cd GUI; make Main
####

####
Manual.class : Manual.java RemoteMarket.class
	javac $(JFLAGS) Manual.java
	rmic jqc.client.Manual

Poisson.class : Poisson.java RemoteMarket.class
	javac $(JFLAGS) Poisson.java

MktMaker.class : MktMaker.java RemoteMarket.class
	javac $(JFLAGS) MktMaker.java
	rmic jqc.client.MktMaker

Populator.class : Populator.java RemoteMarket.class
	javac $(JFLAGS) Populator.java

RemoteMarket.class : RemoteMarket.java
	javac $(JFLAGS) RemoteMarket.java
####



