INCLUDE_DIRECTORIES(${TULIP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR} ${PORGY_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

STRING(REGEX REPLACE ".*/" "" PLUGIN_NAME ${CMAKE_CURRENT_SOURCE_DIR})
STRING(TOLOWER ${PLUGIN_NAME} PLUGIN_NAME)

SET(LIB_SRCS
    src/abstractgraphhierachyitemmodel.cpp
    src/modelgraphtreemodel.cpp
    src/porgyperspective.cpp
    src/ruleswidget.cpp
    src/statusbarpluginprogress.cpp
    src/subgraphshierachytreemodel.cpp
    src/viewmanager.cpp
    src/GlMainWidgetZoomAndPanAnimation.cpp
    src/PorgyPerspectiveLogger.cpp
    src/GraphWidget.cpp
    src/qtimagetooltip.cpp
    src/sortfilterproxymodel.cpp
    src/strategymanagementwidget.cpp
    src/tracesitemmodel.cpp
    src/graphmodificationlistener.cpp
    src/NewTrace.cpp
    src/PorgyPropertyManager.cpp
    src/rulesgraphitemmodel.cpp
    src/special_paste.cpp
    src/strategymanager.cpp
    src/traceswidget.cpp
    src/AboutDialog.cpp
    src/codeeditorlistview.cpp
    src/codeeditorplaintextview.cpp
    src/xmlsyntaxparser.cpp
    src/highlightingrule.cpp
    src/generalpurposesyntaxhighlightingrules.cpp
    src/generalpurposesyntaxhighlighter.cpp
    src/PreferencesDialog.cpp
    src/validaterule.cpp)

SET(LIB_UI
    designer/NewTrace.ui
    designer/PorgyPropertyManager.ui
    designer/porgyperspective.ui
    designer/special_paste.ui
    designer/strategymanagementwidget.ui
    designer/PorgyPerspectiveLogger.ui
    designer/GraphWidget.ui
    designer/ruleswidget.ui
    designer/statusbarpluginprogress.ui
    designer/traceswidget.ui
    designer/AboutDialog.ui
    designer/PreferencesDialog.ui
    designer/rulevalidation.ui)

SET(LIB_MOC
    include/porgyperspective.h
    include/ruleswidget.h
    include/statusbarpluginprogress.h
    include/subgraphshierachytreemodel.h
    include/viewmanager.h
    include/GraphWidget.h
    include/qtimagetooltip.h
    include/strategymanagementwidget.h
    include/PorgyPerspectiveLogger.h
    include/NewTrace.h
    include/PorgyPropertyManager.h
    include/traceswidget.h
    include/abstractgraphhierachyitemmodel.h
    include/AboutDialog.h
    include/codeeditorlistview.h
    include/codeeditorplaintextview.h
    include/generalpurposesyntaxhighlighter.h
    include/PreferencesDialog.h
    include/validaterule.h)

SET(LIB_RCC
    resources/Porgy.qrc
    )

QTX_WRAP_UI(LIB_UI_SRCS ${LIB_UI})
QTX_WRAP_CPP(LIB_MOC_SRCS ${LIB_MOC})
QTX_ADD_RESOURCES(LIB_RCC_SRCS ${LIB_RCC})

ADD_SUBDIRECTORY(documentation)

ADD_LIBRARY(${PLUGIN_NAME}-${TULIP_VERSION} SHARED ${LIB_SRCS} ${LIB_UI_SRCS} ${LIB_MOC_SRCS} ${LIB_RCC_SRCS})
TARGET_LINK_LIBRARIES(${PLUGIN_NAME}-${TULIP_VERSION} ${PORGY_LIBS} ${TULIP_LIBRARIES} ${QT_LIBRARIES} ${TULIP_PYTHON_LIBRARY})
INSTALL(TARGETS ${PLUGIN_NAME}-${TULIP_VERSION} DESTINATION ${TULIP_PERSPECTIVE_PLUGINS_DIR} COMPONENT Porgy)
