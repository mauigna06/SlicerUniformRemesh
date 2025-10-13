project(vtkUniformRemesh)

include(SlicerMacroPythonWrapModuleVTKLibrary)

if(NOT DEFINED Eigen3_SOURCE_DIR)
  message(FATAL_ERROR "Eigen3_SOURCE_DIR is not set. Configure the superbuild to provide Eigen3.")
endif()
if(NOT EXISTS ${Eigen3_SOURCE_DIR})
  message(FATAL_ERROR "Eigen3 source directory '${Eigen3_SOURCE_DIR}' does not exist.")
endif()

if(NOT DEFINED LIBIGL_SOURCE_DIR)
  message(FATAL_ERROR "LIBIGL_SOURCE_DIR is not set. Configure the superbuild to provide libigl.")
endif()
if(NOT EXISTS ${LIBIGL_SOURCE_DIR})
  message(FATAL_ERROR "libigl source directory '${LIBIGL_SOURCE_DIR}' does not exist.")
endif()

if(NOT DEFINED GPYTOOLBOX_SOURCE_DIR)
  message(FATAL_ERROR "GPYTOOLBOX_SOURCE_DIR is not set. Configure the superbuild to provide gpytoolbox.")
endif()
if(NOT EXISTS ${GPYTOOLBOX_SOURCE_DIR})
  message(FATAL_ERROR "gpytoolbox source directory '${GPYTOOLBOX_SOURCE_DIR}' does not exist.")
endif()

# --------------------------------------------------------------------------
# libigl (header-only / interface) setup
# --------------------------------------------------------------------------
if(NOT TARGET igl::core)
  set(LIBIGL_BUILD_TUTORIALS OFF CACHE BOOL "" FORCE)
  set(LIBIGL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(LIBIGL_BUILD_PYTHON OFF CACHE BOOL "" FORCE)
  set(LIBIGL_USE_STATIC_LIBRARY OFF CACHE BOOL "" FORCE)
  set(LIBIGL_INSTALL OFF CACHE BOOL "" FORCE)
  add_subdirectory(${LIBIGL_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libigl EXCLUDE_FROM_ALL)
endif()

# --------------------------------------------------------------------------
# gpytoolbox remesher sources
# --------------------------------------------------------------------------
set(gpytoolbox_remesh_sources
  ${GPYTOOLBOX_SOURCE_DIR}/src/cpp/remesher/collapse_edges.cpp
  ${GPYTOOLBOX_SOURCE_DIR}/src/cpp/remesher/equalize_valences.cpp
  ${GPYTOOLBOX_SOURCE_DIR}/src/cpp/remesher/remesh_botsch.cpp
  ${GPYTOOLBOX_SOURCE_DIR}/src/cpp/remesher/split_edges.cpp
  ${GPYTOOLBOX_SOURCE_DIR}/src/cpp/remesher/split_edges_until_bound.cpp
  ${GPYTOOLBOX_SOURCE_DIR}/src/cpp/remesher/tangential_relaxation.cpp
  )

add_library(gpytoolbox_remesh STATIC ${gpytoolbox_remesh_sources})
set_target_properties(gpytoolbox_remesh PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  )
target_compile_features(gpytoolbox_remesh PUBLIC cxx_std_17)
target_include_directories(gpytoolbox_remesh
  PUBLIC
    ${GPYTOOLBOX_SOURCE_DIR}/src/cpp
    ${Eigen3_SOURCE_DIR}
    ${LIBIGL_SOURCE_DIR}/include
    ${LIBIGL_SOURCE_DIR}
  )
target_link_libraries(gpytoolbox_remesh
  PUBLIC
    igl::core
  )

# --------------------------------------------------------------------------
# vtkUniformRemesh module
# --------------------------------------------------------------------------
set(vtkUniformRemesh_SRCS
  vtkUniformRemeshFilter.cxx
  vtkUniformRemeshFilter.h
  )

set(configure_header vtkUniformRemeshConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header}
  )

add_library(${PROJECT_NAME} ${vtkUniformRemesh_SRCS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    gpytoolbox_remesh
    ${VTK_LIBRARIES}
  )
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${PROJECT_NAME} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  )

# --------------------------------------------------------------------------
# Installation
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR})
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR})
endif()

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkUniformRemeshFilter.h
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header}
  DESTINATION include/${PROJECT_NAME}
  COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)
  SlicerMacroPythonWrapModuleVTKLibrary(
    NAME ${PROJECT_NAME}
    SRCS ${vtkUniformRemesh_SRCS}
    WRAPPED_TARGET_LIBRARIES ${PROJECT_NAME}
    RELATIVE_PYTHON_DIR "."
    )
endif()
