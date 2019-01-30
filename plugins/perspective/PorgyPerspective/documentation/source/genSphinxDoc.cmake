SET(CHAR_SEP ":")
IF(WIN32)
SET(CHAR_SEP ";")
IF(MSVC)
SET(ENV{PATH} "${QT_BIN_DIR};$ENV{PATH}")
ELSE(MSVC)
SET(ENV{PATH} "${QT_BIN_DIR};$ENV{PATH}")
ENDIF(MSVC)
ENDIF(WIN32)


IF(APPLE)
SET(ENV{LC_ALL} "en_EN.UTF-8")
ENDIF(APPLE)
# Generate the html doc.
EXECUTE_PROCESS(COMMAND ${SPHINX_EXECUTABLE} -c ${CMAKE_CURRENT_BINARY_DIR} -b html -E -d ${BIN_DIR}/doctrees ${SOURCE_DIR} ${BIN_DIR}/html)
