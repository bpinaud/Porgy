project(Porgy VERSION 3.2)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

#Force the use of Qt6
SET(TULIP_USE_QT6 ON CACHE BOOL "Use Qt6 instead of Qt5" FORCE)

if(POLICY CMP0167)
cmake_policy(SET CMP0167 OLD)
endif()

SET(TULIP_BINARIES_DIR ${TULIP_DIR}/bin)
SET(PORGY_TULIP_SRC_DIR "/usr/src/tulip" CACHE PATH "Tulip sources tree is mandatory to compile Porgy. Set the path to the root directory of the Tulip sources.")

SET(PORGY_RULE_DEBUG_MESSAGES OFF CACHE BOOL "Do you want to activate rule application debugging (lots of output)? [ON|OFF]")
MARK_AS_ADVANCED(PORGY_RULE_DEBUG_MESSAGES)

IF(PORGY_RULE_DEBUG_MESSAGES)
  ADD_DEFINITIONS(-DPORGY_RULE_DEBUG_MESSAGES)
ENDIF(PORGY_RULE_DEBUG_MESSAGES)

ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
FIND_PACKAGE( Boost 1.60 REQUIRED)

IF(Boost_VERSION_STRING EQUAL "1.74.0" OR Boost_VERSION_STRING EQUAL "1.75.0")
    message(FATAL_ERROR "Boost ${Boost_VERSION_STRING} detected. Boost versions 1.74 and 1.75 have a bug in spirit. They cannot be used")
ENDIF(Boost_VERSION_STRING EQUAL "1.74.0" OR Boost_VERSION_STRING EQUAL "1.75.0")


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

SET(PORTGRAPH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/portgraph/include ${CMAKE_CURRENT_BINARY_DIR}/portgraph/include)
SET(PORTGRAPH_LIB PortGraph-${TULIP_MM_VERSION})
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

SET(PorgyCoreInclude ${CMAKE_CURRENT_SOURCE_DIR}/portgraph/include/)
SET(PorgyCoreBuildInclude ${CMAKE_CURRENT_BINARY_DIR}/portgraph/include/)

SET(PORGY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/porgy-qt/include ${CMAKE_CURRENT_BINARY_DIR}/porgy-qt ${PORTGRAPH_INCLUDE_DIR})
SET(PORGY_LIB PorgyQt-${TULIP_MM_VERSION})
SET(PORGY_LIBS ${PORGY_LIB} ${PORTGRAPH_LIB} ${TULIP_LIBRARIES})
QTX_SET_INCLUDES_AND_DEFINITIONS()

add_subdirectory(porgy-qt)
add_subdirectory(plugins/interactors)
add_subdirectory(plugins/perspective)
add_subdirectory(plugins/view)
add_subdirectory(plugins/general/Strategy-spirit)
