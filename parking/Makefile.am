CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.parkingsrc

AM_CPPFLAGS=-Wall -c -g -I$(OPENGL_HOME)/include -I$(includedir) -I$(HOME)/usr/include/boost_1_33_0

lib_LTLIBRARIES = libparking.la
libparking_la_SOURCES = cmdline.cpp

cmdline.cpp : cmdline.hpp

cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(srcdir)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = parking.hpp \
                  cmdline.hpp

bin_PROGRAMS = parking
parking_SOURCES = parking.cpp
parking_LDFLAGS=-L$(libdir) -lparking -lutils

EXTRA_DIST = cmdline_var_info.txt

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp bargraph.gpd

