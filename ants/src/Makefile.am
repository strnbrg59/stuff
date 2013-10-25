CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.antsrc

AM_CPPFLAGS=-Wall -c -g -I$(OPENGL_HOME)/include -I$(X11_HOME)/include -I$(includedir) -I$(HOME)/usr/include/boost_1_33_0 

lib_LTLIBRARIES = libants.la
libants_la_SOURCES = cmdline.cpp \
                     display.cpp \
                     field.cpp \
                     ant.cpp \
                     antutils.cpp


cmdline.cpp : cmdline.hpp

cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(srcdir)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = display.hpp field.hpp antutils.hpp ant.hpp

bin_PROGRAMS = ants
ants_SOURCES = main.cpp
ants_LDFLAGS=-L$(libdir) -L$(OPENGL_HOME)/lib -L$(X11_HOME)/lib -lants -lutils -lX11 -lGL -lGLU -lglut -lXmu

EXTRA_DIST = cmdline_var_info.txt

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
