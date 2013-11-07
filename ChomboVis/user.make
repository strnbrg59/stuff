#
#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
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
#############################################################

#
# This file defines some paths ChomboVis needs to know about: they begin
# at the line that starts with "# Configuration paths start here".
# Make sure there are no spaces around '=' signs!
#
# The simplest, and possibly safest, thing to do is type in the appropriate
# absolute paths for your system.  However, since this file gets
# interpreted as a Bourne shell script, you can, as a convenience, give those
# paths in terms of variables you define for yourself.
#

# On a MAC OS X where you used Fink to install VTK, set INSTALL_HOME to /sw.
INSTALL_HOME=$HOME/anag/everything-gcc3.4.2/usr

#
# Configuration paths start here.
#
# VTK libraries should be under this in lib/vtk, headers in include/vtk, 
# vtkpython and other executables in bin.  If you give configure the
# --without-vtk argument, then you can comment out this line.
VTK_INSTALL_DIR=$INSTALL_HOME

#
# Set the path to the HDF5 include files directory:
#
HDF5_INCLUDE_DIR=$INSTALL_HOME/include
HDF5_LIBRARY_DIR=$INSTALL_HOME/lib

PYTHON_INCLUDE_DIR=$INSTALL_HOME/include/python2.3

#
# Place where Chombo has been installed by the chombovis_support.sh script
# that's in the root of the Chombo source distribution.  Under CHOMBO_PREFIX
# you should have a directory named 2D and a directory named 3D.
# If BUILD_CHOMBO_BRIDGE == no, then CHOMBO_PREFIX isn't used.
#
CHOMBO_PREFIX=$HOME/builds/chombo/for_chombovis
BUILD_CHOMBO_BRIDGE=no
