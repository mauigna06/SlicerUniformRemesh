set(proj libigl)

# Set dependency list
set(${proj}_DEPENDS
  Eigen3
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED libigl_SOURCE_DIR AND NOT EXISTS ${libigl_SOURCE_DIR})
  message(FATAL_ERROR "libigl_SOURCE_DIR [${libigl_SOURCE_DIR}] variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "https://github.com/libigl/libigl.git"
    GIT_TAG "08be0704c26359bcdbe17449054e6c56b9b7538c"
    SOURCE_DIR ${EP_SOURCE_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDS}
    )
  set(${proj}_DIR ${EP_SOURCE_DIR})
  set(libigl_SOURCE_DIR ${EP_SOURCE_DIR} CACHE PATH "libigl source directory" FORCE)
  set(LIBIGL_SOURCE_DIR ${EP_SOURCE_DIR} CACHE PATH "libigl source directory (upper)" FORCE)

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

mark_as_superbuild(${proj}_DIR:PATH)
mark_as_superbuild(libigl_SOURCE_DIR:PATH)
