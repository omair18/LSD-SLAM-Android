ADD_SUBDIRECTORY(slam_parser)

IF(CHOLMOD_FOUND)
  ADD_SUBDIRECTORY(g2o_interactive)
  IF(${G2O_USE_GPL} STREQUAL "ON")
    MESSAGE(STATUS "Building example depending on GPL'd library parts")
    ADD_SUBDIRECTORY(g2o_incremental)
  ENDIF()
ENDIF(CHOLMOD_FOUND)
