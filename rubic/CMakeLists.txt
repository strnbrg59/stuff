PROJECT (VTKRubic)

INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF (USE_VTK_FILE)
  INCLUDE(${USE_VTK_FILE})
ENDIF (USE_VTK_FILE)

SET(VTKRubic_SRCS
    vtkRubicPythonIface.cxx
    rubic.cxx
    vtk_stuff.cxx)

ADD_EXECUTABLE(rubic main.cxx rubic.cxx vtk_stuff.cxx)
#ADD_EXECUTABLE(unittest unittest.cxx rubic.cxx)
#ADD_EXECUTABLE(orbitlength orbitlength.cxx rubic.cxx)
#ADD_EXECUTABLE(random_turns random_turns.cxx rubic.cxx)

TARGET_LINK_LIBRARIES(rubic vtkRendering)

# List the kits from VTK that are needed by this project.
SET(VTKRubic_LIBS
  vtkCommon
  vtkGraphics
  vtkRendering
  vtkPatented
)

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

# Set VTKRUBIC_BUILD_SHARED_LIBS to 1 or 0 for use in creating the
# configuration header.
IF(BUILD_SHARED_LIBS)
  SET(VTKRUBIC_BUILD_SHARED_LIBS_CONFIG 1)
ELSE(BUILD_SHARED_LIBS)
  SET(VTKRUBIC_BUILD_SHARED_LIBS_CONFIG 0)
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
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/VTKRubicConfigure.h.in
                 ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Configure.h
                 @ONLY IMMEDIATE)

  # Create the instantiator for these classes.
  SET(VTK_USE_INSTANTIATOR_NEW 1)
  VTK_MAKE_INSTANTIATOR2("${PROJECT_NAME}Instantiator" VTKRubicInstantiator_SRCS
                         vtkRubicPythonIface.cxx
                         EXPORT_MACRO "VTK_${PROJECT_NAME}_EXPORT"
                         HEADER_LOCATION ${PROJECT_BINARY_DIR}
                         INCLUDES "${PROJECT_NAME}Configure.h")

  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${VTKRubic_SRCS} ${VTKRubicInstantiator_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${VTKRubic_LIBS})

  # Create Python wrappers.
  IF(VTK_WRAP_PYTHON)
    SET(VTK_WRAP_HINTS "${PROJECT_SOURCE_DIR}/wrap_hints")
    VTK_WRAP_PYTHON2(${PROJECT_NAME}Python VTKRubicPYTHON_SRCS vtkRubicPythonIface.cxx)
    IF (APPLE)
      ADD_LIBRARY(${PROJECT_NAME}PythonD ${VTKRubicPYTHON_SRCS})
      ADD_LIBRARY(${PROJECT_NAME}Python MODULE ${PROJECT_NAME}PythonInit.cxx)
      TARGET_LINK_LIBRARIES(${PROJECT_NAME}PythonD vtkCommonPythonD ${PROJECT_NAME} ${VTKRubicPython_LIBS})
      TARGET_LINK_LIBRARIES(${PROJECT_NAME}Python ${PROJECT_NAME}PythonD ${VTKRubicPython_LIBS})
      INSTALL_TARGETS(/lib/vtk ${PROJECT_NAME}PythonD)
    ELSE (APPLE)
      ADD_LIBRARY(${PROJECT_NAME}Python MODULE ${VTKRubicPYTHON_SRCS})
      TARGET_LINK_LIBRARIES (${PROJECT_NAME}Python vtkCommonPython ${PROJECT_NAME} ${VTKRubicPython_LIBS})
    ENDIF(APPLE)
    INSTALL_TARGETS(/lib/vtk ${PROJECT_NAME}Python)
  ENDIF (VTK_WRAP_PYTHON)

ENDIF(VTK_FOUND)
