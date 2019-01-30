INCLUDE_DIRECTORIES(${TULIP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${PORGY_INCLUDE_DIR})

SET(LIB_abstractporgyview
    AbstractPorgyGraphView.cpp
    )

SET(LIB_MOC_abstractporgyview
    AbstractPorgyGraphView.h
)

SET(LIB_intermediatestates
    intermediatestates.cpp
    )

SET(LIB_MOC_intermediatestates
    intermediatestates.h
    )

SET(LIB_abstractportgraphview
    abstractportgraphview.cpp
    )

SET(LIB_MOC_abstractportgraphview
    abstractportgraphview.h
)

SET(LIB_abstracttraceview
    abstracttraceview.cpp
    )

SET(LIB_MOC_traceview
    abstracttraceview.h
)

SET(LIB_STATEMANAGER
    graphstatemanager.cpp
    )

SET(LIB_MOC_STATEMANAGER
    graphstatemanager.h
)

SET(LIB_RCC
    resources/smview_icons.qrc
    )


QTX_WRAP_CPP(LIB_MOC_SRCS_INTERMEDIATESTATES ${LIB_MOC_intermediatestates})
QTX_WRAP_CPP(LIB_MOC_SRCS_PORGY ${LIB_MOC_abstractporgyview})
QTX_WRAP_CPP(LIB_MOC_SRCS_TRACE ${LIB_MOC_traceview})
QTX_WRAP_CPP(LIB_MOC_SRCS_PORTGRAPH ${LIB_MOC_abstractportgraphview})
QTX_WRAP_CPP(LIB_MOC_SRCS_STATEMANAGER ${LIB_MOC_STATEMANAGER})

QTX_ADD_RESOURCES(LIB_RCC_SRCS ${LIB_RCC})

ADD_LIBRARY(PorgyIntermediateStatesStatic STATIC ${LIB_intermediatestates} ${LIB_MOC_SRCS_INTERMEDIATESTATES} ${LIB_RCC_SRCS})
ADD_LIBRARY(PorgyAbstractViewStatic STATIC ${LIB_abstractporgyview} ${LIB_MOC_SRCS_PORGY})
ADD_LIBRARY(PorgyAbstractTraceViewStatic STATIC  ${LIB_abstracttraceview} ${LIB_MOC_SRCS_TRACE})
ADD_LIBRARY(PorgyAbstractPortGraphViewStatic STATIC ${LIB_abstractportgraphview} ${LIB_MOC_SRCS_PORTGRAPH})
ADD_LIBRARY(PorgyStateManagerStatic STATIC ${LIB_STATEMANAGER} ${LIB_MOC_SRCS_STATEMANAGER})
