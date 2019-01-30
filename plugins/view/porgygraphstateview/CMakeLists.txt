INCLUDE_DIRECTORIES(${TULIP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../AbstractPorgyGraphView ${CMAKE_CURRENT_BINARY_DIR} ${PORGY_INCLUDE_DIR})

STRING(REGEX REPLACE ".*/" "" PLUGIN_NAME ${CMAKE_CURRENT_SOURCE_DIR})
STRING(TOLOWER ${PLUGIN_NAME} PLUGIN_NAME)

SET(LIB_SRCS
    AbstractGraphMorphing.cpp
    GlSceneMorphing.cpp
    porgyAnimationconfigurationwidget.cpp
    porgyAnimationtraceview.cpp
    SimpleCameraMorphing.cpp
    VisualPropertiesMorphing.cpp
    AnimationViewQuickAccessBar.cpp
    rulepreviewwidget.cpp
    )

SET(LIB_UI
    designer/porgyAnimationconfigurationwidget.ui
    designer/rulepreviewwidget.ui)

SET(LIB_MOC
    porgyAnimationconfigurationwidget.h
    porgyAnimationtraceview.h
    AnimationViewQuickAccessBar.h
    rulepreviewwidget.h
    )

SET(LIB_RCC
    resources/traceAnimationView.qrc)

QTX_WRAP_UI(LIB_UI_SRCS ${LIB_UI})
QTX_WRAP_CPP(LIB_MOC_SRCS ${LIB_MOC})
QTX_ADD_RESOURCES(LIB_RCC_SRCS ${LIB_RCC})

ADD_LIBRARY(${PLUGIN_NAME}-${TULIP_VERSION} SHARED ${LIB_SRCS} ${LIB_UI_SRCS} ${LIB_MOC_SRCS} ${LIB_RCC_SRCS})
TARGET_LINK_LIBRARIES(${PLUGIN_NAME}-${TULIP_VERSION} PorgyIntermediateStatesStatic PorgyAbstractViewStatic PorgyAbstractTraceViewStatic PorgyStateManagerStatic ${PORGY_LIBS} ${OPENGL_gl_LIBRARY} ${TULIP_LIBRARIES} ${QT_LIBRARIES})
INSTALL(TARGETS ${PLUGIN_NAME}-${TULIP_VERSION} DESTINATION ${TULIP_VIEW_PLUGINS_DIR} COMPONENT Porgy)
