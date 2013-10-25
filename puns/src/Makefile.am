CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.punsrc

DASH_I = -I$(includedir)

AM_CPPFLAGS = $(KPPFLAGS) $(DASH_I) -DDATADIR=\"$(pkgdatadir)/data/\"
AM_CXXFLAGS =
AM_CFLAGS =
CPPFLAGS =
CXXFLAGS =
CFLAGS =

AM_LDFLAGS=-L$(libdir) -lutils

cmdline.cpp : cmdline.hpp
cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = punserver.hpp phonics.hpp punchlines.hpp cgi_client.hpp punutils.hpp

puns_LTLIBRARIES = libpuns.la
libpuns_la_SOURCES = punutils.cpp cmdline.cpp
punsdir = $(pkglibdir)

bin_PROGRAMS = punserver cgi_punclient
punserver_SOURCES = punserver.cpp punchlines.cpp phonics.cpp 
cgi_punclient_SOURCES = cgi_client.cpp
punserver_LDADD = libpuns.la
cgi_punclient_LDADD = libpuns.la

html_DATA = cgi_begin.html
htmldir = $(pkgdatadir)/data

EXTRA_DIST = $(CMDLINE_VAR_INFO_FILE) cgi_begin.html
MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
