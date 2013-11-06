
# Version.  Edit also:
# Dimensioner.java and install.htm
VERSION = 4.20

# Choices are intranet or extranet:
NET = intranet

#JAVA=/usr/local/jdk1.3
JAVA=/usr
#JAVA=/usr/local/jdk1.2.2

%.class : %.java
	$(JAVA)/bin/javac $<

# Place to put the .jar file for use by a browser.
CGIBIN=$(HOME)/public_html/cgi-bin/dimensioner
INTRANET=$(HOME)/public_html/dimensioner

all : standalone_for_website cgi

standalone_for_website : jar
	cp -f dimensioner-$(VERSION).zip $(INTRANET)/software
	cp -f install.htm $(INTRANET)/software

jar : standalone
	find . -name "*.class" | xargs jar cvf Dimens.jar
	echo "Be sure you actually extract the contents of this zip file" | zip dimensioner-$(VERSION).zip Dimens.jar dimens.bat install.htm manual.htm release-notes.txt -z

standalone : GrammarDatabase.java CGIForm.java LicenseDialog.java
	$(JAVA)/bin/javac *.java

LicenseDialog.java : LicenseDialog.java-template
	sed "s/private long m_build_date =/private long m_build_date = `./now.sh`;/" LicenseDialog.java-template > LicenseDialog.java

javadoc :
	mkdir javadoc
	$(JAVA)/bin/javadoc -d javadoc *.java
	cp -f javadoc-images.tar javadoc; cd javadoc; tar xvf javadoc-images.tar
	cp -rf javadoc $(INTRANET)

GrammarDatabase.java : GrammarDatabase.java-template grammars.dat buildGD.sh
	./buildGD.sh

cgi : jar pkc
	cp -f Dimens.jar $(CGIBIN)
	cp -f dimensioner-cgi.sh $(CGIBIN)
	cp -f callmodel-collect-grammars.html $(INTRANET)
	cp -f callmodel-collect-grammars.tcl $(CGIBIN)
	cp -f callmodel-find-counts.sh $(CGIBIN)

CGIForm.java : CGIForm.java-template cgiForm-$(NET).html
	./buildCGIForm-java.sh $(NET)

# Public Key Cryptology stuff
pkc : cgi
	cp -f modulus $(CGIBIN)
	cp -f exponent $(CGIBIN)
	cp -f public_exponent $(CGIBIN)
	cp -f dimensioner-license.sh $(CGIBIN)
	cp -f dimensioner-license.html $(INTRANET)

clean :
	rm -f GrammarDatabase.java LicenseDialog.java CGIForm.java Dimens.jar
	rm `find . -name "*.class"` `find . -name "*~"`

applet : jar
	cp -f Dimens.jar $(WEBSITE)
	cp -f dimens.htm $(WEBSITE)
