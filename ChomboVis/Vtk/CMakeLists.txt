CMAKE_MINIMUM_REQUIRED(VERSION 1.6)

# Set the project/library name here.  Classes should be declared as
# "class VTKChombo_EXPORT vtkFoo", where VTKChombo is the name of the
# project set here.
PROJECT(VTKChombo)

# Set your list of sources here.  Do not change the name of the
# VTKChombo_SRCS variable.
SET(VTKChombo_SRCS
  vtkChomboRawGL.cxx
  vtkChomboLookupTable.cxx
  vtkChomboPlaneSource.cxx
  vtkChomboReader.cxx
  vtkMattSlicer.cxx
  vtkChomboEmbeddedBoundaryFilter.cxx
  vtkRayBuilder.cxx
  vtkAMRVolume.cxx
  vtkChomboParticles.cxx
  vtkAMRStreamSource.cxx
  vtkChomboTexture.cxx
  vtkChomboAccumulatePolyData.cxx
  vtkChomboOutlineSource.cxx
  vtkBBoxSource.cxx
)

# To understand how this works, look for -DHDF5_LIBRARY_DIR:FILEPATH=... in
# Makefile.am.
SET (HDF5_LIBRARY_DIR will_be_set_by_automake_generated_Makefile CACHE FILEPATH
     "path to libhdf5.so|dylib")
FIND_LIBRARY(HDF5_LIBRARY hdf5 ${HDF5_LIBRARY_DIR})

# List the kits from VTK that are needed by this project.
SET(VTKChombo_LIBS
  vtkCommon
  vtkGraphics
  vtkRendering
  vtkPatented
)

# Libraries to link only the Python-wrapped library:
IF (APPLE)
   SET(VTKChomboPython_LIBS
     vtkFilteringPythonD
     vtkGraphicsPythonD
     )
ELSE (APPLE)
   SET(VTKChomboPython_LIBS
     vtkFilteringPython
     vtkGraphicsPython
     )
ENDIF (APPLE)

# Libraries built in other directories, under control of autoconf:
FIND_LIBRARY(CHOMBOVIS_UTILS_LIBRARY chombovis_utils ../utils/.libs)
FIND_LIBRARY(CHOMBOVIS_DATA_LIBRARY chombovis_data ../data_access/.libs)
FIND_LIBRARY(CHOMBOVIS_EB_LIBRARY EBSurfaceSingleCell ../EBLib/.libs)
SET(autoconf_LIBS
  ${CHOMBOVIS_UTILS_LIBRARY}
  ${CHOMBOVIS_DATA_LIBRARY}
  ${CHOMBOVIS_EB_LIBRARY}
)

ADD_DEFINITIONS( -ftemplate-depth-30 -Wno-long-long )

#-----------------------------------------------------------------------------
# Most users should not need to change anything below this line.

# Need to include class headers and the configuration header.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

# Find and load VTK settings.
INCLUDE(${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ENDIF(VTK_FOUND)

# Give user option of building shared or static libraries.  Default to
# the choice made for VTK.
OPTION(BUILD_SHARED_LIBS "Build with shared libraries." 
       ${VTK_BUILD_SHARED_LIBS})

# Set VTKCHOMBO_BUILD_SHARED_LIBS to 1 or 0 for use in creating the
# configuration header.
IF(BUILD_SHARED_LIBS)
  SET(VTKCHOMBO_BUILD_SHARED_LIBS_CONFIG 1)
ELSE(BUILD_SHARED_LIBS)
  SET(VTKCHOMBO_BUILD_SHARED_LIBS_CONFIG 0)
ENDIF(BUILD_SHARED_LIBS)

SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH
     "Single output directory for building all executables.")

MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

IF(VTK_FOUND)
  # If this is a build tree, provide an option for putting this
  # project's executables and libraries in with VTK's.
  IF (EXISTS ${VTK_DIR}/bin)
    OPTION(USE_VTK_OUTPUT_PATHS
     "Use VTK's output directory for this project's executables and libraries."
     OFF)
    MARK_AS_ADVANCED (USE_VTK_OUTPUT_PATHS)
    IF (USE_VTK_OUTPUT_PATHS)
      SET (LIBRARY_OUTPUT_PATH ${VTK_DIR}/bin)
      SET (EXECUTABLE_OUTPUT_PATH ${VTK_DIR}/bin)
    ENDIF (USE_VTK_OUTPUT_PATHS)
  ENDIF (EXISTS ${VTK_DIR}/bin)

  # Create the header to configure the classes.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/VTKChomboConfigure.h.in
                 ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Configure.h
                 @ONLY IMMEDIATE)

  # Create the instantiator for these classes.
  SET(VTK_USE_INSTANTIATOR_NEW 1)
  VTK_MAKE_INSTANTIATOR2("${PROJECT_NAME}Instantiator" VTKChomboInstantiator_SRCS
                         ${VTKChombo_SRCS}
                         EXPORT_MACRO "VTK_${PROJECT_NAME}_EXPORT"
                         HEADER_LOCATION ${PROJECT_BINARY_DIR}
                         INCLUDES "${PROJECT_NAME}Configure.h")

  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${VTKChombo_SRCS} ${VTKChomboInstantiator_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${VTKChombo_LIBS} ${HDF5_LIBRARY} 
                        ${autoconf_LIBS})

  # Create Python wrappers.
  IF(VTK_WRAP_PYTHON)
    SET(VTK_WRAP_HINTS "${PROJECT_SOURCE_DIR}/../wrap/hints")
    VTK_WRAP_PYTHON2(${PROJECT_NAME}Python VTKChomboPYTHON_SRCS ${VTKChombo_SRCS})
    IF (APPLE)
      ADD_LIBRARY(${PROJECT_NAME}PythonD ${VTKChomboPYTHON_SRCS})
      ADD_LIBRARY(${PROJECT_NAME}Python MODULE ${PROJECT_NAME}PythonInit.cxx)
      TARGET_LINK_LIBRARIES(${PROJECT_NAME}PythonD vtkCommonPythonD ${PROJECT_NAME} ${autoconf_LIBS} ${VTKChomboPython_LIBS})
      TARGET_LINK_LIBRARIES(${PROJECT_NAME}Python ${PROJECT_NAME}PythonD ${autoconf_LIBS} ${VTKChomboPython_LIBS})
      INSTALL_TARGETS(/lib/vtk ${PROJECT_NAME}PythonD)
    ELSE (APPLE)
      ADD_LIBRARY(${PROJECT_NAME}Python MODULE ${VTKChomboPYTHON_SRCS})
      TARGET_LINK_LIBRARIES (${PROJECT_NAME}Python ${PROJECT_NAME} ${autoconf_LIBS} ${VTKChomboPython_LIBS})
    ENDIF(APPLE)
    INSTALL_TARGETS(/lib/vtk ${PROJECT_NAME}Python)
  ENDIF (VTK_WRAP_PYTHON)

ENDIF(VTK_FOUND)
