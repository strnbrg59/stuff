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


AM_CPPFLAGS = -Wall -ftemplate-depth-30 -I$(HDF5_INCLUDE_DIR) -I$(PYTHON_INCLUDE_DIR) -I$(srcdir)/../../utils 

headers_HEADERS = module_templates.hImpl module_templates.cpp BoxLayoutDataInterfaceForPython.h  VisualizableDatasetInterfaceForPython.h NewComponentGenerator.h
headersdir = $(pkgincludedir)/data_access/python

pylib_LTLIBRARIES = libvisdat.la libbld.la
pylibdir = $(pkglibdir)

libvisdat_la_SOURCES = visdatmodule.cpp
libvisdat_la_LIBADD = ../../utils/libchombovis_utils.la ../libchombovis_data.la
libvisdat_la_LDFLAGS = -module -L$(HDF5_LIBRARY_DIR) -lhdf5

libbld_la_SOURCES = bldmodule.cpp
libbld_la_LIBADD = ../../utils/libchombovis_utils.la ../libchombovis_data.la
libbld_la_LDFLAGS =  -module -L$(HDF5_LIBRARY_DIR) -lhdf5

EXTRA_DIST = *.h *.hImpl module_templates.cpp
