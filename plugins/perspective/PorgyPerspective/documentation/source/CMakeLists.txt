CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

ADD_CUSTOM_TARGET(porgyDoc ALL ${CMAKE_COMMAND} -c ${CMAKE_CURRENT_BINARY_DIR} -DSPHINX_EXECUTABLE=${SPHINX_EXECUTABLE} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR} -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DSYSTEM_SIP=${SYSTEM_SIP} -DQT_BIN_DIR=${QT_DOC_DIR}/../bin -DMSVC=${MSVC} -DAPPLE=${APPLE} -P ${CMAKE_CURRENT_SOURCE_DIR}/genSphinxDoc.cmake VERBATIM)

SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "html;doctrees") 

INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html 
  DESTINATION ${TulipShareInstallDir}/doc/porgy
  COMPONENT porgy_doc)
