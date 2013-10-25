all : ngrams html install

WEB_ROOT=/var/www

ngrams : ngrams.o
	g++ -O3 -L$(HOME)/usr/local/lib -o ngrams ngrams.o ../../lib/libutils.a
# Static linking makes it easier for ngrams to run as a cgi program.

ngrams.o : ngrams.cpp
	g++ -c -O3 -I$(HOME)/usr/local/include ngrams.cpp

html : buildHtml.sh
	./buildHtml.sh > cgi_interaction.html

copy_training_texts :
	mkdir -p $(WEB_ROOT)/ngrams
	mkdir -p $(WEB_ROOT)/cgi-bin/ngrams
	cp training-texts/*.txt $(WEB_ROOT)/ngrams

install : copy_training_texts
	cp ngrams $(WEB_ROOT)/cgi-bin/ngrams
	cp *.html $(WEB_ROOT)/ngrams

clean :
	rm -f ngrams *.o *~ cgi_interaction.html
