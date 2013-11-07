# These things are not scripts, but we're using automake's SCRIPTS primary
# because that ensures (1) that these .so's will be built, and (2) that autoconf
# or libtool doesn't try to build them for us (as that would interfere with
# cmake).

#
# The indentations under this "if VTK" have to be made of spaces.  If you use
# tabs, everything gets ignored.  However, the command ("mkdir ..") for the
# "phony:" target *does* have to follow a tab.
#
if VTK
   cmake_SCRIPTS = ../VtkBuild/bin/libVTKChombo.so ../VtkBuild/bin/libVTKChomboPython.so
   cmakedir = $(pkglibdir)
       
   headers_HEADERS = vtkChomboReader.h VTKChomboConfigure.h.in
   headersdir = $(pkgincludedir)/Vtk
       
   ../VtkBuild/bin/libVTKChombo.so : phony
   ../VtkBuild/bin/libVTKChomboPython.so : phony
   phony : checkmacosxenv
		mkdir -p ../VtkBuild; cd ../VtkBuild; cmake $(top_srcdir)/Vtk -DBUILD_SHARED_LIBS:BOOL=ON -DVTK_DIR:PATH=$(VTK_INSTALL_DIR)/lib/vtk -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE -DCMAKE_CXX_FLAGS:STRING='-I$(srcdir)/../utils -I$(HDF5_INCLUDE_DIR) $(CXXFLAGS)' -DCMAKE_C_FLAGS:STRING='$(CFLAGS)' -DHDF5_LIBRARY_DIR:FILEPATH=$(HDF5_LIBRARY_DIR); $(MAKE)
   checkmacosxenv :
		$(srcdir)/../premake/checkmacosxenv.sh $(host)

# Problem here: need libVTKChombo.so before attempting to link unittest, and
# we're having trouble enforcing that order of building.
#       
# if UNITTESTS
# AM_CPPFLAGS = -Wall -I. -I$(HDF5_INCLUDE_DIR) -I$(srcdir)/../utils -I$(srcdir)/../data_access -I$(VTK_INSTALL_DIR)/include/vtk -DUNWRAPPED -Wno-deprecated
# AM_LDFLAGS = -L$(HDF5_LIBRARY_DIR) -L$(pkglibdir) -lVTKChombo -lhdf5
# bin_PROGRAMS = chomboreader_unittest
# chomboreader_unittest_SOURCES = chomboreader_unittest.cpp
# chomboreader_unittest_LDADD = ../data_access/libchombovis_data.la ../utils/libchombovis_utils.la
# #chomboreader_unittest_LDFLAGS = -L$(HDF5_LIBRARY_DIR) -lhdf5
# endif UNITTESTS

   EXTRA_DIST = *.h *.cxx ../VtkImpl/*h ../VtkImpl/*cxx CMakeLists.txt pkgIndex.tcl.in VTKChomboConfigure.h.in
       
   MOSTLYCLEANDIRS = ../VtkBuild
endif VTK
