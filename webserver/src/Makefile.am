CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.webserverrc

AM_CPPFLAGS=-I$(includedir)

lib_LTLIBRARIES = libwebserver.la
libwebserver_la_SOURCES = cmdline.cpp webserver.cpp

cmdline.cpp : cmdline.hpp

cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python ../../utils/src/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = webserver.hpp

bin_PROGRAMS = webserver
webserver_SOURCES = webserver-main.cpp
webserver_LDFLAGS=-L$(libdir) -lwebserver -lutils

#
# Somehow, if you try to build mapclient, bootstrap tells you
# that cmdline.$(OBJECT) will be built both with and without
# libtool, and doesn't generate a Makefile.in.
# This is new behavior -- since moving to almanzo.trhj.homeunix.net,
# with its slightly newer automake.
#
#mapclient_SOURCES = mapclient.cpp cmdline.cpp
#mapclient_LDFLAGS=-L$(libdir) -lutils

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
