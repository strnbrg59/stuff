#
#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
#
# Makefile.am
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
# 
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#

SUBDIRS = $(TOP_MAKEFILE_SUBDIRS)


launcher_SCRIPTS = chombovis chombobrowser chombodata chombodatalite
launcherdir = $(bindir)
chombobrowser : chombovis
chombodata : chombovis
chombodatalite : chombovis
chombovis : chombovis.template
	rm -f chombovis
	echo "#!/bin/sh" > chombovis
	echo VTK_INSTALL_DIR=$(VTK_INSTALL_DIR) >> chombovis
	echo "export VTK_INSTALL_DIR" >> chombovis
	echo PYTHON_INCLUDE_DIR=$(PYTHON_INCLUDE_DIR) >> chombovis
	echo "export PYTHON_INCLUDE_DIR" >> chombovis
	echo VTK_TK_WIDGET_PATH=$(VTK_INSTALL_DIR) >> chombovis
	echo "export VTK_TK_WIDGET_PATH" >> chombovis
	echo CHOMBOVIS_HOME=$(prefix) >> chombovis
	echo "export CHOMBOVIS_HOME" >> chombovis
	echo HDF5_INCLUDE_DIR=$(HDF5_INCLUDE_DIR) >> chombovis
	echo "export HDF5_INCLUDE_DIR" >> chombovis
	echo HDF5_LIBRARY_DIR=$(HDF5_LIBRARY_DIR) >> chombovis
	echo "export HDF5_LIBRARY_DIR" >> chombovis
	echo PATH=$(prefix)/libexec/ChomboVis:$(VTK_INSTALL_DIR)/bin:$(PATH) >> chombovis
	echo "export PATH" >> chombovis
	echo GPP=`which g++` >> chombovis
	echo "export GPP" >> chombovis
	grep -v "^This is not" $(top_srcdir)/chombovis.template >> chombovis
	chmod a+x chombovis
	rm -f chombobrowser; ln -s chombovis chombobrowser
	rm -f chombodata; ln -s chombovis chombodata
if VTK
	rm -f chombodatalite; ln -s chombovis chombodatalite
else !VTK
	cp -f chombovis chombodatalite
	rm -f chombovis chombobrowser chombodata
endif !VTK


# Gotta do this; for some weird reason the boost.tar doesn't get untarred until
# you run "make install" twice.
install-data-local:
	(cd utils; $(MAKE) install)

EXTRA_DIST = README chombovis.template user.make wrap/hints

MOSTLYCLEANFILES = chombovis chombobrowser chombodata chombodatalite
