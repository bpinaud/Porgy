project(Porgy)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

FIND_PACKAGE( TULIP 5.3 REQUIRED )

SET(PORGY_RULE_DEBUG_MESSAGES OFF CACHE BOOL "Do you want to activate rule application debugging (lots of output)? [ON|OFF]")
MARK_AS_ADVANCED(PORGY_RULE_DEBUG_MESSAGES)


IF(PORGY_RULE_DEBUG_MESSAGES)
    ADD_DEFINITIONS(-DPORGY_RULE_DEBUG_MESSAGES)
    ENDIF(PORGY_RULE_DEBUG_MESSAGES)

ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
FIND_PACKAGE( Boost 1.60 REQUIRED)


# OpenMP
IF(NOT CLANG OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.7.0 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 3.7.0)

  FIND_PACKAGE(Threads)
  IF (NOT CMAKE_BUILD_TYPE STREQUAL "")
    IF(CMAKE_DEBUG_MODE)
      OPTION(TULIP_ENABLE_OPENMP "Do you want to enable OpenMP (debug mode)?" OFF)
    ELSE()
      OPTION(TULIP_ENABLE_OPENMP "Do you want to enable OpenMP?" ON)
    ENDIF()
    IF(TULIP_ENABLE_OPENMP)
      FIND_PACKAGE(OpenMP)
      IF(OPENMP_FOUND)
        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OpenMP_CXX_FLAGS}")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${OpenMP_CXX_FLAGS}")
        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OpenMP_CXX_FLAGS}")
        SET(OPENMP_CXX_FLAGS "${OpenMP_CXX_FLAGS}")
        IF(WIN32)
          IF(MSVC)
            SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /openmp")
            SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /openmp")
            SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /openmp")
            SET(OPENMP_CXX_FLAGS "/openmp")
          ELSE()
            SET(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lgomp ${CMAKE_THREAD_LIBS_INIT}")
            SET(OPENMP_LIBRARIES "-lgomp -lpthread")
          ENDIF()
        ENDIF()
          ELSE(OPENMP_FOUND)
        IF(WIN32)
          STRING(COMPARE NOTEQUAL "${OpenMP_C_FLAGS}" "" OMP_CFLAGS)
          IF(OMP_CFLAGS)
            # Force setting OpenMP flags on Windows plateforms
            SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OpenMP_C_FLAGS}")
            SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${OpenMP_C_FLAGS}")
            SET(OPENMP_CXX_FLAGS "${OpenMP_C_FLAGS}")
            IF(NOT MSVC)
              SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_C_FLAGS}")
              SET(OPENMP_LINKER_FLAGS "${OpenMP_C_FLAGS}")
              SET(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lgomp -lpthread")
              SET(OPENMP_LIBRARIES "-lgomp -lpthread")
            ENDIF(NOT MSVC)
            SET(OPENMP_FOUND TRUE)
          ELSE(OMP_CFLAGS)
            MESSAGE("OpenMP not found: multithreaded code will not be used")
          ENDIF(OMP_CFLAGS)
        ELSE(WIN32)
          MESSAGE("OpenMP not found: multithreaded code will not be used")
        ENDIF(WIN32)
      ENDIF(OPENMP_FOUND)
    ENDIF(TULIP_ENABLE_OPENMP)
  ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "")
ENDIF()


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
## Porgy flags, includes and librairies
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

#Clang does not use C++11 by default
#Enable C++11 if gcc version < 6.2 (c++14 activated by default since GCC 6.2)
#Enable C++0x if gcc < 4.7
IF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSE(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
    IF((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.1) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    ENDIF((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.1) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
ENDIF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)

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
