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


SUBDIRS = . python
AM_CPPFLAGS = -Wall -I$(PYTHON_INCLUDE_DIR) -I$(HDF5_INCLUDE_DIR) -I$(srcdir)/../utils -ftemplate-depth-30
AM_LDFLAGS = -L$(HDF5_LIBRARY_DIR) -lhdf5

headers_HEADERS = $(srcdir)/*.h $(srcdir)/*.hImpl
headersdir = $(pkgincludedir)/data_access

data_access_LTLIBRARIES = libchombovis_data.la
no_bridge_sources = FAB.cpp ChomboHDF5_subclasses.cpp Box.cpp Intvect.cpp ChomboHDF5.cpp PointerHandleMap.cpp VisualizableDataset.cpp BoxLayoutData.cpp BoxLayout.cpp
if BUILD_CHOMBO_BRIDGE_AM
  libchombovis_data_la_SOURCES = $(no_bridge_sources) ChomboBridge.cpp LtdlModule.cpp
  libchombovis_data_la_LIBADD = ../utils/libchombovis_utils.la @LIBLTDL@
else
  libchombovis_data_la_SOURCES = $(no_bridge_sources)
  libchombovis_data_la_LIBADD = ../utils/libchombovis_utils.la
endif !BUILD_CHOMBO_BRIDGE_AM
data_accessdir = $(pkglibdir)

#
# Binary programs
#
if UNITTESTS

no_bridge_progs = ascii2hdf5 ChomboHDF5_unittest VisDat_unittest

if BUILD_CHOMBO_BRIDGE_AM
bin_PROGRAMS = $(no_bridge_progs) ChomboBridge_unittest
else !BUILD_CHOMBO_BRIDGE_AM
bin_PROGRAMS = $(no_bridge_progs)
endif !BUILD_CHOMBO_BRIDGE_AM

ChomboHDF5_unittest_SOURCES = ChomboHDF5_unittest.cpp
ChomboHDF5_unittest_LDADD = ./libchombovis_data.la ../utils/libchombovis_utils.la
ChomboHDF5_unittest_LDFLAGS = -L$(HDF5_LIBRARY_DIR) -lhdf5

VisDat_unittest_SOURCES = VisDat_unittest.cpp
VisDat_unittest_LDADD = ./libchombovis_data.la ../utils/libchombovis_utils.la
VisDat_unittest_LDFLAGS = -L$(HDF5_LIBRARY_DIR) -lhdf5

if BUILD_CHOMBO_BRIDGE_AM
ChomboBridge_unittest_SOURCES = ChomboBridge_unittest.cpp
ChomboBridge_unittest_LDADD = ./libchombovis_data.la ../utils/libchombovis_utils.la
ChomboBridge_unittest_LDFLAGS = -L$(HDF5_LIBRARY_DIR) -lhdf5
endif BUILD_CHOMBO_BRIDGE_AM

else !UNITTESTS
bin_PROGRAMS = ascii2hdf5
endif !UNITTESTS

bindir = $(libexecdir)/ChomboVis

ascii2hdf5_SOURCES = ascii2hdf5-main.cpp Ascii2HDF5.cpp
ascii2hdf5_LDADD = ./libchombovis_data.la ../utils/libchombovis_utils.la
ascii2hdf5_LDFLAGS = -L$(HDF5_LIBRARY_DIR) -lhdf5

#
# Scripts
#
ascii_SCRIPTS = ascii2hdf5.sh
asciidir = $(bindir)
ascii2hdf5.sh :
	echo "sed 's/#.*$///' | $(libexecdir)/ChomboVis/ascii2hdf5 \\" > ascii2hdf5.sh
	cat $(srcdir)/dollar.star | grep -v '^#' >> ascii2hdf5.sh
# dollar.star is there because I can't get echo '$*' or echo "\$*" to produce
# the right thing, when called here.

if UNITTESTS
    unittest_SCRIPTS = unittests.sh
else !UNITTESTS
    unittest_SCRIPTS = no_unittests.sh
endif !UNITTESTS
unittestdir = $(libexecdir)/ChomboVis

EXTRA_DIST = *.h *.hImpl dollar.star *unittests.sh
MOSTLYCLEANFILES = ascii2hdf5.sh *unittest
