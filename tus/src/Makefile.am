CMDLINE_VAR_INFO_FILE=$(srcdir)/cmdline_var_info.txt
CMDLINE_FILE=.tusrc

X11_HOME=/usr/X11R6
OPENGL_HOME=/usr/X11R6

DASH_I = -I. -I$(includedir) -I$(OPENGL_HOME)/include -I$(X11_HOME)/include

AM_CPPFLAGS = $(KPPFLAGS) $(DASH_I) -DDATADIR=\"$(pkgdatadir)/data\"
AM_CXXFLAGS =
AM_CFLAGS =
CPPFLAGS =
CXXFLAGS =
CFLAGS =


lib_LTLIBRARIES = libtus.la
libtus_la_SOURCES = cmdline.cpp draw.cpp mouse.cpp elevations.cpp utils.cpp
libtus_la_LDFLAGS = $(KDFLAGS)

cmdline.cpp : cmdline.hpp
cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = draw.hpp mouse.hpp elevations.hpp utils.hpp

bin_PROGRAMS = tus
tus_SOURCES = tus.cpp
tus_LDFLAGS= $(KDFLAGS) -L$(libdir)  -L$(OPENGL_HOME)/lib -L$(X11_HOME)/lib -ltus -lutils -lGL -lGLU -lglut -lXmu

EXTRA_DIST = $(CMDLINE_VAR_INFO_FILE)
MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp *~
