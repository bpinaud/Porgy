INCLUDE_DIRECTORIES(${PORGY_TULIP_SRC_DIR}/library/tulip-gui/include ${PORGY_TULIP_SRC_DIR}/library/tulip-ogl/include)
ADD_SUBDIRECTORY(AbstractPorgyGraphView)
ADD_SUBDIRECTORY(graphview)
ADD_SUBDIRECTORY(TraceGraphView)
ADD_SUBDIRECTORY(RuleGraphView)
ADD_SUBDIRECTORY(porgySMView)
ADD_SUBDIRECTORY(porgygraphstateview)

