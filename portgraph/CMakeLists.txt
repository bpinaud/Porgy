INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${TULIP_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS})

IF(WIN32)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDLL_PORTGRAPH")
ENDIF(WIN32)

SET(LIB_SRCS
    PortBase.cpp
    PortNodeBase.cpp
    Bridge.cpp
    BridgePort.cpp
    PorgyTlpGraphStructure.cpp
    Port.cpp
    PortGraph.cpp
    PortGraphModel.cpp
    PortGraphRule.cpp
    PortNode.cpp
    Trace.cpp
    )

ADD_LIBRARY(${PORTGRAPH_LIB} SHARED ${LIB_SRCS})
TARGET_LINK_LIBRARIES(${PORTGRAPH_LIB} ${TULIP_CORE_LIBRARY})
INSTALL(TARGETS ${PORTGRAPH_LIB}
  RUNTIME DESTINATION ${TulipBinInstallDir} COMPONENT PortGraphLib
  LIBRARY DESTINATION ${TulipLibInstallDir} COMPONENT PortGraphLib
  ARCHIVE DESTINATION ${TulipLibInstallDir} COMPONENT PortGraphLib)
