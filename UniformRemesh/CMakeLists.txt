#-----------------------------------------------------------------------------
set(MODULE_NAME UniformRemesh)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/${MODULE_NAME}.png
  Resources/UI/${MODULE_NAME}.ui
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )

#----------------------------------------------------------------------------- 
set(vtkUniformRemesh_SRCS
  vtkUniformRemesh.cxx
  )

add_library(vtkUniformRemesh ${vtkUniformRemesh_SRCS})

target_include_directories(vtkUniformRemesh
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )

if(NOT TARGET cpytoolbox)
  set(_gpytoolbox_include_hint_paths)
  if(DEFINED gpytoolbox_DIR)
    list(APPEND _gpytoolbox_include_hint_paths
      ${gpytoolbox_DIR}
      ${gpytoolbox_DIR}/src
      ${gpytoolbox_DIR}/../src
      ${gpytoolbox_DIR}/../gpytoolbox/src
      )
  endif()

  find_path(GPYTOOLBOX_INCLUDE_DIR
    NAMES remesher/remesh_botsch.h
    HINTS ${_gpytoolbox_include_hint_paths}
    PATH_SUFFIXES src/cpp cpp
    )

  if(NOT GPYTOOLBOX_INCLUDE_DIR)
    message(FATAL_ERROR "Unable to locate remesher/remesh_botsch.h. Set gpytoolbox_DIR or GPYTOOLBOX_INCLUDE_DIR.")
  endif()

  target_include_directories(vtkUniformRemesh
    PRIVATE
      ${GPYTOOLBOX_INCLUDE_DIR}
    )

  set(_gpytoolbox_library_hint_paths)
  if(DEFINED gpytoolbox_DIR)
    list(APPEND _gpytoolbox_library_hint_paths
      ${gpytoolbox_DIR}
      ${gpytoolbox_DIR}/lib
      ${gpytoolbox_DIR}/../lib
      )
  endif()

  find_library(GPYTOOLBOX_LIBRARY
    NAMES cpytoolbox
    HINTS ${_gpytoolbox_library_hint_paths}
    )

  if(NOT GPYTOOLBOX_LIBRARY)
    message(FATAL_ERROR "Unable to locate gpytoolbox library 'cpytoolbox'. Set gpytoolbox_DIR or GPYTOOLBOX_LIBRARY.")
  endif()

  target_link_libraries(vtkUniformRemesh
    PRIVATE
      ${GPYTOOLBOX_LIBRARY}
    )
else()
  target_link_libraries(vtkUniformRemesh
    PRIVATE
      cpytoolbox
    )
endif()

target_link_libraries(vtkUniformRemesh
  PRIVATE
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersCore
  )

vtk_module_autoinit(
  TARGETS vtkUniformRemesh
  MODULES VTK::CommonCore VTK::CommonDataModel VTK::FiltersCore
  )

set_target_properties(vtkUniformRemesh PROPERTIES
  OUTPUT_NAME vtkUniformRemesh
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )

install(TARGETS vtkUniformRemesh
  ARCHIVE DESTINATION ${Slicer_INSTALL_MODULE_LIB_DIR}
  LIBRARY DESTINATION ${Slicer_INSTALL_MODULE_LIB_DIR}
  RUNTIME DESTINATION ${Slicer_INSTALL_MODULE_BIN_DIR}
  )

install(FILES vtkUniformRemesh.h
  DESTINATION ${Slicer_INSTALL_MODULE_SHARE_DIR}/${MODULE_NAME}/include
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)

  # Register the unittest subclass in the main script as a ctest.
  # Note that the test will also be available at runtime.
  slicer_add_python_unittest(SCRIPT ${MODULE_NAME}.py)

  # Additional build-time testing
  add_subdirectory(Testing)
endif()
