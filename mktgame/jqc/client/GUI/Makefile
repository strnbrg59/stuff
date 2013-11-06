JFLAGS = 

Main : Main.class NotifiableTextArea.class TickerDisplay.class AccountDisplay.class AssetMenuFrame.class OrdBookDisplay.class NotifiableTextArea_Stub.class PlaceOrderFrame.class TopMenuFrame.class LoginFrame.class CancelOrderFrame.class G.class ExerciseOptionsFrame.class

clean :
	rm *~ *.class

Main.class : Main.java
	javac $(JFLAGS) Main.java

NotifiableTextArea.class : NotifiableTextArea.java
	javac $(JFLAGS) NotifiableTextArea.java

NotifiableTextArea_Stub.class : NotifiableTextArea.class
	rmic jqc.client.GUI.NotifiableTextArea

OrdBookDisplay.class : OrdBookDisplay.java
	javac $(JFLAGS) OrdBookDisplay.java

AccountDisplay.class : AccountDisplay.java
	javac $(JFLAGS) AccountDisplay.java

TickerDisplay.class : TickerDisplay.java
	javac $(JFLAGS) TickerDisplay.java

PlaceOrderFrame.class : PlaceOrderFrame.java
	javac $(JFLAGS) PlaceOrderFrame.java

CancelOrderFrame.class : CancelOrderFrame.java
	javac $(JFLAGS) CancelOrderFrame.java

ExerciseOptionsFrame.class : ExerciseOptionsFrame.java
	javac $(JFLAGS) ExerciseOptionsFrame.java

TopMenuFrame.class : TopMenuFrame.java
	javac $(JFLAGS) TopMenuFrame.java

LoginFrame.class : LoginFrame.java
	javac $(JFLAGS) LoginFrame.java

AssetMenuFrame.class : AssetMenuFrame.java
	javac $(JFLAGS) AssetMenuFrame.java

G.class : G.java
	javac $(JFLAGS) G.java
