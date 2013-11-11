CMDLINE_VAR_INFO_FILE=$(srcdir)/cmdline_var_info.txt
CMDLINE_FILE=.waxrc

DASH_I = -I. -I$(includedir) -I/work/PROD_COMMON_LIBS/usr/home/nfs/common/Linux/c++/4.1.2/boost_1_38/include/boost-1_38

AM_CPPFLAGS = $(KPPFLAGS) $(DASH_I) -DDATADIR=\"$(pkgdatadir)/data\"
AM_CXXFLAGS =
AM_CFLAGS =
CPPFLAGS =
CXXFLAGS =
CFLAGS =


lib_LTLIBRARIES = libwax.la
libwax_la_SOURCES = cmdline.cpp piece.cpp board.cpp wax_tree.cpp square.cpp move.cpp
libwax_la_LDFLAGS = $(KDFLAGS)

cmdline.cpp : cmdline.hpp
cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = piece.hpp board.hpp wax_tree.hpp move.hpp square.hpp position.hpp

bin_PROGRAMS = wax
wax_SOURCES = wax.cpp
wax_LDFLAGS= $(KDFLAGS) -L$(libdir) -lwax -lutils

EXTRA_DIST = $(CMDLINE_VAR_INFO_FILE)
MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp

#/work/PROD_COMMON_LIBS/usr/home/nfs/common/Linux/c++/4.1.1/boost_1_38/include/boost-1_38/boost/foreach.hpp
#/work/PROD_COMMON_LIBS/usr/home/nfs/common/Linux/x86_64/c++/4.1.2/boost_1_38/include/boost-1_38/boost/foreach.hpp
#/work/PROD_COMMON_LIBS/usr/home/nfs/common/Linux/i386/c++/4.1.2/boost_1_38/include/boost-1_38/boost/foreach.hpp
