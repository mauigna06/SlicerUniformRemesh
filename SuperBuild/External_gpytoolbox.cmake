set(proj gpytoolbox)

# Set dependency list
set(${proj}_DEPENDS
  libigl
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED GPYTOOLBOX_SOURCE_DIR AND NOT EXISTS ${GPYTOOLBOX_SOURCE_DIR})
  message(FATAL_ERROR "GPYTOOLBOX_SOURCE_DIR [${GPYTOOLBOX_SOURCE_DIR}] variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "https://github.com/sgsellan/gpytoolbox.git"
    GIT_TAG "8b9aed7"
    SOURCE_DIR ${EP_SOURCE_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDS}
    )
  set(${proj}_DIR ${EP_SOURCE_DIR})
  set(gpytoolbox_SOURCE_DIR ${EP_SOURCE_DIR} CACHE PATH "gpytoolbox source directory" FORCE)
  set(GPYTOOLBOX_SOURCE_DIR ${EP_SOURCE_DIR} CACHE PATH "gpytoolbox source directory (upper)" FORCE)

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

mark_as_superbuild(${proj}_DIR:PATH)
mark_as_superbuild(GPYTOOLBOX_SOURCE_DIR:PATH)
