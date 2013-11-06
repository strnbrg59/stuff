CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.foosrater

DASH_I = -I$(includedir) -I$(HOME)/usr/include

AM_CPPFLAGS = $(KPPFLAGS) $(DASH_I) -DDATADIR=\"$(pkgdatadir)/data/\"
AM_CXXFLAGS =
AM_CFLAGS =
CPPFLAGS =
CXXFLAGS =
CFLAGS =

AM_LDFLAGS=-L$(libdir) -L$(HOME)/usr/lib -lutils -lgsl -lgslcblas

cmdline.cpp : cmdline.hpp
cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = foosrater.h

foosrater_LTLIBRARIES = libfoosrater.la
libfoosrater_la_SOURCES = foosrater.cpp cmdline.cpp analysis.cpp display.cpp
foosraterdir = $(pkglibdir)

bin_PROGRAMS = foosrater simulate_games
foosrater_SOURCES = foosrater-main.cpp
foosrater_LDADD = libfoosrater.la
simulate_games_SOURCES = simulate_games.cpp
simulate_games_LDADD = libfoosrater.la

EXTRA_DIST = $(CMDLINE_VAR_INFO_FILE)
MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp

# Upload and untar in /opt/tms/web2/html on a CMC.
websitedist : $(bin_PROGRAMS)
	./foosrater history_length=100 > numbers.html
	cat numbers.html rodgraph.html ../web/theory.html > ../web/index.html
	echo "<html><pre>" >> ../web/index.html
	cat realgames.txt >> ../web/index.html
	echo "</pre></html>" >> ../web/index.html
	convert -rotate 90 history.ps history.gif
	cp history.gif ../web
	cd ../web; cp -R * /u/tsternberg/public_html/foos
