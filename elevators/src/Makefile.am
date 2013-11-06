CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.elevatorssrc

AM_CPPFLAGS=-Wall -c -g -I$(OPENGL_HOME)/include -I$(includedir) -I$(HOME)/usr/include/boost_1_33_0

lib_LTLIBRARIES = libelevators.la
libelevators_la_SOURCES = cmdline.cpp \
                          building.cpp \
                          elevator.cpp \
                          clock.cpp \
                          floor.cpp \
                          riders.cpp \
                          curses_stuff.cpp

cmdline.cpp : cmdline.hpp

cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(srcdir)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = el.hpp

bin_PROGRAMS = elevators
elevators_SOURCES = main.cpp
elevators_LDFLAGS=-L$(libdir) -lelevators -lutils -lncurses

EXTRA_DIST = cmdline_var_info.txt

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
