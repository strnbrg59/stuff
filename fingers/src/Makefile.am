CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.fingersrc

AM_CPPFLAGS=-Wall -c -g -I$(includedir) -I$(HOME)/usr/include/boost_1_33_0 

lib_LTLIBRARIES = libfingers.la
libfingers_la_SOURCES = cmdline.cpp \
                        note.cpp \
                        tree.cpp \
                        score.cpp \
                        norms.cpp \
                        chorda.cpp
                        

cmdline.cpp : cmdline.hpp

cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(srcdir)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = fingers.hpp

bin_PROGRAMS = fingers
fingers_SOURCES = main.cpp
fingers_LDFLAGS=-L$(libdir) -lfingers -lutils

EXTRA_DIST = cmdline_var_info.txt

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
