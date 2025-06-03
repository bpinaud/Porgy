FIND_PACKAGE(Sphinx)
IF(SPHINX_FOUND)
  ADD_SUBDIRECTORY(source)
ELSE(SPHINX_FOUND)
  MESSAGE(AUTHOR_WARNING "Sphinx not found: unable to generate the documentation.")
ENDIF(SPHINX_FOUND)

