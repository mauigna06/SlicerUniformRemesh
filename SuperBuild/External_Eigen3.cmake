set(proj Eigen3)

# Set dependency list
set(${proj}_DEPENDS
  ""
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED Eigen3_SOURCE_DIR AND NOT EXISTS ${Eigen3_SOURCE_DIR})
  message(FATAL_ERROR "Eigen3_SOURCE_DIR [${Eigen3_SOURCE_DIR}] variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "https://gitlab.com/libeigen/eigen.git"
    GIT_TAG "3147391d946bb4b6c68edd901f2add6ac1f31f8c" # 3.4.0-20240313
    SOURCE_DIR ${EP_SOURCE_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDS}
    )
  set(${proj}_DIR ${EP_SOURCE_DIR})
  set(Eigen3_SOURCE_DIR ${EP_SOURCE_DIR} CACHE PATH "Eigen3 source directory" FORCE)
  set(EIGEN3_SOURCE_DIR ${EP_SOURCE_DIR} CACHE PATH "Eigen3 source directory (upper)" FORCE)

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

mark_as_superbuild(${proj}_DIR:PATH)
mark_as_superbuild(Eigen3_SOURCE_DIR:PATH)
