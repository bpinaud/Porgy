INCLUDE_DIRECTORIES(${TULIP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../AbstractPorgyGraphView ${CMAKE_CURRENT_BINARY_DIR} ${PORGY_INCLUDE_DIR})

STRING(REGEX REPLACE ".*/" "" PLUGIN_NAME ${CMAKE_CURRENT_SOURCE_DIR})
STRING(TOLOWER ${PLUGIN_NAME} PLUGIN_NAME)

SET(LIB_SRCS
    SMView.cpp
    porgysmallmultiplesconfigurationwidget.cpp
    )

SET(LIB_UI
    porgysmallmultiplesconfigurationwidget.ui
   )

SET(LIB_MOC
    SMView.h
    porgysmallmultiplesconfigurationwidget.h
    )

QTX_WRAP_UI(LIB_UI_SRCS ${LIB_UI})
QTX_WRAP_CPP(LIB_MOC_SRCS ${LIB_MOC})

ADD_LIBRARY(${PLUGIN_NAME}-${Tulip_VERSION} SHARED ${LIB_SRCS} ${LIB_MOC_SRCS} ${LIB_UI_SRCS})
TARGET_LINK_LIBRARIES(${PLUGIN_NAME}-${Tulip_VERSION} PorgyIntermediateStatesStatic PorgyAbstractViewStatic PorgyAbstractTraceViewStatic PorgyStateManagerStatic ${PORGY_LIBS} ${TULIP_LIBRARIES} ${QT_LIBRARIES} ${OPENGL_gl_LIBRARY})
INSTALL(TARGETS ${PLUGIN_NAME}-${Tulip_VERSION} DESTINATION ${TulipViewPluginsInstallDir} COMPONENT Porgy)
