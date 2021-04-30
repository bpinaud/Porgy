project(Porgy)
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Force the use of the legacy libGL.so for CMake >= 3.11
IF(POLICY CMP0072)
  CMAKE_POLICY(SET CMP0072 OLD)
ENDIF(POLICY CMP0072)

FIND_PACKAGE( TULIP 5.4.0 REQUIRED )
SET(TULIP_BINARIES_DIR ${TULIP_DIR}/bin)
SET(PORGY_TULIP_SRC_DIR "/usr/src/tulip" CACHE PATH "Tulip sources tree is mandatory to compile Porgy. Set the path to the root directory of the Tulip sources.")

SET(PORGY_RULE_DEBUG_MESSAGES OFF CACHE BOOL "Do you want to activate rule application debugging (lots of output)? [ON|OFF]")
MARK_AS_ADVANCED(PORGY_RULE_DEBUG_MESSAGES)

TULIP_SET_COMPILER_OPTIONS()

SET(PORGY_USE_CCACHE OFF CACHE BOOL "Do you want to use ccache when compiling PORGY? [OFF|ON]")
  IF(PORGY_USE_CCACHE)
  FIND_PROGRAM(CCACHE_PROGRAM ccache)
  IF(CCACHE_PROGRAM)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  ENDIF(CCACHE_PROGRAM)
ENDIF(PORGY_USE_CCACHE)

IF(PORGY_RULE_DEBUG_MESSAGES)
  ADD_DEFINITIONS(-DPORGY_RULE_DEBUG_MESSAGES)
ENDIF(PORGY_RULE_DEBUG_MESSAGES)

ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
FIND_PACKAGE( Boost 1.60 REQUIRED)

IF(Boost_VERSION_STRING EQUAL "1.74.0" OR Boost_VERSION_STRING EQUAL "1.75.0")
    message(FATAL_ERROR "Boost ${Boost_VERSION_STRING} detected. Boost versions 1.74 and 1.75 have a bug in spirit. They cannot be used")
ENDIF(Boost_VERSION_STRING EQUAL "1.74.0" OR Boost_VERSION_STRING EQUAL "1.75.0")

## ========================================================
## Porgy version variables
## ========================================================
SET(PorgyMajorVersion 3)
SET(PorgyMinorVersion 2)
SET(PorgyVersionSeparator .)
IF(MSVC) #specific options for Visual Studio
  SET(PorgyVersionSeparator _)
ENDIF(MSVC)
SET(PorgyVersion "${PorgyMajorVersion}${PorgyVersionSeparator}${PorgyMinorVersion}")

# Force build type to release with debug info if none has been provided
IF(NOT MSVC AND "${CMAKE_BUILD_TYPE}" STREQUAL "")
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
ENDIF(NOT MSVC AND "${CMAKE_BUILD_TYPE}" STREQUAL "")

## ========================================================
## Porgy flags, includes and libraries
## ========================================================
IF(NOT MSVC) #visual studio does not recognize these options
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused -Wno-long-long -Wold-style-cast")
   IF(WIN32 AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
   ENDIF(WIN32 AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
   IF(NOT APPLE)
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
   ENDIF(NOT APPLE)
ENDIF(NOT MSVC)
IF(UNIX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF(UNIX)

# enable C++11
SET(CMAKE_CXX_STANDARD 11)

SET(PORTGRAPH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/portgraph/include ${CMAKE_CURRENT_BINARY_DIR}/portgraph/include)
SET(PORTGRAPH_LIB PortGraph-${TULIP_VERSION})
SET(PORTGRAPH_LIBS ${PORTGRAPH_LIB} ${TULIP_LIBRARIES})

## ========================================================
## Porgy options
## ========================================================
CONFIGURE_FILE("portgraph/include/portgraph/PorgyRelease.h.in" "portgraph/include/portgraph/PorgyRelease.h")

add_subdirectory(portgraph)  #no qt required for the portgraph lib
add_subdirectory(plugins/porgyImport)
add_subdirectory(plugins/export/ruleExport)
add_subdirectory(plugins/layout)
add_subdirectory(plugins/measure)
add_subdirectory(plugins/selection)
add_subdirectory(plugins/general/CheckApply)
add_subdirectory(plugins/general/RuleAlgorithm)
add_subdirectory(plugins/general/RuleCondition)
add_subdirectory(plugins/general/TraceIsomorphModels)

FIND_PACKAGE(QtX)
SET(PorgyCoreInclude ${CMAKE_CURRENT_SOURCE_DIR}/portgraph/include/)
SET(PorgyCoreBuildInclude ${CMAKE_CURRENT_BINARY_DIR}/portgraph/include/)

SET(PORGY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/porgy-qt/include ${CMAKE_CURRENT_BINARY_DIR}/porgy-qt ${PORTGRAPH_INCLUDE_DIR})
SET(PORGY_LIB PorgyQt-${TULIP_VERSION})
SET(PORGY_LIBS ${PORGY_LIB} ${PORTGRAPH_LIB} ${TULIP_LIBRARIES})
QTX_SET_INCLUDES_AND_DEFINITIONS()
FIND_PACKAGE(OpenGL REQUIRED)
add_subdirectory(porgy-qt)
add_subdirectory(plugins/interactors)
add_subdirectory(plugins/perspective)
add_subdirectory(plugins/view)
add_subdirectory(plugins/general/Strategy-spirit)
