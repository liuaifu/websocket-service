# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindDBPOOL
# --------
#
# Find dbpool
#
# Find the native dbpool headers and libraries.
#
# ::
#
#   DBPOOL_INCLUDE_DIR    - where to find libdbpool/DbPool.h, etc.
#   DBPOOL_LIBRARIES      - List of libraries when using dbpool.
#   DBPOOL_FOUND          - True if dbpool found.

# Look for the header file.
find_path(DBPOOL_INCLUDE_DIR libdbpool/DbPool.h ${DBPOOL_INCLUDEDIR})
mark_as_advanced(DBPOOL_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
# find_library(DBPOOL_LIBRARY NAMES
#     dbpool
# )
# mark_as_advanced(DBPOOL_LIBRARY)

IF(DBPOOL_INCLUDE_DIR)
  SET(DBPOOL_FOUND TRUE)
  SET(DBPOOL_LIBRARIES ${DBPOOL_LIBRARY})
ELSE(DBPOOL_INCLUDE_DIR)
  SET(DBPOOL_FOUND FALSE)
  SET( DBPOOL_LIBRARIES )
ENDIF(DBPOOL_INCLUDE_DIR)

IF(DBPOOL_FOUND)
  IF(NOT DBPOOL_FIND_QUIETLY)
    MESSAGE(STATUS "Found dbpool: ${DBPOOL_INCLUDE_DIR}")
  ENDIF(NOT DBPOOL_FIND_QUIETLY)
ELSE(DBPOOL_FOUND)
  IF(DBPOOL_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find dbpool library, you can set DBPOOL_INCLUDEDIR to the correct directory")
  ENDIF(DBPOOL_FIND_REQUIRED)
ENDIF(DBPOOL_FOUND)
