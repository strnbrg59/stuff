##   _______              __
##  / ___/ /  ___  __ _  / /  ___
## / /__/ _ \/ _ \/  ' \/ _ \/ _ \
## \___/_//_/\___/_/_/_/_.__/\___/ 
##
## This software is copyright (C) by the Lawrence Berkeley
## National Laboratory.  Permission is granted to reproduce
## this software for non-commercial purposes provided that
## this notice is left intact.
## 
## It is acknowledged that the U.S. Government has rights to
## this software under Contract DE-AC03-765F00098 between
## the U.S. Department of Energy and the University of
## California.
##
## This software is provided as a professional and academic
## contribution for joint exchange.  Thus it is experimental,
## is provided ``as is'', with no warranties of any kind
## whatsoever, no support, no promise of updates, or printed
## documentation.  By using this software, you acknowledge
## that the Lawrence Berkeley National Laboratory and
## Regents of the University of California shall have no
## liability with respect to the infringement of other
## copyrights by any part of this software.
##
## Author: Ted Sternberg

CMDLINE_VAR_INFO_FILE=$(srcdir)/cmdline_var_info.txt
CMDLINE_FILE=.pychombovisrc
cmdline.cpp : cmdline.h
cmdline.h : $(CMDLINE_VAR_INFO_FILE)
	python $(srcdir)/generate_cmdline.py $(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)
	-mv -f cmdline.cpp $(srcdir)
	-mv -f cmdline.h $(srcdir)

AM_CPPFLAGS = -Wall -I$(HDF5_INCLUDE_DIR) -I$(topsrcdir)/utils -ftemplate-depth-30 -Wno-deprecated

headers_HEADERS = $(srcdir)/*.h $(srcdir)/*.hImpl
headersdir = $(pkgincludedir)/utils

utils_LTLIBRARIES = libchombovis_utils.la
libchombovis_utils_la_SOURCES = \
         ostream_stdio_adaptor.cpp \
         cmdline.cpp      \
         Trace.cpp       \
         Consts.cpp      \
         cmdline_base.cpp      \
         argmap.cpp      \
         StatusCodes.cpp \
         Timer.cpp       \
         BoxFinder.cpp \
         RectanglePacker.cpp
utilsdir = $(pkglibdir)

nodist_boost_HEADERS = untarred_boost
untarred_boost :
	gunzip -f boost.tar.gz
	tar xvf boost.tar
	touch untarred_boost
	gzip -f boost.tar
boostdir = $(pkgincludedir)
install-data-local:
	-mkdir -p $(pkgincludedir)
	cp boost.tar.gz $(pkgincludedir); cd $(pkgincludedir); gunzip -f boost.tar.gz; tar xvf boost.tar

EXTRA_DIST = *.h *.hImpl cmdline_var_info.txt generate_cmdline.py boost.tar.gz
clean-local :
	-rm -rf boost
	-rm untarred_boost
