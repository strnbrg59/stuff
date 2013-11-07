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

COMMON_CeePeePeeFLAGS = -Wall -DCH_LANG_CC -DCH_USE_HDF5
COMMON_ElDeeFLAGS = -L$(HDF5_LIBRARY_DIR) -lhdf5

CHOMBOLIBS = -lboxtools -lamrtools

bridge_LTLIBRARIES = libbridge2.la libbridge3.la
bridgedir = $(pkglibdir)

libbridge2_la_SOURCES = bridge_module.cpp
libbridge2_la_CPPFLAGS = $(COMMON_CeePeePeeFLAGS) -DCH_SPACEDIM=2 -I$(CHOMBO_PREFIX)/2D/include/Chombo
libbridge2_la_LDFLAGS = $(COMMON_ElDeeFLAGS) -L$(CHOMBO_PREFIX)/2D/lib/Chombo $(CHOMBOLIBS) -module

libbridge3_la_SOURCES = bridge_module.cpp
libbridge3_la_CPPFLAGS = $(COMMON_CeePeePeeFLAGS) -DCH_SPACEDIM=3 -I$(CHOMBO_PREFIX)/3D/include/Chombo
libbridge3_la_LDFLAGS = $(COMMON_ElDeeFLAGS) -L$(CHOMBO_PREFIX)/3D/lib/Chombo $(CHOMBOLIBS) -module

EXTRA_DIST = bridge_module.cpp bridge_module.h void_pointers.h
