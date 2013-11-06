CMDLINE_VAR_INFO_FILE=$(srcdir)/cmdline_var_info.txt
CMDLINE_FILE=.rhovotrc

DASH_I = -I. -I$(includedir)

AM_CPPFLAGS = $(KPPFLAGS) $(DASH_I) -DDATADIR=\"$(pkgdatadir)/data\"
AM_CXXFLAGS =
AM_CFLAGS =
CPPFLAGS =
CXXFLAGS =
CFLAGS =


lib_LTLIBRARIES = librhovot.la
librhovot_la_SOURCES = cmdline.cpp car.cpp track.cpp curses_stuff.cpp
librhovot_la_LDFLAGS = $(KDFLAGS) -lncurses

cmdline.cpp : cmdline.hpp
cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = rhovot.hpp

bin_PROGRAMS = rhovot
rhovot_SOURCES = main.cpp
rhovot_LDFLAGS= $(KDFLAGS) -L$(libdir) -lrhovot -lutils

EXTRA_DIST = $(CMDLINE_VAR_INFO_FILE)
MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
